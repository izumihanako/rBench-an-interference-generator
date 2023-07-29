#ifndef RBENCH_H
#define RBENCH_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <map>
#include <ctime>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <sched.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
using std::pair ;
using std::make_pair ;
using std::mutex ;
using std::thread ;
using std::vector ;
using std::map ;
using std::ios ;
using std::fstream ;
using std::string ;
using std::shared_ptr ;

extern double timeval_to_double(const struct timeval *tv) ;
extern double time_now(void) ;

// used For help message output
struct help_info_t{
    const char *opt_short ;
    const char *opt_long ;
    const char *opt_alter ;
    const char *description ;
} ;

// arguments passed to every benchmarks
// unified interface
struct bench_args_t{
    uint32_t threads ;
    uint32_t limit_round ;
    uint32_t time ;
    uint32_t strength ; // run strength% time per period
    uint32_t flags ;
    uint32_t period ; // us 
    union{
        uint64_t mem_bandwidth ;
        uint64_t cache_size ;
    } ;
    bench_args_t(){
        threads = 1 ;
        limit_round = 0 ;
        time = 0 ;
        period = 100000 ;
        strength = 100 ;
        mem_bandwidth = 0 ;
        cache_size = 0 ;
        flags = 0 ;
    }
    void print_argsinfo() ;
} ;
typedef int32_t(* bench_func_t )( bench_args_t ) ;
extern uint32_t global_flag ;

// benchmark flag arguments
enum argflag_t{
    FLAG_PRINT_DEBUG_INFO = 0 ,
    FLAG_IS_LIMITED ,
    FLAG_COUNT ,
} ;
void clr_arg_flag( uint32_t& , argflag_t ) ;
void set_arg_flag( uint32_t& , argflag_t ) ;
bool get_arg_flag( const uint32_t , argflag_t ) ;

// Command line long options
enum argvopt_t{
    OPT_undefined = 0 , 
    // short options 
    OPT_limited        = 'l' ,
    OPT_mem_bandwidth  = 'b' ,
    OPT_ninstance      = 'n' ,
    OPT_run            = 'r' ,
    OPT_strength       = 's' ,
    OPT_time           = 't' ,
    // long options only
    OPT_long_ops_start = 0x7f ,
    OPT_cacheL1 ,
    OPT_cacheL2 ,
    OPT_cacheL3 ,
    OPT_cache_size ,
    OPT_check ,
    OPT_debug ,
    OPT_period ,
} ;

enum cpu_cache_type_t {
    CACHE_TYPE_UNKNOWN = 0,     /* Unknown type */
    CACHE_TYPE_DATA,            /* D$ */
    CACHE_TYPE_INSTRUCTION,     /* I$ */
    CACHE_TYPE_UNIFIED,         /* D$ + I$ */
} ;

// CPU cache information
struct cpucache_t{
    cpu_cache_type_t type ;
    uint32_t level ;
    uint32_t line_size ;        // Byte
    uint32_t ways ;             // number of slots of one set
    uint32_t sets ; 
    uint64_t size ;             // Byte
    shared_ptr<char> buffer ;   // not used, preserve space
} ;

// CPU information
struct cpuinfo_t {
    int32_t page_size ;         // Byte
    int32_t core_count ;
    int32_t online_count ;
    int32_t cache_count ;
    cpucache_t caches[10] ;
    cpuinfo_t() ;
    void read_cpuinfo() ;
    uint64_t get_data_cache_size_level( uint32_t ) ;
    void print_cpuinfo() ;
} ;
extern cpuinfo_t cpuinfo ;

/* Fast random numbers : Galois LFSR 32bit random numbers */
// http://www.cse.yorku.ca/~oz/marsaglia-rng.html
struct Galois_LFSR{
    uint32_t lfsr ;
    Galois_LFSR() ;
    void set_LFSR_number( uint32_t ) ;
    uint32_t lfsr32() ;
    uint64_t lfsr64() ;
} ;

/* Fast random numbers : Multiply-with-carry random numbers */
// MWC random number initial seed
#define MWC_SEED_W    (521288629UL)
#define MWC_SEED_Z    (362436069UL)
struct mwc_t {
    uint32_t w;
    uint32_t z;
    uint32_t n16;
    uint32_t saved16;
    uint32_t n8;
    uint32_t saved8;
    uint32_t n1;
    uint32_t saved1;

    mwc_t( uint32_t = MWC_SEED_W , uint32_t = MWC_SEED_Z ) ;

    void flush() ;
    void reseed() ;
    void set_seed( const uint32_t , const uint32_t ) ;
    void set_default_seed() ;
    void print_info() ;

    pair<uint32_t,uint32_t> get_seed() ;
    uint32_t mwc32() ;
    uint32_t mwc32modn( const uint32_t mmod ) ;
    uint32_t mwc32modn_maybe_pwr2( const uint32_t mmod ) ;
    uint16_t mwc16() ;
    uint16_t mwc16modn( const uint16_t mmod ) ;
    uint64_t mwc64() ;
    uint64_t mwc64modn( const uint64_t mmod ) ;
    uint8_t mwc8() ;
    uint8_t mwc8modn( const uint8_t mmod ) ;
    uint8_t mwc1() ;
    uint8_t mwc1modn( const uint8_t mmod ) ;
} ;


// Memory size constants
#define KB            (1ULL << 10)
#define MB            (1ULL << 20)
#define GB            (1ULL << 30)
#define TB            (1ULL << 40)
#define PB            (1ULL << 50)
#define EB            (1ULL << 60)

// const number
#define ONE_BILLION        (1.0E9)
#define ONE_MILLION        (1.0E6)
#define ONE_THOUSAND       (1.0E3)
#define ONE_BILLIONTH     (1.0E-9)
#define ONE_MILLIONTH     (1.0E-6)
#define ONE_THOUSANDTH    (1.0E-3)

// attributes ::
#define ALIGNED(a)          __attribute__((aligned(a)))
// Force alignment to nearest 128 bytes
#define ALIGN128            ALIGNED(128)
// Force alignment to nearest 64 bytes
#define ALIGN64             ALIGNED(64)
#define ALIGN_CACHELINE     ALIGN64
#define WEAK                __attribute__((weak))
// optimisation on branching
#define UNLIKELY(x)         __builtin_expect((x),0)
#define LIKELY(x)           __builtin_expect((x),1)
// optimize attribute 
#define HOT                 __attribute__ ((hot))
#define OPTIMIZE3           __attribute__((optimize("-O3")))
#define OPTIMIZE2           __attribute__((optimize("-O2")))
#define OPTIMIZE0           __attribute__((optnone))

// vitural memory page size is 4K when traslating from level-4 page table 
#define PAGESIZE            4096
// vitural memory page size is 2M when traslating from level-3 page table 
#define PAGESIZEHUGE        2097152
#define UNIVERSAL_CACHELINE 64

// alias_cast to avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
template<typename T, typename F>
struct alias_cast_t{
    union {
        F raw;
        T data;
    };
};

// cast from type F to type T
template<typename T, typename F>
T alias_cast(F raw_data){
    alias_cast_t<T, F> ac;
    ac.raw = raw_data;
    return ac.data;
}

// strength run time calculator 
// params: ( sgl_time , strength , period , module_runrounds , module_sleepus )
void strength_to_time( const double  , const uint32_t , const uint32_t , 
                       int32_t& , int32_t& ) ;

// benchmark entry function 
int32_t cache_bench_entry( bench_args_t ) ;


// mutex print 
extern mutex global_pr_mtx ;
void pr_warning( string ) ;
void pr_warning( char* ) ;
void pr_error( string ) ;
void pr_error( char* ) ;
void pr_info( string ) ;
void pr_info( char* ) ;
void pr_debug( string ) ;
void pr_debug( char* ) ;
void pr_debug( void(* prfunc )() ) ;

#endif 