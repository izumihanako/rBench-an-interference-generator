// stress-ng method
// https://github.com/ColinIanKing/stress-ng/blob/master/stress-cpu.c#L759
#include "rbench.hpp"

#define C1 	(0xf0f0f0f0f0f0f0f0ULL)
#define C2	(0x1000100010001000ULL)
#define C3	(0xffeffffefebefffeULL)
#define CAST_TO_UINT128(hi, lo)   ((((__uint128_t)hi << 64) | (__uint128_t)lo))

extern "C" int cpu_l1i_kernel() ;
// static void 
// dummy(){
//     __asm__
//         (
//            #include "rbench-cpu-l1i-kernel.asm"
//         );
// }

void cpu_l1i_bench( int32_t thrid , bench_args_t args ){
    char infobuf[1024] ;

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = 500 ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        cpu_l1i_kernel() ;
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
            cpu_l1i_kernel() ;
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
        args.bench_name.c_str() ,  thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
}

int32_t cpu_l1i_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }
    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( cpu_l1i_bench , i + 1 , args ) ;
    }
    if( get_arg_flag( args.flags , FLAG_IS_RUN_PARALLEL ) ){
        for( auto &thr : thrs ){
            glob_threads[glob_thr_cnt++].swap( thr ) ;
        }
    } else {
        for( auto &thr : thrs ){
            thr.join() ;
        }        
    }
    return 0 ;
}

#undef C1
#undef C2
#undef C3
#undef CAST_TO_UINT128