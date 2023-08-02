#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#define unlikely(x) __builtin_expect(!!(x),0)
#define OPTIMIZE3 	__attribute__((optimize("-O3")))

#define L1_SIZE (32768)
#define L2_SIZE (1048576)
#define L3_SIZE (14417920)

// Galois LFSR
#define MASK 0xd0000001u
#define rand ( lfsr = ( lfsr >> 1 ) ^ ( unsigned int ) \
               ( 0 - ( lfsr & 1u ) & MASK ) )
#define CACHE_LINE (64)
#define CACHE_SIZE L1_SIZE
#define r (rand%CACHE_SIZE)
const uint32_t OP_LIMIT = 2000000000 ;

static void rand_access( 
    volatile char* block_aligned_
){
    printf( "rand access to %d\n" , CACHE_SIZE ) ;
    register volatile char* block_aligned = block_aligned_ ;
    register unsigned int lfsr = 1 ;
    register uint32_t count = 0 ;
    while( 1 ){
        if( unlikely( count > OP_LIMIT ) ) break ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;

        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;

        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;

        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        block_aligned[r] ++ ;
        count += 20 ;
    }
    // printf( "res[0] = %d\n" , block_aligned[0] ) ;
}

static void stream_access(
    volatile char* block_aligned 
){
    printf( "stream access to %d\n" , CACHE_SIZE ) ;
    register uint32_t count = 0 ;
    register volatile uint8_t* first_half = block_aligned ;
    register volatile uint8_t* second_half = block_aligned + ( CACHE_SIZE / 2 ) ;
    register uint32_t i ;
    while( 1 ) {
        for( i = 0 ; i < CACHE_SIZE / 2 ; i += CACHE_LINE )
            first_half[i] = second_half[i] + 1 ;
        for( i = 0 ; i < CACHE_SIZE / 2 ; i += CACHE_LINE )
            second_half[i] = first_half[i] + 1 ;
        count += CACHE_SIZE / CACHE_LINE ;
        if( unlikely( count > OP_LIMIT ) ) break ;
    }
}

int main( int argc , char **argv ){
    
    char volatile *block , *block_aligned ;
    block = (char*)mmap( NULL , CACHE_SIZE << 1 , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1 , 0 ) ;

    block_aligned = block + CACHE_LINE - (uintptr_t)block % CACHE_LINE ;
    rand_access( block_aligned ) ;
}

/*
gcc validate_cacheL1.c -o validate_cacheL1 -std=gnu11 -O0 && \
perf stat -B -r 5 \
          -e cycles -e instructions \
          -e L1-icache-load-misses \
          -e machine_clears.count \
          -e mem_inst_retired.all_loads \
          -e mem_load_retired.fb_hit \
          -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss \
          -e mem_load_retired.l2_hit -e mem_load_retired.l2_miss \
          -e mem_load_retired.l3_hit -e mem_load_retired.l3_miss \
          taskset -c 20 ./validate_cacheL1
*/