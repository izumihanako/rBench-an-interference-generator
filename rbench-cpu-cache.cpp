// Bubble-up method
// https://ieeexplore.ieee.org/document/7851476
#include "rbench.hpp"

static void OPTIMIZE3 cache_bench_rdacc_kernel( volatile char* block_aligned_ , uint32_t cache_size_ , mwc_t &mwc_eng ){
    #define OP_LIMIT 16383 
    register volatile char* block_aligned = block_aligned_ ;
    register uint32_t cache_size = cache_size_ ;
    register uint32_t count = 0 ;
    while( 1 ){
        if( count > OP_LIMIT ) break ;
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

static void cache_bench_rand_access_module( int32_t thrid , bench_args_t args , uint32_t cache_line_size ){
    char volatile *block , *block_aligned ;
    char infobuf[1024] ;
    
    // Cache size range check
    uint32_t cache_size = (uint32_t) args.cache_size , cache_align = cache_line_size ;
    if( cache_size != args.cache_size ){
        sprintf( infobuf , "%s( thread %d ): The given cache size %llu exceeds the range of uint32_t" , 
            args.bench_name.c_str() , thrid , (unsigned long long) args.cache_size ) ;
        pr_error( infobuf ) ;
        return ;
    }
    
    // Allocate memory buffer for cache benchmark
    block = (char*) mmap_with_retry( cache_size + cache_align ) ;
    if( UNLIKELY( block == MAP_FAILED ) ){
        sprintf( infobuf , "%s( thread %d ): mmap fails after retry, thread exits", args.bench_name.c_str() , thrid ) ;
        pr_error( infobuf ) ;
        return ;
    }
    block_aligned = block + cache_align - (uintptr_t)block % cache_align ;

    // Calculate load parameters 
    mwc_t mwc_eng ;
    mwc_eng.reseed() ;
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = std::min( std::max( int( 1000000000ll / cache_size ) , 500 ) , 5000 ) ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        cache_bench_rdacc_kernel( block_aligned , cache_size , mwc_eng ) ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    int32_t module_runrounds , module_sleepus ;
    strength_to_time( sgl_time , sgl_idle , args.strength , args.period , module_runrounds , module_sleepus ) ;

    // Run stressor
    bool in_low_actl_strength_warning = false ;
    int32_t round_cnt = 0 , time_limit = args.time , low_actl_strength_warning = 0 ;
    int64_t knl_round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT64_MAX ;
    int64_t knl_round_sumup = 0 ;
    double t_start = time_now() , sum_krounds = 0 , sum_sleepus = 0 , sum_runtimeus = 0 , sum_runidleus = 0 ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            cache_bench_rdacc_kernel( block_aligned , cache_size , mwc_eng) ;
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

        knl_round_sumup += measure_rounds ;
        if( knl_round_sumup >= knl_round_limit ) break ;
        if( time_limit && md_t_end - t_start >= time_limit ) break ;

        // Load strength feedback regulation
        if( !( round_cnt & 0x7 ) ){
            sgl_time = sum_runtimeus * ONE_MILLIONTH / sum_krounds , sgl_idle = sum_runidleus * ONE_MILLIONTH / sum_krounds ;
            double actual_strength = 100 * sum_runtimeus / ( sum_runtimeus + sum_sleepus + sum_runidleus ) ;
            sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, strength=%.1f, (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
                args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , actual_strength , sum_runtimeus, sum_sleepus , sum_runidleus ) ;
            pr_debug( infobuf ) ;
            // re-calculate load parameters
            if( args.strength - STRENGTH_CONTROL_LBOUND > actual_strength || 
                args.strength + STRENGTH_CONTROL_RBOUND < actual_strength ){
                strength_to_time( sgl_time , sgl_idle , args.strength , args.period , module_runrounds , module_sleepus ) ;
            }
            if( args.strength - 1 > actual_strength ){
                if( ++low_actl_strength_warning > 8 ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): current %.1f%%, target %.1f%%, adjusting..." , 
                        args.bench_name.c_str() , thrid , actual_strength , (double)args.strength ) ;
                    pr_warning( infobuf ) ;
                    in_low_actl_strength_warning = true ;
                    low_actl_strength_warning = 0 ;
                }
            } else {
                if( in_low_actl_strength_warning ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): adjustment succeed, current %.1f%%, target %.1f%%" , 
                        args.bench_name.c_str() , thrid , actual_strength , (double)args.strength ) ;
                    pr_warning( infobuf ) ;
                }
                in_low_actl_strength_warning = false ;
                low_actl_strength_warning = 0 ;
            }
            sum_runtimeus -= ( sum_runtimeus ) / 5 , sum_krounds -= ( sum_krounds ) / 5 ;
            sum_sleepus -= ( sum_sleepus ) / 5 , sum_runidleus -= ( sum_runidleus ) / 5 ;
        }
    }
    sprintf( infobuf , "%s( thread %d ): stopped after %.3f seconds, %ld rounds" , 
        args.bench_name.c_str() , thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
    // Deallocate the memory buffer
    munmap( (void*)block , cache_size + cache_align ) ;
}

int32_t cache_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) ){
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
        thrs[i] = thread( cache_bench_rand_access_module , i + 1 , args , cache_line_size ) ;
    }
    if( get_arg_flag( args.flags , FLAG_IS_RUN_PARALLEL ) ){
        for( auto &thr : thrs ){
            thr.swap( glob_threads[glob_thr_cnt++] ) ;
        }
    } else {
        for( auto &thr : thrs ){
            thr.join() ;
        }        
    }
    return 0 ;
}