/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University
 * of Wisconsin-Madison.
 */

/**
 *
 * Student: Haishuo Chen
 * Student ID: 9082791279
 * Student Email: hchen727@wisc.edu
 *
 * Student: Qinhang Wu
 * Student ID: 9084142851
 * Student Email: qinhang.wu@wisc.edu
 *
 * Student: Zihan Zhao
 * Student ID: 9082666315
 * Student Email: zzhao383@wisc.edu
 *
 */

#include "buffer.h"

#include <iostream>
#include <memory>

#include "exceptions/bad_buffer_exception.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/hash_not_found_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"

namespace badgerdb {

constexpr int HASHTABLE_SZ(int bufs) { return ((int)(bufs * 1.2) & -2) + 1; }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(std::uint32_t bufs)
    : numBufs(bufs),
      hashTable(HASHTABLE_SZ(bufs)),
      bufDescTable(bufs),
      bufPool(bufs) {
  for (FrameId i = 0; i < bufs; i++) {
    bufDescTable[i].frameNo = i;
    bufDescTable[i].valid = false;
  }

  clockHand = bufs - 1;
}

void BufMgr::advanceClock() { clockHand = (clockHand + 1) % numBufs; }

void BufMgr::allocBuf(FrameId& frame) {
  int loop = 0;
  advanceClock();
  FrameId start = clockHand;
  // Set all refbits to false in the first loop
  // Check if all frames are pinned in the second
  // Return whenever find an available frame
  while (loop < 3) {
    // Start a new loop
    if (start == clockHand) loop++;
    // Get current frame
    BufDesc& t_frame = bufDescTable[clockHand];
    // Find a frame with valid page in it
    if (t_frame.valid) {
      // Frame recently referenced
      if (t_frame.refbit) {
        t_frame.refbit = false;
        advanceClock();
        continue;
      }

      // Frame not recently referenced but being pinned
      if (t_frame.pinCnt > 0) {
        advanceClock();
        continue;
      }

      // Frame not being pinned anymore but dirty
      // bufPool is not modified here
      if (t_frame.dirty) {
        // Write modified page to disk
        t_frame.file.writePage(bufPool[clockHand]);
      }

      // Remove frame mapping from hashtable
      hashTable.remove(t_frame.file, t_frame.pageNo);
      // Reset frame metadata
      t_frame.clear();
    }

    // Find an available frame
    frame = clockHand;
    return;
  }

  // No available frames found
  throw BufferExceededException();
}

void BufMgr::readPage(File& file, const PageId pageNo, Page*& page) {
  FrameId fid;
  try {
    // Check if page in buffer
    hashTable.lookup(file, pageNo, fid);
    // Frame in the buffer
    // Set refbit to true
    bufDescTable[fid].refbit = true;
    // Increment pinCnt
    bufDescTable[fid].pinCnt++;
  } catch (const HashNotFoundException&) {
    // Frame not in the buffer
    // Allocate a new frame
    allocBuf(fid);
    // Read page from disk into buffer
    bufPool[fid] = file.readPage(pageNo);
    // Insert page into hashtable
    hashTable.insert(file, pageNo, fid);
    // Set up frame metadata
    bufDescTable[fid].Set(file, pageNo);
  }

  // Fetch the frame containing the page
  page = &bufPool[fid];
}

void BufMgr::unPinPage(File& file, const PageId pageNo, const bool dirty) {
  FrameId fid;
  try {
    // check if the page exists in the buffer
    hashTable.lookup(file, pageNo, fid);
    if (bufDescTable[fid].pinCnt == 0) {
      throw PageNotPinnedException(file.filename(), pageNo, fid);
    } else {
      // decrease pinCnt and set dirty bit
      bufDescTable[fid].pinCnt--;
      if (dirty) {
        bufDescTable[fid].dirty = true;
      }
    }
  } catch (const HashNotFoundException&) {
    // do nothing when the page is not found
  }
}

void BufMgr::allocPage(File& file, PageId& pageNo, Page*& page) {
  FrameId fid;
  // obtain a buffer pool frame
  allocBuf(fid);
  // allocate an empty page
  bufPool[fid] = file.allocatePage();
  // get page number
  pageNo = bufPool[fid].page_number();
  // insert the entry into the hash table
  hashTable.insert(file, pageNo, fid);
  // setup the frame properly
  bufDescTable[fid].Set(file, pageNo);

  page = &bufPool[fid];
}

void BufMgr::flushFile(File& file) {
  // iterate over bufDescTable
  for (FrameId fid = 0; fid < numBufs; fid++) {
    BufDesc& bufDesc = bufDescTable[fid];
    if (bufDesc.file == file) {
      // Throws BadBufferException if an invalid page belonging to the file is
      // encountered.
      if (!bufDesc.valid) {
        throw BadBufferException(fid, bufDesc.dirty, bufDesc.valid,
                                 bufDesc.refbit);
      }
      // Throws PagePinnedException if some page of the file is pinned.
      if (bufDesc.pinCnt > 0) {
        throw PagePinnedException(file.filename(), bufDesc.pageNo, fid);
      }
      if (bufDesc.dirty) {
        // Call file.writePage() to flush the page to disk.
        file.writePage(bufPool[fid]);
        // Set the dirty bit for the page to false.
        bufDesc.dirty = false;
      }
      // Remove the page from the hashtable.
      hashTable.remove(file, bufDesc.pageNo);
      // Clear the bufDesc.
      bufDesc.clear();
    }
  }
}

void BufMgr::disposePage(File& file, const PageId PageNo) {
  FrameId fid;
  try {
    // Check if page in buffer
    hashTable.lookup(file, PageNo, fid);
    // Frame in the buffer
    // Clear DescTable
    bufDescTable[fid].clear();
    // remove page from hashTable
    hashTable.remove(file, PageNo);
    // delete page from file
    file.deletePage(PageNo);
  } catch (const HashNotFoundException&) {
    // Frame not in the buffer
    file.deletePage(PageNo);
  }
}

void BufMgr::printSelf(void) {
  int validFrames = 0;

  for (FrameId i = 0; i < numBufs; i++) {
    std::cout << "FrameNo:" << i << " ";
    bufDescTable[i].Print();

    if (bufDescTable[i].valid) validFrames++;
  }

  std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}  // namespace badgerdb
