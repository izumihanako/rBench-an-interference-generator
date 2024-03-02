// stream method
// http://www.cs.virginia.edu/stream/ref.html 
#include "rbench.hpp"

typedef double stream_type ;

const uint64_t KERNEL_ITER = 120000 ;
const uint64_t BYTES_ADD_KERNEL = (KERNEL_ITER*3*sizeof(stream_type)) ;

static void OPTIMIZE3 mem_bw_add_kernel( stream_type* a_ , stream_type* b_ , stream_type* c_ , uint64_t len , uint64_t &ptr_ ){
    register stream_type *a = a_ ;
    register stream_type *b = b_ ;
    register stream_type *c = c_ ;
    register uint64_t ptr = ptr_ ;
    for( register uint64_t j = 0 ; j < KERNEL_ITER ; j ++ ){
        a[ptr] = b[ptr] + c[ptr] ;
        if( ++ptr >= len ) ptr = 0 ;
    }
    ptr_ = ptr ;
    // stream_type scalar = 3 ;
    // for( int j = 0 ; j < len_ ; j ++ ){
    //     c_[j] = a_[j] + scalar * b_[j] ;
    // }
}

static void OPTIMIZE3 mem_bw_init_kernel( stream_type* a , stream_type* b , stream_type*c , uint64_t len ){
    for( uint64_t j = 0 ; j < len ; j ++ ){
        a[j] = 1.0 ;
        b[j] = 2.0 ;
        c[j] = 0.0 ;
    }
}

static void mem_bw_bench_module( int32_t thrid , bench_args_t args , uint64_t array_len_ , stream_type*a , stream_type *b , stream_type *c ){
    // char* blocka, *blockb , *blockc ;
    // stream_type *a , *b , *c ;
    char infobuf[1024] ;

    // // Allocate memory buffer for membw benchmark
    // // at least 6 * size(L3) for each array
    uint64_t array_len = array_len_ , array_ptr = 0 ;
    // uint64_t buffer_size = array_len * sizeof( stream_type ) , buffer_align = cpuinfo.page_size ;
    // blocka = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    // blockb = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    // blockc = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    // if( blocka == MAP_FAILED || blockb == MAP_FAILED || blockc == MAP_FAILED ){
    //     sprintf( infobuf , "%s( thread %d ): mmap fails after retry, thread exits", args.bench_name.c_str() , thrid ) ;
    //     pr_error( infobuf ) ;
    //     return ;
    // }
    // a = (stream_type*)( blocka + buffer_align - (uintptr_t)blocka % buffer_align ) ;
    // b = (stream_type*)( blockb + buffer_align - (uintptr_t)blockb % buffer_align ) ;
    // c = (stream_type*)( blockc + buffer_align - (uintptr_t)blockc % buffer_align ) ;

    // init value 
    mem_bw_init_kernel( a , b , c , array_len ) ;
    sprintf( infobuf , "%s( thread %d ): init end\n" , args.bench_name.c_str() , thrid ) ;
    pr_debug( infobuf ) ;

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = 10000 ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        mem_bw_add_kernel( a , b , c , array_len , array_ptr ) ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    // double actl_membw = BYTES_ADD_KERNEL * measure_rounds / ( actl_runt + run_idlet ) ;
    // sprintf( infobuf , "%s( thread %d ): calc load para -> bw = %.1fMB/s, sgl_time = %.1fus\n" , args.bench_name.c_str() , thrid , actl_membw / MB , sgl_time * ONE_MILLION ) ;
    // pr_debug( infobuf ) ;
    int32_t module_runrounds , module_sleepus ;
    membw_to_time( BYTES_ADD_KERNEL , args.mem_bandwidth , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
    // sprintf( infobuf , "%s( thread %d ): adjust to -> run = %d, sleep %.1fus\n"
    //                    "        estimated bw = %.1fMB/s\n" , 
    //                    args.bench_name.c_str() , thrid , module_runrounds , (double)module_sleepus , 
    //                    1.0 * BYTES_ADD_KERNEL * module_runrounds / MB / ( 1.0 * module_runrounds * ( sgl_time + sgl_idle ) + module_sleepus ) ) ;
    // pr_debug( infobuf ) ;

    // Run stressor
    bool in_low_actl_membw_warning = false ;
    int32_t round_cnt = 0 , time_limit = args.time , low_actl_membw_warning = 0 ;
    int64_t knl_round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT64_MAX ;
    int64_t knl_round_sumup = 0 ;
    double t_start = time_now() , sum_krounds = 0 , sum_sleepus = 0 , sum_runtimeus = 0 , sum_runidleus = 0 ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            mem_bw_add_kernel( a , b , c , array_len , array_ptr ) ;
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
            double actl_membw = sum_krounds * (double)BYTES_ADD_KERNEL / ( sum_runtimeus + sum_sleepus + sum_runidleus ) * ONE_MILLION ;
            sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, membw=%.1fMB/s, (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
                args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , actl_membw / MB , sum_runtimeus, sum_sleepus , sum_runidleus ) ;
            pr_debug( infobuf ) ;
            // re-calculate load parameters
            if( (double) args.mem_bandwidth - MEMBW_CONTROL_LBOUND > actl_membw || 
                (double) args.mem_bandwidth + MEMBW_CONTROL_RBOUND < actl_membw ){
                membw_to_time( BYTES_ADD_KERNEL , args.mem_bandwidth , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
            }
            if( (double) args.mem_bandwidth - 2 * MEMBW_CONTROL_LBOUND > actl_membw ){
                if( ++low_actl_membw_warning > 8 ){
                    sprintf( infobuf , "LOW MEM_BW - %s( thread %d ): current %.1fMB/s, target %.1fMB/s, adjusting..." , 
                        args.bench_name.c_str() , thrid , actl_membw / MB , (double)args.mem_bandwidth / MB ) ;
                    pr_warning( infobuf ) ;
                    in_low_actl_membw_warning = true ;
                    low_actl_membw_warning = 0 ;
                }
            } else {
                if( in_low_actl_membw_warning ){
                    sprintf( infobuf , "LOW MEM_BW - %s( thread %d ): adjustment succeed, current %.1fMB/s, target %.1fMB/s" , 
                        args.bench_name.c_str() , thrid , actl_membw / MB , (double)args.mem_bandwidth / MB ) ;
                    pr_warning( infobuf ) ;
                }
                in_low_actl_membw_warning = false ;
                low_actl_membw_warning = 0 ;
            }
            sum_runtimeus -= ( sum_runtimeus ) / 5 , sum_krounds -= ( sum_krounds ) / 5 ;
            sum_sleepus -= ( sum_sleepus ) / 5 , sum_runidleus -= ( sum_runidleus ) / 5 ;
        }
    }
    sprintf( infobuf , "%s( thread %d ): stopped after %.3f seconds, %ld rounds" , 
        args.bench_name.c_str() , thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
    // Deallocate the memory buffer
    // munmap( (void*)blocka , buffer_size + buffer_align ) ;
    // munmap( (void*)blockb , buffer_size + buffer_align ) ;
    // munmap( (void*)blockc , buffer_size + buffer_align ) ;
}

int32_t mem_bw_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }


    // Allocate memory buffer for membw benchmark
    // at least 6 * size(L3) for each array
    uint64_t array_len = cpuinfo.get_data_cache_size_level( 3 ) * 6 ,
             buffer_size = array_len * sizeof( stream_type ) , buffer_align = cpuinfo.page_size ;
    char* blocka = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    char* blockb = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    char* blockc = (char*)mmap_with_retry( buffer_size + buffer_align ) ;
    if( blocka == MAP_FAILED || blockb == MAP_FAILED || blockc == MAP_FAILED ){
        char infobuf[1024] ;
        sprintf( infobuf , "%s: mmap fails after retry, exits", args.bench_name.c_str() ) ;
        pr_error( infobuf ) ;
        return -1 ;
    }
    stream_type* a_start = (stream_type*)( blocka + buffer_align - (uintptr_t)blocka % buffer_align ) ;
    stream_type* b_start = (stream_type*)( blockb + buffer_align - (uintptr_t)blockb % buffer_align ) ;
    stream_type* c_start = (stream_type*)( blockc + buffer_align - (uintptr_t)blockc % buffer_align ) ;


    // Allocate memory buffer for membw benchmark
    // at least 6 * size(L3) in total, so at least 6*size(L3)/ninstance for each instance 
    uint64_t instance_array_len = cpuinfo.get_data_cache_size_level( 3 ) * 6 / count_thr ;

    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( mem_bw_bench_module , i + 1 , args , instance_array_len ,
                          a_start + instance_array_len * i ,
                          b_start + instance_array_len * i ,
                          c_start + instance_array_len * i ) ;
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
