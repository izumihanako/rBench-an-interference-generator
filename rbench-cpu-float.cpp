// stress-ng method
// https://github.com/ColinIanKing/stress-ng/blob/master/stress-cpu.c#L759
#include "rbench.hpp"
#ifndef __long_double_t 
#define __long_double_t long double
#endif 

bool warn_flag = false ;

#define float_thresh(x, _type)	x = (_type)		\
	((fabs((double)x) > 1.0) ?	\
	((_type)(0.1 + (double)x - (double)(long)x)) :	\
	((_type)(x)))

template<typename T, typename Func>
static void OPTIMIZE0 float_ops_ikernel( T r_final , Func _sin , Func _cos ){
    // mwc_t mwc_eng ;
    // mwc_eng.set_default_seed() ;
    // const uint32_t r1 = mwc_eng.mwc32() ;
    // const uint32_t r2 = mwc_eng.mwc32() ;
    const uint32_t r1 = 820856226u ;
    const uint32_t r2 = 2331188998u ;

    T register a = (T) 0.18728L ,
               b = (T) ( (double)r1 / 65536.0 ) ,
               c = (T) ( (double)r2 / 65536.0 ) ,
               d = (T) 0.0 , r ;
    for( float i = 0.0 ; i < 1000.0 ; i ++ ){
        do{
            a = a + b ;
            d = a * c ;
            b = a * c ;
            c = a - b ;
            d = a / (T)8.1 ;
            float_thresh( d , T ) ;
            a = c / (T)5.1923 ;
            float_thresh( a , T ) ;
            float_thresh( c , T ) ;
            b = c + a ;
            c = b * (T)_sin(b) ;
            d = d + b + (T)_sin(a) ;
            a = (T)_cos( (double)( b + c ) ) ;
            b = b * c ; 
            c = c + (T)1.5 ;
            d = d - (T)_sin(c) ;
            a = a * (T)_cos(b) ;
            b = b + (T)_cos(c) ;
            c = (T)_sin( a + b ) / (T)2.344 ;
            b = d - (T)0.5 ;
        } while( 0 ) ;
    } 

    r = a + b + c + d ;
    // Calculate verification answer
    if( false ){ // Calculate before compilation
        std::stringstream ss ; ss.precision( 15 ) ;
        string sr ;
        ss << (double)r , ss >> sr ; ss.clear() ;
        printf( "%s: r = %s \n" , typeid( T ).name() , sr.c_str() ) ;
    }

    // verify
    if( !f_is_zero( r - r_final ) && !warn_flag ){
        warn_flag = true ;
        pr_warning( string( "error decected @ cpu-float-kernel, failed " ) + 
                  string( typeid( T ).name() ) + string( " math operations" ) ) ;
    }
}

static void float_ops_kernel(){
// #if (_GLIBCXX_USE_FLOAT128)
//         float_ops_ikernel<__float128>( 0 , __builtin_sin , __builtin_cos ) ;
// #endif
    float_ops_ikernel<__long_double_t>( -2.0687397322345 , __builtin_sin , __builtin_cos ) ;
    float_ops_ikernel<double_t>( -5.21491991288263 , __builtin_sin , __builtin_cos ) ;
    float_ops_ikernel<double_t>( -5.21491991288263 , __builtin_sin , __builtin_cos ) ;
    float_ops_ikernel<float_t>( (float)-2.88806390762329 , __builtin_sin , __builtin_cos ) ;
}

void cpu_float_bench( int32_t thrid , bench_args_t args ){
    char infobuf[1024] ;

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = 5000 ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        float_ops_kernel() ;
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
            float_ops_kernel() ;
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

int32_t cpu_float_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }
    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( cpu_float_bench , i + 1 , args ) ;
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

#undef float_thresh