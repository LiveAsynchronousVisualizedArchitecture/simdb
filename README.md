# simdb
A shared memory, lock free, cross platform, single file, no dependencies, C++11 key-value store






 SimDB

 What it does:
 |  SimDB is a key value store that uses arbitrary byte data (of arbitrary length) as both the key and the value. 
 |  It additionally uses shared memory, which allows processes to communicate with each other quickly.  
 |  It is lock free and scales well with multiple threads writing, reading, and deleting concurrently.  

 How it works:
 |-simdb:
   | This contains the user facing interface. It contains the ConcurrentHash, ConcurentStore, and SharedMem classes as members.
   | These data structures are made to be an interface over the contiguous memory given to them using a single address. 
   | They do not allocate any heap memory themselves, but do have a few class members that will be on the stack. At the time of this writing it is 176 bytes on the stack.
   |-SharedMem:
   |  |  Interface to OS specific shared memory functions.  Also handles an initial alignment.
   |-ConcurrentHash:
   |  |  Hash map that uses atomic operations on an array of VerIdx structs. 
   |  |  It uses 64 bit atomic operations to compare-exchange one VerIdx at a time (VerIdx is two unsigned 32 bit integers, a version and an index). 
   |  |  This makes sure that reading, writing and deleting is lock free. 
   |  |  Writing is lock free since a VerIdx is already fully created and written to before putting it in the VerIdx array (m_vis) and the put operation here is a single 64 bit compare and swap.    
   |  |  Deletion is lock free since the index in VerIdx is only freed from the CncrLst after setting the VerIdx here to DELETED. Actually deletion means 1. setting the VerIdx to DELETED 2. decrementing the readers of the blocklist that idx points to 3. If the readers variable of that blocklist is decremented below its initial value then the thread that took it below its initial value is the one to free it. 
   |  |  Get is lock free since it can read an index from a VerIdx, increment readers, compare its key to the key in the list of blocks, read the value in the blocks to the output buffer and finally decrement the readers variable. Just like deletion, if a thread decrements readers below its initial value, it needs to free the block list.  This means the last one out cleans up.
   |-ConcurrentStore:
   |  |  Keeps track of block lists.
   |  |  This primarily uses an array of BlkLst structs (which are 24 bytes each). 
   |  |  The BlkLst lava_vec is used to make linked lists of block indices. 
   |  |  The idea of a block list ends up being a starting index (from the VerIdx struct in the concurrent hash). The BlkLst struct at the starting index contains an index of the next BlkLst struct and so on until reaching a BlkLst that has an index of LIST_END. This means that one array contains multiple linked lists (using indices and not pointers of course).
   |  |  This exposes an alloc() function and a free() function. 
   |  |  alloc() gets the index of the next block from CncrLst (concurrent list).
   |  |  The BlkLst struct keeps the total length and the key length / value offset since it does not have to be atomic and is only initialized and used when one thread allocates and only destroyed when one thread frees, just like the actual data blocks.
   |-ConcurrentList:
   |  |  The concurrent list is an array integers.
   |  |  The number of elements (like all the arrays) is the number of blocks.
   |  |  There is one integer per block with the integer at a given index representing the next slot in the list.
   |  |  The end of the list will have value LIST_END.  On initialization the array's values would be |1|2|3|4| ... LIST_END, which makes a list from the start to the end. This means s_lv[0] would return 1.

 Terms:
 |-Block List: 
 |  A sequence of block indices.  The entry in ConcurrentHash gives the position in the block list array where the list starts.  
 |  The value at each index in the array contains the index of the next block.  
 |  The list end is know when a special value of LIST_END is found as the value in the array.
 |-Block List Version:
 |  This is a version number given to each block list on allocation (not each block). 
 |  It is used to link a ConcurrentHash value to the block list. 
 |  If the versions are the same, it is known that the block list at the index read from ConcurrentHash has not changed.
 |  This change could happen if:
 |  |  1. Thread ONE reads the entry in ConcurrentHash but has not accessed the block list index in the entry yet. Pretend that thread one stalls and nothing more happens until further down.
 |  |  2. Thread TWO has already allocated a block list and swaps its new entry for the old entry which is still carried by thread one. 
 |  |  3. Thread TWO now must free the block list given by the old entry, which it does, because no thread is reading it since thread one is still stalled.
 |  |  4. Thread TWO allocates another block list, which ends up using the blocks it just deallocated.
 |  |  5. Thread ONE wakes up and reads from the block index it found in the ConcurrentHash entry, which is no longer the same and may not even be the head of the list.
 |  |  If the index is used purely for matching the binary key, this wouldn't be a problem. 
 |  |  When the index is used to find a binary value however, this is a problem, since the length of a different value could be the same, and there would be no data to be able to tell that they are different.

 How it achieves lock free concurrency:
 |  ConcurrentHash is treated as the authority of what is stored in the database. 
 |  It has an array of VerIdx structs that can also be treated as 64 bit integers. Each is dealt with atomically.
 |  Its individual bits are used as a bitfied struct containing an index into ConcurrentStore's block list as well as the version number of that list.
 |  The core is m_vis, which is an array of VerIdx structs. The memory ordering is swapped on every other index in preparation for robin hood hashing techniques, so the actual memory layout (separated into 128 bit chunks) is |Index Version Version Index|Index Version Version Index| 
 |-Finding a matching index: 
 |  |  1. Use the hash of the key bytes to jump to an index.
 |  |  2. Load the integer atomically from that index and treat it as a VerIdx struct.
 |  |  3. Use the index from that struct to read the bytes from the list of blocks in BlkLst. 
 |  |  4. Increment the readers variable atomically, so that it won't be deleted before this thread is done with it.
 |  |  5. If there is a match, keep reading the list of blocks to fill the output buffer with the value section of the block list.
 |  |  6. After, decrement the readers variable atomically.  If readers goes below its initial value, this thread will be the one to free the block list.

 Other notables:
 | All of the main classes have a static sizeBytes() function that takes in the same arguments as a constructor and return the number of bytes that it will need in the shared memory
 | Classes have member variables that are used as interfaces to the shared memory denoted with s_ (s for shared)
 | Normal member variables that are just data on the stack are denoted with m_ (m for member)

 _________________
 | Memory Layout | 
 -----------------
   ______________________________________________________________________________________________________________________
   |Flags|BlockSize|BlockCount|ConcurrentHash|ConcurrentStore|ConcurentList|...BlockCount*BlockSize bytes for blocks....|
       _____________________________/               \_______       \______________________________________________________
 ______|____________________________________   ____________|_________________________________________________    ________|___________________________________________
 |size(bytes)|...array of VerIdx structs...|   |Block List Version|size(bytes)|...array of BlkLst structs...|    |size(bytes)|...array of unsigned 32 bit ints (u32)|


 First 24 bytes (in 8 byte / unsigned 64 bit chunks): 
     ____________________________ 
     |Flags|BlockSize|BlockCount|
  
  Flags:      Right now holds count of the number of processes that have the db open.  When the count goes to 0, the last process will delete the shared memory file.
  BlockSize:  The size in bytes of a block.  A good default would be to set this to the common page size of 4096 bytes.  
  BlockCount: The number of blocks.  This hash table array, block list array and concurrent list array will all be the same length.  This multiplied by the BlockSize will give the total amount of bytes available for key and value data. More blocks will also mean the hash table will have less collisions as well as less contention between threads.

