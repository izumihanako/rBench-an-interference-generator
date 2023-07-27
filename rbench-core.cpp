#include "rbench.hpp"

namespace mytime{

double OPTIMIZE3 timeval_to_double(const struct timeval *tv)
{
    return (double)tv->tv_sec + ((double)tv->tv_usec * ONE_MILLIONTH);
}

double OPTIMIZE3 time_now(void)
{
    timeval now;

    if (gettimeofday(&now, NULL) < 0)
        return -1.0;

    return timeval_to_double(&now);
}

}

cpuinfo_t::cpuinfo_t(){
    page_size = 0 ;
    core_count = 0 ;
    online_count = 0 ;
    cache_count = 0 ;
}

void cpuinfo_t::get_cpuinfo(){
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
    printf( "        size  : %lu KB\n" , caches[i].size ) ;
    printf( "        ways  : %d\n" , caches[i].ways ) ;
    printf( "        sets  : %d\n" , caches[i].sets ) ;
    }

}