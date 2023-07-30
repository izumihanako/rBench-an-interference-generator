// Bubble-up method
// https://ieeexplore.ieee.org/document/7851476
#include "rbench.hpp"

void OPTIMIZE3 cache_bench_rdacc_kernel( volatile char* block_aligned_ , uint32_t cache_size , mwc_t &mwc_eng ){
    register volatile char* block_aligned = block_aligned_ ;
    #define OP_LIMIT 20000 
    uint32_t count = 0 ;
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
//    block_aligned[0] += block_aligned[20] ;
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
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = std::min( std::max( int( 1000000000ll / cache_size ) , 500 ) , 5000 ) ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        cache_bench_rdacc_kernel( block_aligned , cache_size , random_mwc_eng ) ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    
    sprintf( infobuf , "cache bench( thread %d ): %d times kernel running takes %.6fs, average is %.6fs" , 
        thrid , measure_rounds , actl_runt , sgl_time ) ;
    pr_debug( infobuf ) ;
    
    int32_t module_runrounds , module_sleepus ;
    strength_to_time( sgl_time , sgl_idle , args.strength , args.period , module_runrounds , module_sleepus ) ;

    sprintf( infobuf , "cache bench( thread %d ): strength=%d , (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
        thrid , args.strength , module_runrounds * sgl_time * ONE_MILLION , (double)module_sleepus , module_runrounds * sgl_idle * ONE_MILLION ) ;
    pr_debug( infobuf ) ;

    // Run stressor
    int32_t round_cnt = 0 , time_limit = args.time , low_actl_strength_warning = 0 , 
            round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT32_MAX ;
    double t_start = time_now() , sum_krounds = 0 , sum_sleepus = 0 , sum_runtimeus = 0 , sum_runidleus = 0 ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            cache_bench_rdacc_kernel( block_aligned , cache_size , random_mwc_eng ) ;
        }
        md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
        actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , run_idlet = md_t_end - md_t_start - actl_runt ;
        sum_runtimeus += actl_runt * ONE_MILLION , sum_runidleus += run_idlet * ONE_MILLION ;
        sum_krounds += measure_rounds ;

        md_t_start = time_now() ;
        std::this_thread::sleep_for (std::chrono::microseconds( module_sleepus ) );
        md_t_end = time_now() ;
        double actl_sleepus = ( md_t_end - md_t_start ) * ONE_MILLION ;
        sum_sleepus += actl_sleepus ;
        sprintf( infobuf , "cache bench( thread %d ): (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
            thrid , actl_runt * ONE_MILLION, actl_sleepus , run_idlet * ONE_MILLION ) ;
        pr_debug( infobuf ) ;

        if( round_cnt + 1 > round_limit ) break ;
        if( time_limit && md_t_end - t_start > time_limit ) break ;

        // Load strength feedback regulation
        if( !( round_cnt & 0x7 ) ){
            sgl_time = sum_runtimeus * ONE_MILLIONTH / sum_krounds , sgl_idle = sum_runidleus * ONE_MILLIONTH / sum_krounds ;
            double actual_strength = 100 * sum_runtimeus / ( sum_runtimeus + sum_sleepus + sum_runidleus ) ;
            sprintf( infobuf , "cache bench( thread %d ): sgl_time = %.1fus, strength=%.1f, (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
                thrid , sgl_time * ONE_MILLION , actual_strength , sum_runtimeus, sum_sleepus , sum_runidleus ) ;
            pr_debug( infobuf ) ;
            // re-calculate load parameters
            if( args.strength - 0.5 > actual_strength || actual_strength > args.strength + 0.5 ){
                strength_to_time( sgl_time , sgl_idle , args.strength , args.period , module_runrounds , module_sleepus ) ;
            }
            if( args.strength - 0.5 > actual_strength ){
                low_actl_strength_warning += 0x8 ;
                if( low_actl_strength_warning > 0x20 ){
                    sprintf( infobuf , "LOW STRENGTH - cache bench( thread %d ): strength adjustment failed, current %.1f, target %.1f" , 
                        thrid , actual_strength , (double)args.strength ) ;
                    pr_warning( infobuf ) ;
                    low_actl_strength_warning = 0 ;
                }
            }
            sum_runtimeus -= ( sum_runtimeus ) / 8 , sum_krounds -= ( sum_krounds ) / 8 ;
            sum_sleepus -= ( sum_sleepus ) / 8 , sum_runidleus -= ( sum_runidleus ) / 8 ;
        }
    }
    sprintf( infobuf , "cache bench( thread %d ): stopped after %.1f seconds" , thrid , time_now() - t_start ) ;
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

perf stat -B -I 10000 \
          -e cycles -e instructions \
          -e L1-icache-loads -e L1-icache-load-misses \
          -e L1-dcache-loads -e L1-dcache-load-misses \
          -e l2_rqsts.references -e l2_rqsts.miss \
          -e LLC-loads -e LLC-load-misses \
          -e LLC-stores -e LLC-store-misses \
          -p 1676661
*/