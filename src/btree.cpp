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
	/// -1
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
		
		// insert entries
		// Do we need to set the rootpage to NULL for all values?
		// The first(smallest) key mut be -inf
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

void BTreeIndex::insertUnderNode(RIDKeyPair<int>* entry, Page* cur_page, bool is_leaf, PageKeyPair<int>* new_child)
{
	if (is_leaf)
	{
		LeafNodeInt* cur_node = reinterpret_cast<LeafNodeInt*>(cur_page);
		/// find the last record's position
		int last_record = 0;
		for (; last_record < INTARRAYLEAFSIZE; last_record++) {
			if (cur_node->ridArray[last_record].page_number == 0) {
				break;
			}
		}
		/// find the position to insert
		/// No duplicate keys, so we don't need <= or >=
		int pos = 0;
		for (; pos < this->leafOccupancy; pos++) {
			if ((pos == this->leafOccupancy - 1) ||
				(entry->key < cur_node->keyArray[pos] ) ||
				(cur_node->ridArray[pos+1].page_number == 0)) {
				break;
			}
		}
		copyArray<int>(cur_node->keyArray + pos, cur_node->keyArray + pos + 1, last_record - 1 - pos);
		copyArray<RecordId>(cur_node->ridArray + pos, cur_node->ridArray + pos + 1, last_record - 1 - pos);
		cur_node->keyArray[pos] = entry->key;
		/// Can I copy struct directly?
		cur_node->ridArray[pos] = entry->rid;
		/// For every leafnode, we restrict the max number or records we store to INTARRAYLEAFSIZE-1
		if (last_record == this->leafOccupancy - 1) {
			/// We split it half-half. For odd lengths, we allocate the left leaf one more record.
			Page* new_page = new Page;
			PageId new_page_id;
			this->bufMgr->allocPage(this->file, new_page_id, new_page);
			LeafNodeInt* new_right_sibling = reinterpret_cast<LeafNodeInt*>(new_page);
			copyArray<int>(cur_node->keyArray + (INTARRAYLEAFSIZE+1)/2, new_right_sibling->keyArray, INTARRAYLEAFSIZE/2);
			copyArray<RecordId>(cur_node->ridArray + (INTARRAYLEAFSIZE+1) / 2, new_right_sibling->ridArray, INTARRAYLEAFSIZE / 2);
			new_right_sibling->rightSibPageNo = cur_node->rightSibPageNo;
			cur_node->rightSibPageNo = new_page_id;
			new_child->set(new_page_id, new_right_sibling->keyArray[0]);
		}
		new_child->set(0, entry->key);
	}
	else {
		/// Nonleaf could be empty
		NonLeafNodeInt* cur_node = reinterpret_cast<NonLeafNodeInt*>(cur_page);
		/// find the position to insert
		/// No duplicate keys, so we don't need <= or >=
		int pos = 0;
		for (; pos < this->leafOccupancy; pos++) {
			if ((pos == this->leafOccupancy - 1) ||
				(entry->key < cur_node->keyArray[pos]) ||
				(cur_node->pageNoArray[pos+1] == 0)) {
				break;
			}
		}
		Page* child_page;
		bufMgr->readPage(this->file, cur_node->pageNoArray[pos], child_page);
		this->insertUnderNode(entry, child_page, cur_node->level, new_child);
		

		if (new_child->pageNo == 0) {
			return;
		}

		/// find the last record's position
		int last_record = 0;
		for (; last_record < INTARRAYLEAFSIZE; last_record++) {
			if (cur_node->pageNoArray[last_record] == 0) {
				break;
			}
		}

		copyArray<int>(cur_node->keyArray + pos, cur_node->keyArray + pos + 1, last_record - 1 - pos);
		copyArray<PageId>(cur_node->pageNoArray + pos, cur_node->pageNoArray + pos + 1, last_record - 1 - pos);
		cur_node->keyArray[pos] = new_child->key;
		/// Can I copy struct directly?
		cur_node->pageNoArray[pos] = new_child->pageNo;
		/// For every leafnode, we restrict the max number or records we store to INTARRAYLEAFSIZE-1
		if (last_record == this->leafOccupancy - 1) {
			/// We split it half-half. For odd lengths, we allocate the left leaf one more record.
			Page* new_page = new Page;
			PageId new_page_id;
			this->bufMgr->allocPage(this->file, new_page_id, new_page);
			NonLeafNodeInt* new_right_sibling = reinterpret_cast<NonLeafNodeInt*>(new_page);
			copyArray<int>(cur_node->keyArray + (INTARRAYLEAFSIZE + 1) / 2, new_right_sibling->keyArray, INTARRAYLEAFSIZE / 2);
			copyArray<PageId>(cur_node->pageNoArray + (INTARRAYLEAFSIZE + 1) / 2, new_right_sibling->pageNoArray, INTARRAYLEAFSIZE / 2);
			new_right_sibling->level = cur_node->level;
			new_child->set(new_page_id, new_right_sibling->keyArray[0]);
		}
		new_child->set(0, entry->key);
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
	
	if (new_child->pageNo == 0) {
		return;
	}

	Page* new_root_page = new Page;
	PageId new_root_page_id;
	this->bufMgr->allocPage(this->file, new_root_page_id, new_root_page);
	NonLeafNodeInt* new_root_node = reinterpret_cast<NonLeafNodeInt*>(new_root_page);
	new_root_node->keyArray[0] = -std::numeric_limits<int>::max();
	new_root_node->pageNoArray[0] = this->rootPageNum;
	new_root_node->keyArray[1] = new_child->key;
	new_root_node->pageNoArray[1] = new_child->pageNo;
	this->rootPageNum = new_root_page_id;
	return;
}

// -----------------------------------------------------------------------------
// BTreeIndex::startScan
// -----------------------------------------------------------------------------


void BTreeIndex::scanPage(Page* cur_page, bool is_leaf, ) {
	if (is_leaf) {
		LeafNodeInt* cur_node = reinterpret_cast<LeafNodeInt*>(cur_page);
		int pos;

		///...

		this->nextEntry = pos;
	}
	else {
		NonLeafNodeInt* cur_node = reinterpret_cast<NonLeafNodeInt*>(cur_page);
		Page* child_page;
		PageId child_pageId;
		int pos;

		///.....

		bufMgr->readPage(this->file, child_pageId, child_page);

		bool leaf_child = cur_node->level;
		if (leaf_child) {
			this->currentPageNum = cur_node->pageNoArray[pos];
			this->currentPageData =
				bufMgr->pin(curpage);
		}
	}
}

void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{
	if (scanExecuting) {
		endScan();
	}
	this->lowOp = lowOpParm;
	this->highOp = highOpParm;
	this->lowValInt = *((int*)lowValParm);
	this->highValInt = *((int*)highValInt);

	Page* root_page;
	bufMgr->readPage(this->file, this->rootPageNum, root_page);

	Page* meta_page;
	bufMgr->readPage(this->file, this->headerPageNum, meta_page);
	IndexMetaInfo* meta_info = reinterpret_cast<IndexMetaInfo*>(meta_page);
	bool is_leaf = (meta_info->height == 1);
	

	scanPage(root_page, is_leaf);
	
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
