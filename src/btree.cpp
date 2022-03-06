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

	this->leafOccupancy = INTARRAYLEAFSIZE-1;
	this->nodeOccupancy = INTARRAYNONLEAFSIZE-1;

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


		// key1 <= entry < key2.
		metaInfo->height = 1;
		// Initialization of rootnode
		LeafNodeInt* root_node = reinterpret_cast<LeafNodeInt*>(rootPage);
		root_node->rightSibPageNo = Page::INVALID_NUMBER;
		root_node->keyArray[0] = INT_MAX;
		root_node->ridArray[0].page_number = Page::INVALID_NUMBER;
		
		// insert entries
		{
			typedef struct tuple {
				int i;
				double d;
				char s[64];
			} RECORD;
			FileScan fscan(relationName, this->bufMgr);
			try{
				RecordId scanRid;
				while(1){
					// copy from main.cpp:118
					fscan.scanNext(scanRid);
					std::string recordStr = fscan.getRecord();
					const char *record = recordStr.c_str();
					int* key = (int*)(record + offsetof(RECORD, i));
					insertEntry((void*)key, scanRid);
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

void BTreeIndex::insertUnderNode(RIDKeyPair<int>* entry, Page* cur_page, bool is_leaf, PageKeyPair<int>* new_child)
{
	if (is_leaf)
	{
		LeafNodeInt* cur_node = reinterpret_cast<LeafNodeInt*>(cur_page);
		/// find the last record's position
		int entry_num = 0;
		for (; entry_num <= this->leafOccupancy; entry_num++) {
			if (cur_node->ridArray[entry_num].page_number == 0) {
				break;
			}
		}
		/// find the position to insert
		/// No duplicate keys, so we don't need <= or >=
		int pos = 0;
		for (; pos <= this->leafOccupancy; pos++) {
			if ((entry->key < cur_node->keyArray[pos] ) ||
				(cur_node->ridArray[pos].page_number == 0)) {
				break;
			}
		}

		/// If the position of insertion is at the end of the node, we don't need to move the slots after the
		/// position one element behind.
		if (pos < this->leafOccupancy) {
			copyArray<int>(cur_node->keyArray + pos, cur_node->keyArray + pos + 1, entry_num - pos);
			copyArray<RecordId>(cur_node->ridArray + pos, cur_node->ridArray + pos + 1, entry_num - pos);
		}
		cur_node->keyArray[pos] = entry->key;
		/// Can I copy struct directly?
		cur_node->ridArray[pos] = entry->rid;
		
		/// For every leafnode, we restrict the max number or records we store to INTARRAYLEAFSIZE-1
		if (entry_num == this->leafOccupancy) {
			/// We split it half-half. For odd lengths, we allocate the left leaf one more record.
			Page* new_page = new Page;
			PageId new_page_id;
			this->bufMgr->allocPage(this->file, new_page_id, new_page);
			LeafNodeInt* new_right_sibling = reinterpret_cast<LeafNodeInt*>(new_page);
			/// I am not very sure about the copy index here
			copyArray<int>(cur_node->keyArray + this->leafOccupancy / 2 + 1, new_right_sibling->keyArray, (this->leafOccupancy + 1) / 2);
			copyArray<RecordId>(cur_node->ridArray + this->leafOccupancy / 2 + 1, new_right_sibling->ridArray, (this->leafOccupancy + 1) / 2);
			
			new_right_sibling->rightSibPageNo = cur_node->rightSibPageNo;
			cur_node->rightSibPageNo = new_page_id;

			/// set the next entry as [INT_MAx, Page::INVALID_NUMBER] to be identify as bound
			cur_node->ridArray[this->leafOccupancy / 2 + 1].page_number = Page::INVALID_NUMBER;
			cur_node->keyArray[this->leafOccupancy / 2 + 1] = INT_MAX;
			new_right_sibling->ridArray[(this->leafOccupancy + 1) / 2].page_number = Page::INVALID_NUMBER;
			new_right_sibling->keyArray[(this->leafOccupancy + 1) / 2] = INT_MAX;
			
			/// return the new right sibling page for an insertion in the parent node
			new_child->set(new_page_id, new_right_sibling->keyArray[0]);
		}
		else {
			/// set the next entry as [INT_MAx, Page::INVALID_NUMBER] to be identify as bound
			cur_node->ridArray[entry_num + 1].page_number = Page::INVALID_NUMBER;
			cur_node->keyArray[entry_num + 1] = INT_MAX;
			new_child->set(Page::INVALID_NUMBER, entry->key);
		}
		
	}
	else {
		/// Nonleaf could be empty
		NonLeafNodeInt* cur_node = reinterpret_cast<NonLeafNodeInt*>(cur_page);
		/// find the position of PageNo to go down the tree
		/// We use >= for lower keys and < for higher keys
		/// logic: pageNoArray[0] store the records that have the keys less than keyArray[0]
		/// pageNoArray[n] store the records that have the keys less than keyArray[n] but greater or equal than keyArray[n-1]
		int pos = 0;
		for (; pos <= this->nodeOccupancy; pos++) {
			if ((entry->key < cur_node->keyArray[pos]) ||
				(cur_node->pageNoArray[pos+1] == 0)) {
				break;
			}
		}
		Page* child_page;
		bufMgr->readPage(this->file, cur_node->pageNoArray[pos], child_page);
		this->insertUnderNode(entry, child_page, cur_node->level, new_child);
		

		if (new_child->pageNo == Page::INVALID_NUMBER) {
			return;
		}

		/// find how many children do the node have ( the number of entries)
		int entry_num = 0;
		for (; entry_num <= this->nodeOccupancy+1; entry_num++) {
			if (cur_node->pageNoArray[entry_num] == Page::INVALID_NUMBER) {
				break;
			}
		}

		if (pos < this->nodeOccupancy) {
			copyArray<int>(cur_node->keyArray + pos, cur_node->keyArray + pos + 1, entry_num - 1 - pos);
			copyArray<PageId>(cur_node->pageNoArray + pos + 1, cur_node->pageNoArray + pos + 2, entry_num - 1 - pos);
		}
		cur_node->keyArray[pos] = new_child->key;
		/// Can I copy struct directly?
		cur_node->pageNoArray[pos + 1] = new_child->pageNo;
		
		/// For every leafnode, we restrict the max number or records we store to INTARRAYLEAFSIZE-1
		if (entry_num == this->nodeOccupancy + 1) {
			/// We split it half-half. For odd lengths, we allocate the left leaf one more record.
			Page* new_page = new Page;
			PageId new_page_id;
			this->bufMgr->allocPage(this->file, new_page_id, new_page);
			NonLeafNodeInt* new_right_sibling = reinterpret_cast<NonLeafNodeInt*>(new_page);
			int new_slot_key = cur_node->keyArray[(this->nodeOccupancy+1) / 2];

			copyArray<int>(cur_node->keyArray + (this->nodeOccupancy + 1) / 2 + 1, new_right_sibling->keyArray, this->nodeOccupancy / 2);
			copyArray<PageId>(cur_node->pageNoArray + (this->nodeOccupancy + 1) / 2 + 1, new_right_sibling->pageNoArray, this->nodeOccupancy / 2 + 1);
			new_right_sibling->level = cur_node->level;

			/// set the next entry as [Page::INVALID_NUMBER, INT_MAX] to be identify as bound
			cur_node->pageNoArray[(this->nodeOccupancy + 1) / 2 + 1] = Page::INVALID_NUMBER;
			cur_node->keyArray[(this->nodeOccupancy + 1) / 2] = INT_MAX;
			new_right_sibling->pageNoArray[this->nodeOccupancy / 2 + 1] = Page::INVALID_NUMBER;
			new_right_sibling->keyArray[this->nodeOccupancy / 2] = INT_MAX;
			
			new_child->set(new_page_id, new_slot_key);
		}
		else {
			/// set the next entry as [Page::INVALID_NUMBER, INT_MAX] to be identify as bound
			cur_node->pageNoArray[entry_num] = Page::INVALID_NUMBER;
			cur_node->keyArray[entry_num + 1] = INT_MAX;
			new_child->set(Page::INVALID_NUMBER, entry->key);
		}
		
	}
}


void BTreeIndex::insertEntry(const void *key, const RecordId rid) 
{
	RIDKeyPair<int>* insert_entry;
	insert_entry->set(rid, *((int*)key));
	Page* root_page;
	bufMgr->readPage(this->file, this->rootPageNum, root_page);
	PageKeyPair<int>* new_child;
	
	Page* meta_page;
	bufMgr->readPage(this->file, this->headerPageNum, meta_page);
	IndexMetaInfo* meta_info = reinterpret_cast<IndexMetaInfo*>(meta_page);
	bool is_leaf = (meta_info->height == 1);

	this->insertUnderNode(insert_entry, root_page, is_leaf, new_child);
	
	if (new_child->pageNo == Page::INVALID_NUMBER) {
		return;
	}

	Page* new_root_page = new Page;
	PageId new_root_page_id;
	this->bufMgr->allocPage(this->file, new_root_page_id, new_root_page);
	NonLeafNodeInt* new_root_node = reinterpret_cast<NonLeafNodeInt*>(new_root_page);
	new_root_node->pageNoArray[0] = this->rootPageNum;
	new_root_node->keyArray[0] = new_child->key;
	new_root_node->pageNoArray[1] = new_child->pageNo;
	this->rootPageNum = new_root_page_id;
	return;
}

void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{
	if (lowOpParm != GT && lowOpParm != GTE) {
		throw BadOpcodesException();
	}
	if (highOpParm != LT && highOpParm != LTE) {
		throw BadOpcodesException();
	}
	
	lowOp = lowOpParm;
	highOp = highOpParm;
	lowValInt = *((int*)lowValParm);
	highValInt = *((int*)highValParm);

	if (lowValInt > highValInt) {
		throw BadScanrangeException();
	}

	// End the previous scan
	if (scanExecuting) {
		endScan();
	}

	scanExecuting = true;

	// Set up initial scan parameters (from root)
	currentPageNum = rootPageNum;
	bufMgr->readPage(file, currentPageNum, currentPageData);
	NonLeafNodeInt* currNonLeafNode = 
		reinterpret_cast<NonLeafNodeInt*>(currentPageData);
	LeafNodeInt* leaf = NULL;

	// Loop until a leaf node is found or throw exception if no such key
	while (!leaf) {
		for (int entryId = 0; entryId < INTARRAYNONLEAFSIZE; entryId++) {
			bool isLastEntry = entryId == (INTARRAYNONLEAFSIZE - 1);
			int currKey = currNonLeafNode->keyArray[entryId];
			// Found a possible entry or the last entry of the node
			if (currKey >= lowValInt || isLastEntry) {
				int childEntryId = currKey > lowValInt ? entryId : entryId + 1;
				PageId childPageNum = currNonLeafNode->pageNoArray[childEntryId];
				
				// Failed to find a valid child node
				if (childPageNum == Page::INVALID_NUMBER) {
					throw NoSuchKeyFoundException();
				}

				// Found a valid child node
				// Set up the child node page paramters
				bufMgr->unPinPage(file, currentPageNum, false);
				currentPageNum = childPageNum;
				bufMgr->readPage(file, currentPageNum, currentPageData);

				// Found the leaf node
				if (currNonLeafNode->level == 1) {
					leaf = reinterpret_cast<LeafNodeInt*>(currentPageData);
					currNonLeafNode = NULL;
				}
				// Found the next non-leaf node
				else {
					currNonLeafNode = 
						reinterpret_cast<NonLeafNodeInt*>(currentPageData);
				}

				// Loop a new node
				break;
			}
		}
	}
	
	// Found a leaf node possibly containing the first wanted entry
	// Set a default (but impossible) nextEntry value
	nextEntry = -1;
	// Check if the wanted entry exists in the current leaf
	for (int entryId = 0; entryId < INTARRAYLEAFSIZE; entryId++) {
		int currKey = leaf->keyArray[entryId];
		PageId currPageNo = leaf->ridArray[entryId].page_number;

		// Have traversed to the last valid entry of the leaf
		if (currPageNo == Page::INVALID_NUMBER) {
			break;
		}

		if (currKey > lowValInt || (currKey == lowValInt && lowOp == GTE)) {
			nextEntry = entryId;
			break;
		}
	}

	// Found the wanted entry in the current leaf
	if (nextEntry != -1) {
		return;
	}

	// Have known that the wanted entry is in the right sibling node
	// Throw exception if the right sibling node does not exist
	if (leaf->rightSibPageNo == Page::INVALID_NUMBER) {
		throw NoSuchKeyFoundException();
	}

	// Set up the sibling page parameters and set the next entry to be the
	// first entry of the sibling page/node, i.e., 0
	bufMgr->unPinPage(file, currentPageNum, false);
	currentPageNum = leaf->rightSibPageNo;
	bufMgr->readPage(file, currentPageNum, currentPageData);
	nextEntry = 0;
}

// -----------------------------------------------------------------------------
// BTreeIndex::scanNext
// -----------------------------------------------------------------------------

void BTreeIndex::scanNext(RecordId& outRid) 
{
	if (!scanExecuting) {
		throw ScanNotInitializedException();
	}
	if (nextEntry == -1) {
		throw IndexScanCompletedException();
	}

	LeafNodeInt* leaf = reinterpret_cast<LeafNodeInt*>(currentPageData);
	outRid = leaf->ridArray[nextEntry];

	// The next entry is an invalid entry for the current leaf.
	// Go to its sibling leaf node
	if (leaf->ridArray[nextEntry + 1].page_number == Page::INVALID_NUMBER) {
		// Failed to find a sibling node, indicating scan completion
		if (leaf->rightSibPageNo == Page::INVALID_NUMBER) {
			nextEntry = -1;
		}

		// Found a sibling node
		// Set up the sibling page parameters
		bufMgr->unPinPage(file, currentPageNum, false);
		currentPageNum = leaf->rightSibPageNo;
		bufMgr->readPage(file, currentPageNum, currentPageData);
		leaf = reinterpret_cast<LeafNodeInt*>(currentPageData);
		int nextKey = leaf->keyArray[0];
			
		// Next key still within the boundary
		if (nextKey < highValInt || (nextKey == highValInt && highOp == LTE) {
			nextEntry = 0;
			return;
		}

		// Next key not within the boundary, indicating scan completion
		nextEntry = -1;
	}

	// The next leaf is a valid entry for the current leaf.
	nextEntry++;
}

// -----------------------------------------------------------------------------
// BTreeIndex::endScan
// -----------------------------------------------------------------------------
//
void BTreeIndex::endScan() 
{
	if (!scanExecuting) {
		throw ScanNotInitializedException();
	}

	scanExecuting = false;
	bufMgr->unPinPage(file, currentPageNum, false);
}

}
