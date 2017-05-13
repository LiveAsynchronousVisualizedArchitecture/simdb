
# simdb
A high performance, shared memory, lock free, cross platform, single file, no dependencies, C++11 key-value store.

simdb is part of LAVA (Live Asynchronous Visualized Architecture) which is a series of single file, minimal dependency, C++11 files to create highly concurrent software while the program being written runs live with internal data visualized.

> simdb db("test", 1024, 4096);

This creates a shared memory file that will be named "simdb_test". It will be a file in a temp directory on Linux and OSX and a 'section object' in the current windows session namespace (basically a temp file complicated by windows nonsense).

It will have 4096 blocks of 1024 bytes each.  It will contain about 4 megabytes of space in its blocks and the actual file will have a size of about 4MB + some overhead for the organization (though the OS won't write pages of the memory map to disk unless it is neccesary). 

> auto dbs = simdb_listDBs();

This will return a list of the simdb files in the temp directory as a std::vector of std::string.  Simdb files are automatically prefixed with "simdb_" and thus can searched for easily here.  This can make interprocess communication easier so that you can do things like list the available db files in a GUI.  It is here for convenience largely because of how difficult it is to list the temporary memory mapped files on windows. 

> db.put("lock free", "is the way to be");

Simdb works with arbitrary byte buffers for both keys and values. This example uses a convenience function to make a common case easier. 

> string s = db.get("lock free");

This is another convenience function for the same reason. Next will be an example of the direct functions that these wrap.

> string lf  = "lock free";

> string way = "is the way to be";

>  i64    len = db.len( lf.data(), (u32)lf.length() );

>  string way2(len,'\0');

>  bool    ok = db.get( lf.data(), (u32)lf.length(), (void*)way.data(), (u32)way.length() );


Here we can see the fundamental functions used to interface with the db. An arbitrary bytes buffer is given for the key and another for the value.  Keep in mind here that get() can fail, since another thread can delete or change the key being read between the call to len() (which gets the number of bytes held in the value of the given key) and the call to get().
Not shown is del(), which will take a key and delete it.


#### Inside simdb.hpp there is a more extensive explanation of the inner working and how it achieves lock free concurrency


