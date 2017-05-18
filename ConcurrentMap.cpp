
// todo: test 128 bit atomic with native C++
// todo: look into 128 bit atomic with windows

#ifdef _MSC_VER
 #pragma warning(push, 0)
#endif

#include <stdint.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include <regex>
#include <random>
#include <iostream>
#include <sstream>
#include <thread>
#include "simdb.hpp"

using    u8   =   uint8_t;
using   u32   =   uint32_t;
using   u64   =   uint64_t;
using    i8   =   int8_t;
using   i32   =   int32_t;
using   i64   =   int64_t;
using  au64   =   std::atomic<u64>;
using  au32   =   std::atomic<u32>;

#ifdef _WIN32
  #include <intrin.h>
  #include <windows.h>
#endif

//#include <SIM/SIM_GeneralTemplateUtil.hpp>

#ifndef COMBINE
  #define COMBINE2(a,b) a ## b
  #define COMBINE(a,b) COMBINE2(a,b)
#endif

#ifndef PAUSE
  #define PAUSE std::cout << "Paused at line " << __LINE__ << std::endl; int COMBINE(VAR,__LINE__); std::cin >> COMBINE(VAR,__LINE__);
#endif

#ifndef TO
  #define TO(to, var) for(std::remove_const<decltype(to)>::type var = 0; var < to; ++var)
  //#define TO(to, var) for(auto var = 0ull; var < (unsigned long long)to; ++var)
#endif

u32       intHash(u32    h)
{
  //h += 1;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}
u32  nextPowerOf2(u32    v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  return v;
}

template <typename T> struct RngInt
{
  std::mt19937                       m_gen;
  std::uniform_int_distribution<T>   m_dis;
  
  RngInt(T lo = 0, T hi = 1, int seed = 16807)
   : m_gen(seed), m_dis(lo, hi)
  { }

  inline T operator()()
  { return m_dis(m_gen); }

  inline T operator()(T lo, T hi)
  {
    std::uniform_int_distribution<T>  dis(lo, hi); 
    return dis(m_gen);
  }
};

template<class STR>
STR keepAlphaNumeric(STR const& s)
{
  using namespace std;

  regex            alphaNumeric("[a-zA-Z\\d]+");
  sregex_iterator  iter( ALL(s), alphaNumeric );
  sregex_iterator  iter_end;

  STR out;
  while( iter != iter_end )
    out += iter++->str();      // ...

  return out;
}

template<class STR1, class STR2>
STR1 subNonFilename(STR1 const& s, STR2 const& substr)
{
  using namespace std;

  //string    patStr("[#%&\\{\\}\\\\<>\\*\\?/\\w\\$!'\":@\\+`\\|=\\.]+");
  //string    patStr("#|%|&|\\{|\\}|\\\\|<|>|\\*|\\?|/|\\w|\\$|!|'|\"|:|@|\\+|`|\\||=|\\.");

  STR1      patStr(":|\\*|\\.|\\?|\\\\|/|\\||>|<");
  regex     pattern(patStr);
  return regex_replace(s, pattern, substr);
}

template<class T> inline auto
Concat(const T& a) -> T
{ return a; }
template<class T1, class... T> inline auto
Concat(const T1& a, const T&... args) -> T1
{
  //T1 ret;
  //ret.append( ALL(a) );
  //ret.append( ALL(Concat(args...)) );
  return a + Concat(args...);
}

inline std::string 
toString(std::vector<std::string> const& v)
{
  using namespace std;
  
  ostringstream convert;
  TO(v.size(),i) convert << v[i] << " ";
  convert << endl;
  return convert.str();
}

template<class T> inline std::string 
toString(T const& x)
{
  std::ostringstream convert;
  convert << x;
  return convert.str();
}

template<class T1, class... T> inline std::string
toString(const T1& a, const T&... args)
{
  return toString(a) + toString(args...) ;
}

//template< template<class...> class L, class... T, int IDX = 0> std::string 
//toString(const std::tuple<T...>& tpl)
//{
//  using namespace std;
//  
//  const auto len = mp_len<T...>::value;
//  
//  string ret;
//  ret  +=  toString(get<IDX>(tpl), " ");
//  if(IDX < len-1) ret  += toString(get<IDX+1>(tpl));
//  return ret;
//}

inline std::ostream&  Print(std::ostream& o) { return o; }
template<class... T> inline std::ostream&
 Print(std::ostream& o, const T&... args)
{
  o << toString(args ...);
  o.flush();
  return o;
}
template<class... T> inline std::ostream&
 Println(std::ostream& o, const T&... args)
{
  //o << toString(args...) << std::endl;
  Print(o, args..., "\n");
  return o;
}
template<class... T> inline void
 Print(const T&... args)
{
  Print(std::cout, args...);
  //std::cout << toString(args...);
}
template<class... T> inline void
 Println(const T&... args)
{
  Println(std::cout, args...);
  //std::cout << toString(args...) << std::endl;
}
template<class T> inline void
 PrintSpaceln(const T& a)
{
  Print(std::cout, a);
}
template<class T1, class... T> inline void
 PrintSpaceln(const T1& a, const T&... args)
{
  Print(std::cout, a, " ");
  PrintSpaceln(args...);
  Println();
}

using std::thread;
using str   =   std::string;

//inline void prefetch2(char const* const p)
//{
//  _mm_prefetch(p, _MM_HINT_T2);
//  //_m_prefetch((void*)p);
//}
//inline void prefetch1(char const* const p)
//{
//  _mm_prefetch(p, _MM_HINT_T1);
//  //_m_prefetch((void*)p);
//}
//inline void prefetch0(char const* const p)
//{
//  _mm_prefetch(p, _MM_HINT_T0);
//  //_m_prefetch((void*)p);
//}

template<class T, class A=std::allocator<T> > using vec = std::vector<T, A>;  // will need C++ ifdefs eventually

void printkey(simdb const& db, str const& key)
{
  //u32 vlen;
  //auto len = db.len(key, &vlen);
  //
  //auto val = str(vlen, '\0');
  //auto  ok = db.get(key);

  auto val = db.get(key);
  Println(key,": ", val);
}

void printdb(simdb const& db)
{
  Println("size: ", db.size());

  //str memstr;
  //memstr.resize(db.size()+1);

  vec<i8> memv(db.memsize(), 0);
  memcpy( (void*)memv.data(), db.mem(), db.memsize() );

  //str memstr( (const char*)db.data(), (const char*)db.data() + db.size());
  //Println("\nmem: ", memstr, "\n" );

  Println("\n");

  u64 blksz = db.blockSize();
  TO(memv.size(),i){ 
    if(i % blksz == 0){
      putc('|', stdout);
    }
    putc(memv[i] ,stdout);
  }
}

void printkeys(simdb const& db)
{
  Println("\n---Keys---");
  auto keys = db.getKeyStrs();
  TO(keys.size(), i){
    Println(keys[i].str, ": ", db.get(keys[i].str) );
    //printkey(db, db.get(keys[i].s) );
  }
  
  //printkey(db, keys[i].s );
}

void printhsh(simdb const& db)
{
  u32* d = (u32*)db.hashData();
  for(u32 i=0; i<(db.blocks()*2); ++i){
    if(i%4==0) printf("|");
    else if(i%2==0) printf(" ");

    printf(" 0x%08x ", d[i]);

    //if(i%8) printf("|");
    //else if(i%4) printf(" ");
  }
  printf("\n\n");

  //auto vi = (simdb::VerIdx*)db.hashData();
  //for(u32 i=0; i<(db.blocks()); ++i){
  //  if(i%2==0) printf("|");
  //
  //  printf(" %u %u ", vi[i].idx, vi[i].version);
  //
  //  //if(i%8) printf("|");
  //  //else if(i%4) printf(" ");
  //}
  //printf("\n\n");
}

int main()
{
  using namespace std;

  //Println("size of simdb on the stack: ", sizeof(simdb));

  simdb db("test", 2<<10, 2<<12);

  //simdb db("H:\\projects\\lava\\test.simdb", 32, 64, true);
  //simdb db("test.simdb", 32, 64, true);

  //printhsh(db);

  str numkey[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};
  str  label[] = {"zero","one","two","three","four","five","six","seven","eight","nine","ten","eleven"};
  
  //int sz = (int)thrds.size(); 

  //int sz = 12;
  //vec<thread>            thrds;
  //vec<RngInt<u32>> rngSwitches;
  //TO(sz,i){ rngSwitches.emplace_back(0,1,i); }
  //  
  //TO(sz,i)
  //{
  //  int idx = i % sz;
  //  thrds.emplace_back([i, idx, &rngSwitches, &numkey, &label, &db]
  //  {
  //    auto& numk = numkey[idx];
  //    auto&  lbl = label[idx]; 
  //    TO(1,j){ 
  //      db.put(numk, lbl); 
  //      if(rngSwitches[idx]()){ db.del(numk); }
  //      //bool ok = db.del(numk);
  //      //if(!ok){ Println(numk," not deleted"); }
  //      //while(!db.del(numk)){}
  //    }
  //    
  //    Println(i, " done");
  //  });
  //}
  //TO(thrds.size(),i){ thrds[i].join(); }

  str       wat  =       "wat";
  str       wut  =       "wut";
  str  skidoosh  =  "skidoosh";
  str    kablam  =    "kablam";
  str   longkey  =  "this is a super long key as a test";
  str   longval  =  "value that is really long as a really long value test";

  string  lf = "lock free";
  string way = "is the way to be";
  //db.put( lf.data(), (u32)lf.length(), way.data(), (u32)way.length() );
  
  i64    len = db.len( lf.data(), (u32)lf.length() );
  string way2(len,'\0');
  /*bool    ok =*/ db.get( lf.data(), (u32)lf.length(), (void*)way.data(), (u32)way.length() );

  Println("\n",way,"\n");

  //Println("put: ", db.put( wat.data(),   (u32)wat.length(),    skidoosh.data(), (u32)skidoosh.length()) );
  //Println("put: ", db.put( (void*)wat.data(),   (u32)wat.length(),    (void*)skidoosh.data(), (u32)skidoosh.length()) );

  if( db.isOwner() ){
    Println("put: ", db.put("lock free", "is the way to be") );

    Println("put: ", db.put(wat, skidoosh) );
    //db.del("wat");
    Println("put: ", db.put( wut.data(),   (u32)wut.length(),    kablam.data(),   (u32)kablam.length())   ); 
    //db.del("wut");
    Println("put: ", db.put(kablam, skidoosh) ); //Println("put: ", db.put( kablam.data(),(u32)kablam.length(), skidoosh.data(), (u32)skidoosh.length()) ); 
    //db.del("kablam");
  
    Println("put: ", db.put(wat, skidoosh) );
    //Println("del wat: ", db.del("wat") );
  
    //Println("put: ", db.put(longkey, longval) );
    //Println("del wat: ", db.del(longkey) );
  
    Println();
  }
  //db.flush();

  Println();
  //printdb(db);

  //else{
  //  Println("put: ", db.put( (void*)wat.data(),   (u32)wat.length(),    (void*)skidoosh.data(), (u32)skidoosh.length()) );
  //}

  Println();
  //printhsh(db); // Println();

  //Println("BlkLst size: ", sizeof(CncrHsh::BlkLst) );
  //
  //u32 vlen=0,ver=0;
  //i64  len = db.len( wat.data(), (u32)wat.length(), &vlen, &ver);
  //str val(vlen, '\0');
  //bool  ok = db.get( wat.data(), (u32)wat.length(), (void*)val.data(), (u32)val.length() );
  //Println("ok: ", ok, " value: ", val, "  wat total len: ", len, " wat val len: ", vlen, "\n");
  //
  //len = db.len( longkey.data(), (u32)longkey.length(), &vlen, &ver);
  //val = str(vlen, '\0');
  //ok  = db.get( longkey.data(), (u32)longkey.length(), (void*)val.data(), (u32)val.length() );
  //Println("ok: ", ok, " longkey value: ", val, "  longkey total len: ", len, " longkey val len: ", vlen, "\n");
  //
  //str v; 
  //db.get(wat,   &v);  Println("value: ", v);
  //db.get(wut,   &v);  Println("value: ", v);
  //db.get(kablam,&v);  Println("value: ", v);

  printkeys(db);

  //Println("\nKEYS");
  //auto keys = db.getKeyStrs();
  //for(auto k : keys) Println(k,":  ", db.get(k) );
  //Println("\n");

  //TO(6,i)
  //{  
  //  u32 klen, vlen;
  //  auto   nxt = db.nxt();
  //  bool oklen = db.len(nxt.idx, nxt.version, &klen, &vlen);
  //  str key(klen,'\0');
  //  bool okkey = db.getKey(nxt.idx, nxt.version, (void*)key.data(), klen);
  //
  //  //str val;
  //  //bool okval = db.get(key, &val);
  //  str val = db.get(key);
  //  Println("VerIdx: ",nxt.idx,", ",nxt.version,
  //          " str len: ", key.length(), "  nxt key: [", key, 
  //          "] val: [", val,"] val len: ", val.length() );
  //}

  //Println("wat data len: ",    db.len(wat)    );
  //Println("wut data len: ",    db.len(wut)    );
  //Println("kablam data len: ", db.len(kablam) );
  //
  //str clear = "                ";
  //auto watlen = db.get("wat",      (void*)clear.data() );
  ////auto watslen = db.get(str("w"), (void*)clear.data() );
  //Println("watlen: ", watlen);
  //Println("get \"wat\": ", clear);
  //Println();
  //
  //clear = "                ";
  //auto wutlen = db.get("wut", (void*)clear.data() );
  //Println("wutlen: ", wutlen);
  //Println("get \"wut\": ", clear);
  //Println();
  //
  //clear = "                ";
  //auto kablamlen = db.get("kablam", (void*)clear.data() );
  //Println("kablamlen: ", kablamlen);
  //Println("get \"kablam\": ", clear);
  //Println();
  //

  //Println("size: ", db.size());
  //
  ////str memstr;
  ////memstr.resize(db.size()+1);
  //vec<i8> memv(db.memsize(), 0);
  //memcpy( (void*)memv.data(), db.mem(), db.memsize() );
  //
  //str memstr( (const char*)db.data(), (const char*)db.data() + db.size());
  //Println("\nmem: ", memstr, "\n" );

  //
  //Println("owner: ", db.isOwner(), "\n\n");
  //
  ////std::vector<i64>::value_type v;
  ////Println("v size: ", sizeof(v));
  //
  ////ui64    cnt = (ui64)((1<<17)*1.5);
  //ui64    cnt = (1<<16);
  ////ui64  bytes = lava_vec<i32>::sizeBytes(cnt);
  ////void*   mem = malloc( bytes ); 
  ////lava_vec<i32> lv(mem, cnt);
  //
  //auto lv = STACK_VEC(i64, cnt);
  ////memset(lv.data(), 0, 16*sizeof(u32) );
  //Println("capacity: ",  lv.capacity() );
  //Println("size: ",      lv.size() );
  //Println("sizeBytes: ", lv.sizeBytes() );
  //TO(lv.size(), i) lv[i] = i;
  //cout << lv[lv.size()-1] << " ";
  ////TO((i32)lv.size(), i) Print(" ",i,":",lv[i]);
  ////TO(lv.size(), i) cout << lv[i] << " ";
  ////lv.~lava_vec();  // running the destructor explicitly tests double destrucion since it will be destructed at the end of the function also

  auto dbs = simdb_listDBs();
  Println("\n\n db list");
  //TO(dbs.size(),i) wcout << dbs[i] << "\n";
  TO(dbs.size(),i){ cout << dbs[i] << "\n"; }
  Println("\n\n");


  Println("\n\n DONE \n\n");
  PAUSE
  db.close();
  Println("\n\n CLOSED \n\n");
  PAUSE

  return 0;
}

#ifdef _MSC_VER
 #pragma warning(pop)
#endif







//      template<class T, class _Alloc=std::allocator<T> > 
//using vec        =  std::vector<T, _Alloc>;

//struct _u128 { u64 hi, lo; };
//using u128 = __declspec(align(128)) volatile _u128;

//SECTION(128 bit atomic compare and exchange)
//{
//  u128 dest = { 101, 102 };
//  u128 comp = { 101, 102 };
//  u128 desired = { 85, 86 };
//
//  _InterlockedCompareExchange128(
//    (i64*)(&dest), 
//    desired.hi,
//    desired.lo,
//    (i64*)(&comp) );
//
//  Println("dest: ", dest.hi, " ", dest.lo);
//  Println("comp: ", comp.hi, " ", comp.lo);
//  Println("\n\n");
//
//  //u128 dest = { 101, 102 };
//  //u128 comp = { 101, 102 };
//  //u128 desired = { 85, 86 };
//
//  _InterlockedCompareExchange128(
//    (i64*)(&dest), 
//    desired.hi,
//    desired.lo,
//    (i64*)(&comp) );
//
//  Println("dest: ", dest.hi, " ", dest.lo);
//  Println("comp: ", comp.hi, " ", comp.lo);
//  Println("\n\n");
//}

//u32 sz = 18921703;
//u32 sz = 400;
//ConcurrentMap cm(sz);

//ScopeTime t;
//t.start();
//cm.init(sz);
//t.stop("Init");

//Println( (i64)t.stop() );
//t.start();
//
//Println("sz: ", cm.size());
//
//TO(100,i) {
//  Println("i: ",i," ", intHash(i) );
//}
//
//TO(100,i) {
//  Println("i: ",i," ", nextPowerOf2(i));
//}

//
//u32 loopSz  =  (u32)( double(cm.size()) / 1.5);

//
//RngInt<int> rng(1, 2);

//vec<thread> thrds;
//TO(5,tid)
//{
//  thrds.push_back( thread([&cm, &rng, loopSz, tid]()
//  {
//    ScopeTime t;
//
//    t.start();
//    TO(loopSz, i) {
//      auto val = i*10 + tid;
//      u32 pidx = cm.put(i, val);
//      SleepMs( rng() );
//
//      //SleepMs( (int)pow(4-tid,2) );
//      //cout << pidx << "  ";
//      //Println("Put Idx: ", (i64)pidx);
//    }
//    t.stop("Put");
//    //Println( t.getSeconds() );
//
//    //t.start();
//    //TO(loopSz, i) {
//    //  u32 gidx = cm.get(i);
//    //  cout << gidx << "  ";
//    //  //Println("Get Idx: ", (i64)gidx);
//    //}
//    //t.stop("Get");
//    //Println( t.getSeconds() );
//  })); // .detach();
//  //thrds.back().detach();
//}
////for(auto& th : thrds) th.detach();
//for(auto& th : thrds) th.join();

// test getting back from the map
//t.start();
//TO(loopSz, i) {
//  u32 gidx = cm.get(i);
//  cout << gidx << "  ";
//  //Println("Get Idx: ", (i64)gidx);
//}
//Println();
//t.stop("Get");

//RngInt<int> rngb(0,1);
//RngInt<int> rngl(0,loopSz-1);
//ConcurrentList cl(loopSz);

//// serial test of ConcurrentList
////t.start();
////TO(loopSz, i){
////  Print(cl.idx(),":", cl.alloc(), " ");
////}
////TO(loopSz, i){
////  Print(cl.idx(),":", cl.free(i), " ");
////}
////Println();
////auto lv = cl.list();
////TO(lv->size(), i){
////  Print( i,":",(*lv)[i], " ");
////}
////Println();
////t.stop("List");

//Println("\nLinks: ", cl.lnkCnt(), " ");
//vec<thread> thrds;
//TO(12,tid)
//{
//  thrds.push_back( thread([&cl, &rngb, &rngl, loopSz, tid]()
//  {
//    ScopeTime t;

//    t.start();
//    TO(loopSz/5, i){
//      //if(rngb()) 
//      Print(tid,":",cl.nxt()," ");
//      //else       Print(tid,":",cl.free(rngl()), " ");
//      SleepMs( rngb() );
//    }
//    t.stop("alloc/free");
//  }));
//}
//for(auto& th : thrds) th.join();

//Println();
//auto lv = cl.list();
//TO(lv->size(), i){
//  Print( i,":",(*lv)[i], " ");
//}
//Println();
//Println("\nLinks: ", cl.lnkCnt(), " ");

//i32 blkSz  = 5;
//i32 blocks = 2;
//vec<ui8> mem(blocks*blkSz, 0);
//ConcurrentStore cs(mem.data(), blkSz, (u32)(blocks) );
//
//Println("\n");
//
//TO(cs.m_cl.list()->size(), i){
//  Println( (*cs.m_cl.list())[i] );
//}
//Println("\n\n");
//
//TO(2,i){
//  i32  blks = 0;
//  auto    s = "w";
//  i32  slen = (i32)strlen(s)+1;
//  //i32  slen = 1;
//  auto  idx = cs.alloc(slen, &blks);   // must allocate the exact number of bytes and no more, since that number will be used to read and write
//  cs.put(idx, (void*)s, slen);
//
//  vec<char> gs(slen,0);
//  cs.get(idx, gs.data(), slen);
//  Println(gs.data());
//  cs.free(idx);
//
//  TO(cs.m_blockCount, b){
//    Println(cs.nxtBlock(b));
//  }
//  Println("\n\n");
//  TO(cs.m_cl.list()->size(), i){
//    Println( (*cs.m_cl.list())[i] );
//  }
//  Println("\n\n");
//
//}

//ConcurrentHash ch(64);
//vec<thread> thrds;
//TO(24,tid)
//{
//  thrds.push_back( thread([&ch, &rng, tid]()
//  {
//    TO(64,h)
//    {
//      ch.put(h, h*h);
//      //Print(h,": ", ch.put(h, h*h) );
//      Print(h,":", ch.get(h)==h*h, " ");
//    }
//  } ));
//  thrds.back().detach();
//}

//auto fileHndl = CreateFileMapping(
//  INVALID_HANDLE_VALUE,
//  NULL,
//  PAGE_READWRITE,
//  0,
//  0x0000FFFF,
//  "Global\\simdb_15");
//
//if(fileHndl==NULL){/*error*/}
//
//i32  memSz  = 256;
//auto mapmem = MapViewOfFile(fileHndl,   // handle to map object
//  FILE_MAP_ALL_ACCESS,   // read/write permission
//  0,
//  0,
//  memSz);

// OpenFileMapping if the file exists
//
//Println(fileHndl);
//Println("\n\n");
//Println(mapmem);

//
//Println("kv sz: ", sizeof(simdb::KV) );

//u32    isKey :  1;
//i32   readers : 31;

//union KeyAndReaders
//{
//  struct{ u32    isKey :  1; i32   readers : 31; };
//  u32 asInt;
//};
//union BlkLst
//{
//  struct { KeyAndReaders kr; u32 idx; };
//  ui64 asInt;
//};
//Println("KeyAndReaders sz: ", sizeof(KeyAndReaders) );
//Println("BlkLst sz: ",        sizeof(BlkLst) );

//union      KV         // 256 million keys (28 bits), 256 million values (28 bit),  255 readers (8 bits)
//{
//  struct
//  {
//    ui64     key : 28;
//    ui64     val : 28;
//    ui64 readers :  8;
//  };
//  ui64 asInt;
//};

//Println("KV sz: ", sizeof(ConcurrentHash::KV) );
//Println("empty kv: ", ConcurrentHash::empty_kv().key == ConcurrentHash::EMPTY_KEY );
//Println("empty kv: ", ConcurrentHash::EMPTY_KEY );

//Println("\n");
//struct ui128_t { uint64_t lo, hi; };
////struct ui128_t { uint64_t low; };

//bool lkFree = atomic<ui128_t>{}.is_lock_free();
//Println("is lock free 128: ",  lkFree );

//ui128_t a = {0, 101};
//i8 alignmem[256];
//void* mem = (void*)(alignmem+(128-((ui64)alignmem % 128)));
////Println("mem: ", mem, "  rem: ", ((ui64)mem)%128 );
//memcpy(mem, &a, sizeof(a));
//int ok = _InterlockedCompareExchange128((volatile long long*)mem, 202, 1, (long long*)&a);
//memcpy(&a, mem, sizeof(a));
//ui128_t* b = (ui128_t*)mem;
////Println("ok: [", ok, "]  lo: [", b->lo, "]  hi: [", b->hi, "]");

//auto sz = sizeof(ConcurrentStore::BlkLst);
//Println("Blklst sz: ", sz);

//
//Println("simdb stack sz: ", sizeof(simdb) );

//thrds[i] = move(thread( [i,&label,&db]
//RngInt<u32> rnd(i*10, ((i+1)*10)-1);
//RngInt<u32> rnd(0, 10, i);
//db.put( toString(rnd()), label[i] );
//thrds[i].detach();

//TO(10,i) Print(rnd(), " ");
//break;
