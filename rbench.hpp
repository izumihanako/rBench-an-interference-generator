#ifndef RBENCH_H
#define RBENCH_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <thread>
#include <sched.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
using std::pair ;
using std::make_pair ;

namespace mytime{
    
extern double timeval_to_double(const struct timeval *tv) ;
extern double time_now(void) ;

}

/* Fast random numbers */
/* MWC random number initial seed */
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

    pair<uint32_t,uint32_t> get_seed() ;
    uint32_t mwc32() ;
    uint32_t mwc32modn( const uint32_t mmod ) ;
    uint16_t mwc16() ;
    uint16_t mwc16modn( const uint16_t mmod ) ;
    uint64_t mwc64() ;
    uint64_t mwc64modn( const uint64_t mmod ) ;
    uint8_t mwc8() ;
    uint8_t mwc8modn( const uint8_t mmod ) ;
    uint8_t mwc1() ;
    uint8_t mwc1modn( const uint8_t mmod ) ;
} ;


/* Memory size constants */
#define KB			(1ULL << 10)
#define	MB			(1ULL << 20)
#define GB			(1ULL << 30)
#define TB			(1ULL << 40)
#define PB			(1ULL << 50)
#define EB			(1ULL << 60)

#define ONE_BILLION		    (1.0E9)
#define ONE_MILLION		    (1.0E6)
#define ONE_THOUSAND		(1.0E3)
#define ONE_BILLIONTH		(1.0E-9)
#define ONE_MILLIONTH		(1.0E-6)
#define ONE_THOUSANDTH		(1.0E-3)

// attributes ::
#define ALIGNED(a)	        __attribute__((aligned(a)))
// Force alignment to nearest 128 bytes
#define ALIGN128	        ALIGNED(128)
// Force alignment to nearest 64 bytes
#define ALIGN64		        ALIGNED(64)
#define ALIGN_CACHELINE     ALIGN64
#define WEAK                __attribute__((weak))
// optimisation on branching
#define UNLIKELY(x)         __builtin_expect((x),0)
#define LIKELY(x)           __builtin_expect((x),1)
// optimize attribute 
#define HOT		            __attribute__ ((hot))
#define OPTIMIZE3 	        __attribute__((optimize("-O3")))
#define OPTIMIZE2 	        __attribute__((optimize("-O2")))
#define OPTIMIZE0	        __attribute__((optnone))

// vitural memory page size is 4K when traslating from level-4 page table 
#define PAGESIZE            4096
// vitural memory page size is 2M when traslating from level-3 page table 
#define PAGESIZEHUGE        2097152

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

#endif 