#include "rbench.hpp"
#define DISK_WRITE_MB 1
#define DISK_WRITE_GRANU (DISK_WRITE_MB*MB)

static void OPTIMIZE3 disk_write_kernel2( int fd , uint64_t fsize , uint64_t fpos ){
    static uint8_t mem[DISK_WRITE_GRANU] ;
    static bool inited = false ;
    if( inited == false ){
        for( uint64_t i = 0 ; i < DISK_WRITE_GRANU ; i += 4 ){
            *(uint32_t*)(mem+i) = rand() ;
        }
        inited = true ;
    }
    if( fpos + DISK_WRITE_GRANU >= fsize ) {
        lseek( fd , 0 , SEEK_SET ) ;
        fpos = 0 ;
    }
    write( fd , mem , DISK_WRITE_GRANU ) ;
    fpos += DISK_WRITE_GRANU ;
}

void disk_write_bench( int32_t thrid , bench_args_t args ){
    char infobuf[1024] ;
    double start_t = time_now() ;

    int diskfile_rnd = 0xffffff & ( (long long)( new char ) + rand() + time(0) ) ;
    char filename[30] ;
    sprintf( filename , "%d.diskfile" , diskfile_rnd ) ;
    int fd = open( filename , O_RDWR | O_CREAT | O_SYNC , 0777 ) ;
    uint64_t fsize = args.disk_file_size ;
    if( ftruncate64( fd , fsize ) == -1 ){
        sprintf( infobuf , "%s( thread %d ): ftruncate64( %s , %lu ) failed" ,
                 args.bench_name.c_str() , thrid , filename , fsize ) ;
        remove( filename ) ;
        return ;
    }
    uint64_t fpos = 0 ;
    disk_write_kernel2( fd , fsize , fpos ) ;
    fflush( stdout ) ;

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = 1000 ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        disk_write_kernel2( fd , fsize , fpos ) ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    int32_t module_runrounds , module_sleepus ;
    strength_to_time( sgl_time , sgl_idle , args.strength , args.period , module_runrounds , module_sleepus ) ;
    printf( "sgl_time = %.1fus, prepare end\n" , (sgl_time + sgl_idle)*ONE_MILLION ) ;
    fflush( stdout ) ;

    // Run stressor
    int32_t round_cnt = 0 , time_limit = args.time ;
    int64_t knl_round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT64_MAX ;
    int64_t knl_round_sumup = 0 ;
    double t_start = time_now() ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            disk_write_kernel2( fd , fsize , fpos ) ;
        }
        md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
        actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , run_idlet = md_t_end - md_t_start - actl_runt ;

        knl_round_sumup += measure_rounds ;
        if( knl_round_sumup >= knl_round_limit ) break ;
        if( time_limit && md_t_end - t_start >= time_limit ) break ;

        // the disk bench do not suit for Load strength feedback regulation
        // so just output information there, without adjusting
        if( !( round_cnt & 0x7 ) ){
            sgl_time = (actl_runt+run_idlet) / ONE_MILLIONTH / measure_rounds , sgl_idle = run_idlet / ONE_MILLIONTH / measure_rounds ;
            sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus (cpu_time=%.1fus , cpu_idletime=%.1fus) speed = %.1lfMB/s", 
                args.bench_name.c_str() , thrid , sgl_time , sgl_time-sgl_idle , sgl_idle , DISK_WRITE_MB / sgl_time / ONE_MILLIONTH ) ;
            pr_info( infobuf ) ;
        }
    }
    sprintf( infobuf , "%s( thread %d ): stopped after %.3f seconds, %ld rounds" , 
        args.bench_name.c_str() ,  thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
}

int32_t disk_write_bench_entry( bench_args_t args ){
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }
    // run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    for( int i = 0 ; i < count_thr ; i ++ ){
        thrs[i] = thread( disk_write_bench , i + 1 , args ) ;
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