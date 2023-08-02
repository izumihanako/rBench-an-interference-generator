#include "rbench.hpp"

double OPTIMIZE3 timeval_to_double( const struct timeval *tv ) {
    return (double)tv->tv_sec + ( (double)tv->tv_usec * ONE_MILLIONTH ) ;
}

double OPTIMIZE3 time_now() {
    timeval now;
    if ( gettimeofday( &now, NULL ) < 0 ){
        return -1.0 ;
    }
    return timeval_to_double( &now ) ;
}

double OPTIMIZE3 thread_time_now(){
    struct rusage rusageinfo ;
    getrusage( RUSAGE_THREAD , &rusageinfo ) ; // (since Linux 2.6.26)
    return timeval_to_double( &(rusageinfo.ru_utime ) ) ;
        //    timeval_to_double( &(rusageinfo.ru_stime ) ) ;
}

void set_arg_flag( uint32_t& flags , argflag_t id ){
	flags |= ( 1 << static_cast<int>(id) ) ;
}

void clr_arg_flag( uint32_t& flags , argflag_t id ) {
	if( flags & ( 1 << static_cast<int>(id) ) ){
		flags -= ( 1 << static_cast<int>(id) ) ;
	}
}

bool get_arg_flag( const uint32_t flags , argflag_t id ){
	return ( flags & ( 1 << static_cast<int>(id) ) ) >> id ;
}


cpuinfo_t::cpuinfo_t(){
    page_size = 0 ;
    core_count = 0 ;
    online_count = 0 ;
    cache_count = 0 ;
}

void cpuinfo_t::read_cpuinfo(){
    core_count = (int32_t) sysconf( _SC_NPROCESSORS_CONF ) ;
    online_count = (int32_t) sysconf( _SC_NPROCESSORS_ONLN ) ;
    page_size = (int32_t) sysconf( _SC_PAGE_SIZE ) ;

    fstream fs ;
    char buffer[4096] ;
    for( char i = '0' ; i <= '9' ; i ++ ){
        string path( "/sys/devices/system/cpu/cpu0/cache/index" ) ;
        path.push_back( i ) ;
        path.push_back( '/' ) ;
        string file = path + string( "type" ) ;
        fs.open( file , ios::in ) ;
        if( !fs.is_open() ) break ;

        cache_count ++ ;
        int index = i - '0' ;
        fs.getline( buffer , 20 ) ;
        if( !strncasecmp( buffer , "Data" , 4 ) ){
            caches[index].type = CACHE_TYPE_DATA ;
        } else if( !strncasecmp( buffer , "Instruction" , 11 ) ){
            caches[index].type = CACHE_TYPE_INSTRUCTION ;
        } else if( !strncasecmp( buffer , "Unified" , 7 ) ){
            caches[index].type = CACHE_TYPE_UNIFIED ;
        } else {
            caches[index].type = CACHE_TYPE_UNKNOWN ;
        }
        fs.close() ;
        
        file = path + string( "level" ) ;
        fs.open( file , ios::in ) ;
        if( fs.is_open() ){
            fs >> caches[index].level ;
            fs.close() ;
        }

        file = path + string( "coherency_line_size" ) ;
        fs.open( file , ios::in ) ;
        if( fs.is_open() ){
            fs >> caches[index].line_size ;
            fs.close() ;
        }

        file = path + string( "number_of_sets" ) ;
        fs.open( file , ios::in ) ;
        if( fs.is_open() ){
            fs >> caches[index].sets ;
            fs.close() ;
        }

        file = path + string( "ways_of_associativity" ) ;
        fs.open( file , ios::in ) ;
        if( fs.is_open() ){
            fs >> caches[index].ways ;
            fs.close() ;
        }

        file = path + string( "size" ) ;
        fs.open( file , ios::in ) ;
        if( fs.is_open() ){
            string unit ;
            fs >> caches[index].size >> unit ;
            if( !strncasecmp( unit.c_str() , "K" , 1 ) ){
                caches[index].size *= KB ;
            } else if( !strncasecmp( unit.c_str() , "M" , 1 ) ){
                caches[index].size *= MB ;
            } else if( !strncasecmp( unit.c_str() , "G" , 1 ) ){
                caches[index].size *= GB ;
            } else if( unit.length() == 0 ) {
                // do nothing 
            } else {
                fprintf( stderr , "reading cache info: \n") ;
                fprintf( stderr , "unrecognizable size suffix: %s\n" , unit.c_str() ) ;
            }
            fs.close() ;
            caches[index].buffer = shared_ptr<char>( new char[ caches[index].size ] ) ;
        } else {
			printf( "warning:\n") ;
			printf( "  at cache index %d:\n" , index ) ;
			printf( "  read cache size error\n" ) ;
			printf( "  cannot open file: %s\n" , file.c_str() ) ;
		}
    }
	if( cache_count == 0 ){
		printf( "warning:\n") ;
		printf( "  attempt to read cache info, but found no cache" );
	}
	return ;
}

void bench_args_t::print_argsinfo() {
    printf( "%s\n%-4sthreads = %u, strength = %u%%, period = %uus\n" , 
            bench_name.c_str() , "" , threads , strength , period ) ;
    if( get_arg_flag( flags , FLAG_IS_LIMITED ) ){
        printf( "%-4slimited , rounds = %ld, " , "" , limit_round ) ;
    } else{
        printf( "%-4srounds unlimited, " , "" ) ;
    }
    if( time != 0 ){
        printf( "time limit = %ds, " , time ) ;
    } else {
        printf( "time unlimited, " ) ;
    }
    printf( "size(bw) = %llu\n" , (unsigned long long) cache_size ) ;
}

uint64_t cpuinfo_t::get_data_cache_size_level( uint32_t lev ){
    for( int i = 0 ; i < cache_count ; i ++ ){
        if( caches[i].level != lev ) continue ;
        if( caches[i].type != CACHE_TYPE_UNIFIED &&
            caches[i].type != CACHE_TYPE_DATA ) continue ;
        return caches[i].size ;
    }
    return 0 ;
}

void cpuinfo_t::print_cpuinfo() {
    static const char* cache_type_txt[] = {
        "CACHE_TYPE_UNKNOWN" ,
        "CACHE_TYPE_DATA" , 
        "CACHE_TYPE_INSTRUCTION" ,
        "CACHE_TYPE_UNIFIED"
    } ;
    printf( "cpuinfo:\n") ;
    printf( "    core count  : %-2d\n" , core_count ) ;
    printf( "    core online : %-2d\n" , online_count ) ;
    printf( "    page size   : %-2d KB\n" , page_size ) ;
    printf( "    cache count : %-2d\n" , cache_count ) ;
    
    for( int i = 0 ; i < cache_count ; i ++ ){
    printf( "    cache index %d (%s):\n" , i , cache_type_txt[ caches[i].type ] ) ;
    printf( "        level : %d\n" , caches[i].level ) ;
    printf( "         size : %lu KB\n" , caches[i].size ) ;
    printf( "         ways : %d\n" , caches[i].ways ) ;
    printf( "         sets : %d\n" , caches[i].sets ) ;
    printf( "    line size : %d\n" , caches[i].line_size ) ;
    }
}

void strength_to_time( const double sgl_round_time , const double sgl_idle_time , const uint32_t strength_ , 
                       const uint32_t period , int32_t& module_runround , int32_t &module_sleepus ){
    if( strength_ == 100 ){
        module_runround = 100 ;
        module_sleepus = 0 ;
    } else {
        // a = sgl_round_time , b = sgl_idle_time , p = strength
        // x = module_runround , y = module_sleepus 
        // ax/(ax+bx+y) = p 
        // ax = pax + pbx + py
        // x( a-pa-pb ) = py
        // x = p/(a-pa-pb) y
        double p = strength_ / 100.0 ;
        double y = ( 1 - p ) * period ; // let y = ( 1 - p ) * period
        double x = y * p / ( ( sgl_round_time - p * sgl_round_time - p * sgl_idle_time ) * ONE_MILLION ) ;
        double x_time = x * ( sgl_round_time + sgl_idle_time ) * ONE_MILLION ;
        double ratio = period / ( x_time + y ) ;
        y *= ratio , x *= ratio ;
        if( x < 0 || isnan( x ) ){
            x = 100 , y = 0 ;
        }
        module_sleepus = (int32_t)round( y ) ;
        module_runround = (int32_t)round( x ) ;
    }
    return ;
}

// void try_precise_usleep( int32_t sleepus ){
//     double start_time = time_now() ;
//     int32_t span , nxtsleep = sleepus / 100 , std100 = nxtsleep ;
//     for( int i = 1 ; i <= 100 ; i ++ ){
//         if( nxtsleep )
//             usleep( nxtsleep ) ;
//         span = (int)( ( time_now() - start_time ) * ONE_MILLION ) ;
//         if( span > i * std100 ){
//             if( i != 100 )
//                 nxtsleep = std::max( std100 - ( span - i * std100 ) , 0 ) ;
//             else 
//                 nxtsleep = std::max( sleepus - span , 0 ) ;
//         } else nxtsleep = std100 ;
//         if( span > sleepus ) break ;
//     }
// }


void pr_info( string info ){
    global_pr_mtx.lock() ;
    printf( "Info: %s\n" , info.c_str() ) ;
    global_pr_mtx.unlock() ;
}

void pr_info( char* info ){
    global_pr_mtx.lock() ;
    printf( "Info: %s\n" , info ) ;
    global_pr_mtx.unlock() ;
}

void pr_warning( string info ){
    global_pr_mtx.lock() ;
    printf( "Warning: %s\n" , info.c_str() ) ;
    global_pr_mtx.unlock() ;
}

void pr_warning( char* info ){
    global_pr_mtx.lock() ;
    printf( "Warning: %s\n" , info ) ;
    global_pr_mtx.unlock() ;
}

void pr_error( string info ){
    global_pr_mtx.lock() ;
    printf( "Error: %s\n" , info.c_str() ) ;
    global_pr_mtx.unlock() ;
}

void pr_error( char* info ){
    global_pr_mtx.lock() ;
    printf( "Error: %s\n" , info ) ;
    global_pr_mtx.unlock() ;
}

void pr_debug( string info ){
    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) == 0 )
        return ;
    global_pr_mtx.lock() ;
    printf( "DbgInfo: %s\n" , info.c_str() ) ;
    global_pr_mtx.unlock() ;
}

void pr_debug( char* info ){
    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) == 0 )
        return ;
    global_pr_mtx.lock() ;
    printf( "DbgInfo: %s\n" , info ) ;
    global_pr_mtx.unlock() ;
}

void pr_debug( void (*prfunc)() ){
    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) == 0 )
        return ;
    global_pr_mtx.lock() ;
    prfunc() ;
    global_pr_mtx.unlock() ;
}