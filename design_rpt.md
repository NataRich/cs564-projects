# BadgerDB Design Report
Authors:
- Zihan Zhao (zzhao383@wisc.edu)
- Haishuo Chen (hchen727@wisc.edu)
- Qinhang Wu (qinhang.wu@wisc.edu)

## Implementation
### Constructor `BTreeIndex::BTreeIndex`
Generally, the constructor initializes some private data members of the bTree class and constructs headerPage and rootPage separately for either existed index file or brand new relation file. For existed index file, the buffer manager will first read the headerPage from file, and then unpin it after retrieving the information from it. For new relation file, the buffer manager will allocate new pages for the headerPage and rootPage separately and unpin them right after the corresponding information have been inserted.

## Additional Tester
We implemented additional tester functions in `main.cpp` to test our design extensively. `test5()` searches for key from -1000 to 6000 as a boundary check of the scanning functionality. `test6()` reopens an index file after it was saved to test the construction of a b+ tree based on an existed one. `test7()` constructs a b+ tree based on 50 thousand records to force the non-leaf node to split. Our program passes all the given tests as well as the self-implemented ones.
