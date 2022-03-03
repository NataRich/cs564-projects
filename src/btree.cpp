/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include "btree.h"
#include "filescan.h"
#include "exceptions/bad_index_info_exception.h"
#include "exceptions/bad_opcodes_exception.h"
#include "exceptions/bad_scanrange_exception.h"
#include "exceptions/no_such_key_found_exception.h"
#include "exceptions/scan_not_initialized_exception.h"
#include "exceptions/index_scan_completed_exception.h"
#include "exceptions/file_not_found_exception.h"
#include "exceptions/end_of_file_exception.h"


//#define DEBUG

namespace badgerdb
{

// -----------------------------------------------------------------------------
// BTreeIndex::BTreeIndex -- Constructor
// -----------------------------------------------------------------------------

BTreeIndex::BTreeIndex(const std::string & relationName,
		std::string & outIndexName,
		BufMgr *bufMgrIn,
		const int attrByteOffset,
		const Datatype attrType)
{
	// init bTree private data members
	this->bufMgr = bufMgrIn;
	this->attributeType = attrType; // according to the manual, this should be INT (fixed)
	this->attrByteOffset = attrByteOffset;
	this->leafOccupancy = INTARRAYLEAFSIZE;
	this->nodeOccupancy = INTARRAYNONLEAFSIZE;

	// construct the indexName string (name of the index file)
	std::ostringstream idxStr;
	idxStr << relationName << '.' << attrByteOffset;
	outIndexName = idxStr.str();

	// check whether the specified index file exists
	std::fstream fileTest(outIndexName);
	if(fileTest){ // file already existed
		fileTest.close();
		this->file = new BlobFile(outIndexName, true);
		// FIXME: for the next line, make a new file object in the tester, allocate the first page and print the page number -> examine whether the first page num is #0 or #1
		this->headerPageNum = (PageId) 1;

		Page *metaPage = new Page(this->file->readPage(this->headerPageNum));
		IndexMetaInfo *metaInfo = reinterpret_cast<IndexMetaInfo *>(metaPage);
		this->rootPageNum = metaInfo->rootPageNo;
		// FIXME: delete heap memory if needed
	}
	else{ // file not existed
		// create a file
		this->file = new BlobFile(outIndexName, true);

		// create MetaPage @388
		Page *metaPage = new Page;
		PageId metaPageId;
		this->bufMgr->allocPage(this->file, metaPageId, metaPage);
		this->headerPageNum = metaPageId;
		
		IndexMetaInfo *metaInfo = reinterpret_cast<IndexMetaInfo *>(metaPage);
		relationName.copy(metaInfo->relationName, 20);
		metaInfo->attrByteOffset = attrByteOffset;
		metaInfo->attrType = attrType;

		// create RootPage @393
		Page *rootPage = new Page;
		PageId rootPageId;
		this->bufMgr->allocPage(this->file, rootPageId, rootPage);
		metaInfo->rootPageNo = rootPageId;
		
		// insert entries
		{
			FileScan fscan(relationName, this->bufMgr);
			try{
				RecordId scanRid;
				while(1){
					// copy from main.cpp:118
					fscan.scanNext(scanRid);
					std::string recordStr = fscan.getRecord();
					const char *record = recordStr.c_str();
				}
			}
			catch(const EndOfFileException &e){}
		}

		// closing
		this->bufMgr->unPinPage(this->file, metaPageId, true);
		this->bufMgr->unPinPage(this->file, rootPageId, true);
		this->bufMgr->flushFile(this->file);
	}
}


// -----------------------------------------------------------------------------
// BTreeIndex::~BTreeIndex -- destructor
// -----------------------------------------------------------------------------

BTreeIndex::~BTreeIndex()
{
	// TODO: close pinned B+ tree pages

	this->bufMgr->flushFile(this->file);
	delete this->file;
	this->file = NULL;
}

// -----------------------------------------------------------------------------
// BTreeIndex::insertEntry
// -----------------------------------------------------------------------------

void BTreeIndex::insertEntry(const void *key, const RecordId rid) 
{

}

// -----------------------------------------------------------------------------
// BTreeIndex::startScan
// -----------------------------------------------------------------------------

void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{

}

// -----------------------------------------------------------------------------
// BTreeIndex::scanNext
// -----------------------------------------------------------------------------

void BTreeIndex::scanNext(RecordId& outRid) 
{

}

// -----------------------------------------------------------------------------
// BTreeIndex::endScan
// -----------------------------------------------------------------------------
//
void BTreeIndex::endScan() 
{

}

}
