// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
#include "rbench.hpp"
#include <immintrin.h>

static bool warn_flag = false ;

static void OPTIMIZE0 simd_avx_ops_kernel(){
    mwc_t mwc_eng ;
    mwc_eng.set_default_seed() ;

    __m256d a = _mm256_set_pd( (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() ) ;
    __m256d b = _mm256_set_pd( (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() ) ;
    __m256d c = _mm256_set_pd( (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() , (double)mwc_eng.mwc32() ) ;
    __m256d d = _mm256_set_pd( 0.0 , 0.0 , 0.0 , 0.0 ) ;
    __m256d tmp ;

    for( int i = 0 ; i < 1000 ; i ++ ){
        do{
            a = _mm256_add_pd( a , b ) ;
            d = _mm256_mul_pd( a , c ) ;
            b = _mm256_mul_pd( a , c ) ;
            c = _mm256_sub_pd( a , b ) ;
            d = _mm256_div_pd( a , _mm256_set_pd( 8.1 , 8.1 , 8.1 , 8.1 ) ) ;
            tmp = _mm256_round_pd( d , _MM_FROUND_TO_ZERO |_MM_FROUND_NO_EXC ) ;
            d = _mm256_sub_pd( d , tmp ) ;
            a = _mm256_div_pd( c , _mm256_set_pd( 5.1923 , 5.1923 , 5.1923 , 5.1923 ) ) ;
            tmp = _mm256_round_pd( a , _MM_FROUND_TO_ZERO |_MM_FROUND_NO_EXC ) ;
            a = _mm256_sub_pd( a , tmp ) ;
            tmp = _mm256_round_pd( c , _MM_FROUND_TO_ZERO |_MM_FROUND_NO_EXC ) ;
            c = _mm256_sub_pd( c , tmp ) ;
            b = _mm256_add_pd( c , a ) ;
            d = _mm256_castsi256_pd( _mm256_srli_epi64( _mm256_castpd_si256(d) , 2 ) ) ;
            d = _mm256_castsi256_pd( _mm256_slli_epi64( _mm256_castpd_si256(d) , 2 ) ) ;
            c = _mm256_hadd_pd( a , b ) ;
            d = _mm256_permute4x64_pd( d , 0x78 ) ;
            a = _mm256_permute4x64_pd( a , 0x63 ) ;
            b = _mm256_mul_pd( b , c ) ;
            c = _mm256_add_pd( c , _mm256_set_pd( 1.5 , 1.5 , 1.5 , 1.5 ) ) ;
            d = _mm256_addsub_pd( d , c ) ;
            a = _mm256_addsub_pd( a , b ) ;
            b = _mm256_addsub_pd( b , c ) ;
            b = _mm256_castsi256_pd( _mm256_srli_epi64( _mm256_castpd_si256(b) , 3 ) ) ;
            b = _mm256_castsi256_pd( _mm256_slli_epi64( _mm256_castpd_si256(b) , 3 ) ) ;
            c = _mm256_div_pd( _mm256_add_pd( a , b ) , c ) ;
            b = _mm256_sub_pd( d , b ) ;
        } while( 0 ) ;
    }
    tmp = _mm256_add_pd( a , b ) ;
    tmp = _mm256_add_pd( tmp , c ) ;
    tmp = _mm256_add_pd( tmp , d ) ;
    ALIGN64 double res[4] , std[4] = { -2.5, 2.5, -2.5, 2.5 } ;
    _mm256_storeu_pd( res , tmp ) ;
    // printf( "%.10f %.10f %.10f %.10f\n" , res[0] , res[1] , res[2] , res[3] ) ;
    if( ( !f_is_zero( res[0] - std[0] ) || !f_is_zero( res[1] - std[1] ) ||
        !f_is_zero( res[2] - std[2] ) || !f_is_zero( res[3] - std[3] ) ) && !warn_flag ){
            char infobuf[128] ;
            warn_flag = true ;
            pr_warning( string( "error decected @ simd-avx, failed __m256d" ) + string( " math operations" ) ) ;
            sprintf( infobuf , "expected (%.2f|%.2f|%.2f|%.2f), got (%.2f|%.2f|%.2f|%.2f)" ,
                     std[0] , std[1] , std[2] , std[3] , res[0] , res[1] , res[2] , res[3] ) ;
            pr_warning( infobuf ) ;
        }
}

void simd_avx_bench( int32_t thrid , bench_args_t args ){
    char infobuf[1024] ;

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = 5000 ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        simd_avx_ops_kernel() ;
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
            simd_avx_ops_kernel() ;
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

int32_t simd_avx_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }
    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( simd_avx_bench , i + 1 , args ) ;
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
