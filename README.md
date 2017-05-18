
![alt text](https://github.com/LiveAsynchronousVisualizedArchitecture/simdb/blob/master/numbered_slots_upshot.jpg "A key value store is kind of like this")

# SimDB
#### A high performance, shared memory, lock free, cross platform, single file, no dependencies, C++11 key-value store.

SimDB is part of LAVA (Live Asynchronous Visualized Architecture) which is a series of single file, minimal dependency, C++11 files to create highly concurrent software while the program being written runs live with internal data visualized.

- Hash based key-value store created to be a fundamental piece of a larger software architecture. 

- High Performance - Real benchmarking needs to be done, but superficial loops seem to run *conservatively* at 500,000 small get() and put() calls per logical core per second. Because it is lock free the performance scales well while using at least a dozen threads. 

- Shared Memory - Uses shared memory maps on Windows, Linux, and OS X without relying on any external dependencies.  This makes it __exceptionally good at interprocess communication__. 

- Lock Free - The user facing functions are thread-safe and lock free with the exception of the constructor (to avoid race conditions between multiple processes creating the memory mapped file at the same time). 

- Cross Platform - Compiles with Visual Studio 2013 and ICC 15.0 on Windows, gcc 5.4 on Linux, gcc on OS X, and clang on OS X.

- Single File - simdb.hpp and the C++11 standard library is all you need. No Windows SDK or any other dependencies, not even from the parent project. 

- Apache 2.0 License - No need to GPL your whole program to include one file. 

This has already been used for both debugging and visualization, but *should be treated as alpha software*.  Though there are no known outstanding bugs, there are almost certainly bugs (and small design issues) waiting to be discovered and so will need to be fixed as they arise. 

#### Getting Started

```cpp
simdb db("test", 1024, 4096);
```

This creates a shared memory file that will be named "simdb_test". It will be a file in a temp directory on Linux and OSX and a 'section object' in the current windows session namespace (basically a temp file complicated by windows nonsense).

It will have 4096 blocks of 1024 bytes each.  It will contain about 4 megabytes of space in its blocks and the actual file will have a size of about 4MB + some overhead for the organization (though the OS won't write pages of the memory map to disk unless it is neccesary). 

```cpp
auto dbs = simdb_listDBs();
```

This will return a list of the simdb files in the temp directory as a std::vector of std::string.  Simdb files are automatically prefixed with "simdb_" and thus can searched for easily here.  This can make interprocess communication easier so that you can do things like list the available db files in a GUI.  It is here for convenience largely because of how difficult it is to list the temporary memory mapped files on windows. 

```cpp
db.put("lock free", "is the way to be");
```

SimDB works with arbitrary byte buffers for both keys and values. This example uses a convenience function to make a common case easier. 

```cpp 
string s = db.get("lock free");       // returns "is the way to be"
```

This is another convenience function for the same reason. Next will be an example of the direct functions that these wrap.

```cpp
string lf  = "lock free";

string way = "is the way to be";

i64    len = db.len( lf.data(), (u32)lf.length() );

string way2(len,'\0');

bool    ok = db.get( lf.data(), (u32)lf.length(), (void*)way.data(), (u32)way.length() );
```

Here we can see the fundamental functions used to interface with the db. An arbitrary bytes buffer is given for the key and another for the value.  Keep in mind here that get() can fail, since another thread can delete or change the key being read between the call to len() (which gets the number of bytes held in the value of the given key) and the call to get().
Not shown is del(), which will take a key and delete it.


*Inside simdb.hpp there is a more extensive explanation of the inner working and how it achieves lock free concurrency*


