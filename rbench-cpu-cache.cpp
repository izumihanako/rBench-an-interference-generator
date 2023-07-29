// Bubble-up method
// https://ieeexplore.ieee.org/document/7851476
#include "rbench.hpp"

void HOT OPTIMIZE3 cache_bench_rdacc_kernel( volatile char* block_aligned_ , uint32_t cache_size , mwc_t &mwc_eng ){
    register volatile char* block_aligned = block_aligned_ ;
    #define OP_LIMIT 20000 
    register uint32_t count = 0 ;
    while( 1 ){
        if( UNLIKELY( count > OP_LIMIT ) ) break ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;

        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;

        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;

        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        block_aligned[mwc_eng.mwc32modn_maybe_pwr2(cache_size)] ++ ;
        count += 20 ;
    }
    #undef OP_LIMIT 
    // printf( "res[0] = %d\n" , block_aligned[0] ) ;
}

void cache_bench_rand_access( int32_t thrid , bench_args_t args , uint32_t cache_line_size ){
    char volatile *block , *block_aligned ;
    char infobuf[1024] ;
    
    // Cache size range check
    uint32_t cache_size = (uint32_t) args.cache_size ;
    if( cache_size != args.cache_size ){
        sprintf( infobuf , "cache bench( thread %d ): The given cache size %llu exceeds the range of uint32_t" , 
            thrid , (unsigned long long) args.cache_size ) ;
        pr_error( infobuf ) ;
        return ;
    }
    
    // Allocate memory buffer for cache benchmark
    block = (char*)mmap( NULL , cache_size << 1 , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1 , 0 ) ;
    if( UNLIKELY( block == MAP_FAILED ) ){
        sleep( 1 ) ; // wait for 1 second then retry
        block = (char*)mmap( NULL , cache_size << 1 , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1 , 0 ) ;
        if( block == MAP_FAILED ){
            sprintf( infobuf , "cache bench( thread %d ): mmap fails after retry, thread exits", thrid ) ;
            pr_error( infobuf ) ;
            return ;
        }
    }
    block_aligned = block + cache_line_size - (uintptr_t)block % cache_line_size ;

    // Calculate load parameters 
    mwc_t random_mwc_eng ;
    double time_start = time_now() ;
    int measure_rounds = std::min( std::max( int( 1000000000ll / cache_size ) , 500 ) , 5000 ) ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        cache_bench_rdacc_kernel( block_aligned , cache_size , random_mwc_eng ) ;
    }
    double time_span = time_now() - time_start , stressor_span = time_span , sgl_time = time_span / measure_rounds ;
    
    sprintf( infobuf , "cache bench( thread %d ): %d times kernel running takes %.6fs, average is %.6fs" , 
        thrid , measure_rounds , time_span , sgl_time ) ;
    pr_debug( infobuf ) ;
    
    int32_t module_runrounds , module_sleepus ;
    strength_to_time( sgl_time , args.strength , args.period , module_runrounds , module_sleepus ) ;

    sprintf( infobuf , "cache bench( thread %d ): strength=%d period=%dus , (runtime=%.1fus , sleeptime=%.1fus)" , 
        thrid , args.strength , args.period , module_runrounds * sgl_time * ONE_MILLION , (double)module_sleepus ) ;
    pr_debug( infobuf ) ;

    // Run stressor
    double stress_start_time = time_now() , sum_rounds = 0 ;
    int32_t round_cnt = 0 , time_limit = args.time , 
            round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT32_MAX ;
    time_start = stress_start_time ;
    while( true ){
        round_cnt ++ ;
        measure_rounds = module_runrounds ;
        for( int i = 0 ; i < module_runrounds ; i ++ ){
            cache_bench_rdacc_kernel( block_aligned , cache_size , random_mwc_eng ) ;
        }
        usleep( module_sleepus ) ;
        sum_rounds ++ ;
        if( round_cnt + 1 > round_limit ){
            break ;
        }
        double tmp_nowtime = time_now() ;
        if( time_limit && tmp_nowtime - stress_start_time > time_limit )
            break ;

        // Load strength feedback regulation
        if( ( round_cnt & (0x10) ) == 0x10 ){
            time_span = tmp_nowtime - time_start ;
            stressor_span = time_span - sum_rounds * module_sleepus * ONE_MILLIONTH ;
            sgl_time = stressor_span / sum_rounds / module_runrounds ;
            double actual_strength = 100 * stressor_span / time_span ;
            // re-calculate load parameters
            if( args.strength - 1 > actual_strength || actual_strength > args.strength + 1 ){
                sprintf( infobuf , "cache bench( thread %d ): before- sgl_time=%.1fus , (runtime=%.1fus , sleeptime=%.1fus)" , 
                    thrid , sgl_time * ONE_MILLION , stressor_span * ONE_MILLION , (double)module_sleepus ) ;
                pr_debug( infobuf ) ;
                strength_to_time( sgl_time , args.strength , args.period , module_runrounds , module_sleepus ) ;
                sprintf( infobuf , "cache bench( thread %d ): after- strength=%d period=%dus , (exptime=%.1fus , sleeptime=%.1fus)" , 
                    thrid , args.strength , args.period , 1.0 * module_runrounds * sgl_time * ONE_MILLION , (double)module_sleepus ) ;
                pr_debug( infobuf ) ;
            }
            time_start += ( tmp_nowtime - time_start ) / 2 ; 
            sum_rounds /= 2 ;
        }
    }
    sprintf( infobuf , "cache bench( thread %d ): stopped after %.1f seconds" , thrid , time_now() - stress_start_time ) ;
    pr_info( infobuf ) ;
    // Deallocate the memory buffer
    munmap( (void*)block , cache_size << 1 ) ;
}

int32_t cache_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    pr_debug( string( "cache_bench_entry function, the args are:" ) ) ;
    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }
    // get cache line size
    uint32_t cache_line_size = UNIVERSAL_CACHELINE ;
    for( int i = 0 ; i < cpuinfo.cache_count ; i ++ ){
        if( cpuinfo.caches[i].type == CACHE_TYPE_INSTRUCTION ) continue ;
        if( args.cache_size > cpuinfo.caches[i].size ){
            cache_line_size = cpuinfo.caches[i].line_size ;
        }
    }
    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( cache_bench_rand_access , i + 1 , args , cache_line_size ) ;
    }
    for( auto &thr : thrs ){
        thr.join() ;
    }
    return 0 ;
}

/*
inst retired version perf:
perf stat -B -r 5 \
          -o bubblecache_Data/rand_access_14417920_CAT11way.txt \
          -e cycles -e instructions \
          -e L1-icache-load-misses \
          -e machine_clears.count \
          -e mem_inst_retired.all_loads \
          -e mem_load_retired.fb_hit \
          -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss \
          -e mem_load_retired.l2_hit -e mem_load_retired.l2_miss \
          -e mem_load_retired.l3_hit -e mem_load_retired.l3_miss \
          commands...

Hardware cache event perf:

perf stat -B -r 5 \
          -o bubblecache_Data/stream_access_57671680.txt \
          -e cycles -e instructions \
          -e L1-icache-loads -e L1-icache-load-misses \
          -e L1-dcache-loads -e L1-dcache-load-misses \
          -e l2_rqsts.references -e l2_rqsts.miss \
          -e LLC-loads -e LLC-load-misses \
          -e LLC-stores -e LLC-store-misses \
          commands...
*/