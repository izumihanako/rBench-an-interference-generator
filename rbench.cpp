#include "rbench.hpp"

mutex global_pr_mtx ;
static pid_t main_pid ;
cpuinfo_t cpuinfo ;
uint32_t global_flag ;

// help options
static const help_info_t help_entrys[] = {
    { NULL           , "cacheL1"        , NULL         , "run cacheL1 stressor" } ,
    { NULL           , "cacheL2"        , NULL         , "run cacheL2 stressor" } ,
    { NULL           , "cacheL3"        , NULL         , "run cacheL3 stressor" } ,
    { NULL           , "cache-size N"   , NULL         , "run cache stressor with cache size N" } ,
    { NULL           , "check"          , NULL         , "run preset system check missions" } ,
    { NULL           , "debug"          , NULL         , "output debug information" } ,
    { "l N"          , "limited N"      , NULL         , "if limited, benchmark will stop after N rounds instead of running forever" } ,
    { "b W"          , "mem-bandwidth W", "mb W"       , "for memBw stressor, stress mem bw for W MB/s" },
    { "n N"          , "ninstance N"    , "instance N" , "start N instances of benchmark" } ,
    { NULL           , "period N"       , NULL         , "If specified, the time granularity is N microseconds" } ,
    { "r NAME"       , "run NAME"       , NULL         , "run the specified benchmark"    } ,
    { "s P"          , "strength N"     , NULL         , "stress CPU by P%, for every instance ( run P% time per time granularity )" } ,
    { "t N"          , "time N"         , NULL         , "if specified, benchmark will stop after N seconds instead of running forever" } ,
    { NULL           , NULL             , NULL         , NULL }
} ;

// GNU "long options" command line options
static const struct option long_options[] = {
    { "limited"       , optional_argument , 0 , OPT_limited       } ,
    { "mem-bandwidth" , required_argument , 0 , OPT_mem_bandwidth } ,
    { "mb"            , required_argument , 0 , OPT_mem_bandwidth } ,
    { "ninstance"     , required_argument , 0 , OPT_ninstance     } ,
    { "instance"      , required_argument , 0 , OPT_ninstance     } ,
    { "run"           , required_argument , 0 , OPT_run           } ,
    { "strength"      , required_argument , 0 , OPT_strength      } ,
    { "time"          , required_argument , 0 , OPT_time          } ,
    { "cacheL1"       , no_argument       , 0 , OPT_cacheL1       } ,
    { "cacheL2"       , no_argument       , 0 , OPT_cacheL2       } ,
    { "cacheL3"       , no_argument       , 0 , OPT_cacheL3       } ,
    { "cache-size"    , required_argument , 0 , OPT_cache_size    } ,
    { "check"         , no_argument       , 0 , OPT_check         } ,
    { "debug"         , no_argument       , 0 , OPT_debug         } ,
    { "period"        , required_argument , 0 , OPT_period        } ,
    { 0               , 0                 , 0 , 0                 }
} ;

static const map<string , bench_func_t > bench_funcs = {
    pair< string , bench_func_t >( "cache"   , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL1" , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL2" , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL3" , &cache_bench_entry ) ,
} ;

struct bench_mission_t{
    bench_func_t func ;
    bench_args_t args ;
    bench_mission_t( const bench_func_t &func_ = NULL , const bench_args_t &args_ = bench_args_t() ){
        func = func_ ;
        args = args_ ;
    }
} ;
vector<bench_mission_t> bench_missions ;

void print_usage_help(){
	const int cols = 80 ;
	for ( int32_t i = 0 ; help_entrys[i].description ; i++ ) {
		char opt_short[10] = "";
		int wd = 0;
		bool first = true ;
		const char *ptr, *space = NULL;
		const char *start = help_entrys[i].description;

		if( help_entrys[i].opt_short )
			snprintf( opt_short, sizeof(opt_short), "-%s," , help_entrys[i].opt_short ) ;
		printf( "%-10s--%-22s" , opt_short , help_entrys[i].opt_long ) ;
        if( help_entrys[i].opt_alter )
            printf( "\n%-10s--%-22s" , " " , help_entrys[i].opt_alter ) ;

		for (ptr = start; *ptr; ptr++) {
			if (*ptr == ' ')
				space = ptr;
			wd++;
			if (wd >= cols - 34) {
				const size_t n = (size_t)(space - start);

				if (!first)
					(void)printf("%-34s", "");
				first = false;
				(void)printf("%*.*s\n", (int)n, (int)n, start);
				start = space + 1;
				wd = 0;
			}
		}
		if (start != ptr) {
			const int n = (int)(ptr - start);
			if (!first)
				(void)printf("%-34s", "");
			(void)printf("%*.*s\n", n, n, start);
		}
        if( ( !first ) || help_entrys[i].opt_alter )
            printf( "\n" ) ;
	}
    printf( "Note:\n") ;
    printf( "  PLEASE specify the benchmark project BEFORE specifying the runtime parameters!!\n\n" ) ;
}

void parse_opts( int argc , char **argv ){
    // usage of getopt 
    // https://linux.die.net/man/3/getopt_long
    if( argc == 1 ){
        print_usage_help() ;
        return ;
    }
    ::optind = 0 ;
    int c , option_index ;
    bench_args_t null_buffer , *pargs = &null_buffer ;
    char infobuf[1024] ;
    while( true ){
        if( ( c = getopt_long( argc , argv , "?r:n:l::t:s:b:" , long_options , &option_index ) ) == -1 ){
            break ;
        }
        switch( c ){
            case OPT_ninstance:{
                int32_t i32 = atoi( optarg ) ;
                if( i32 < 1 || i32 > cpuinfo.online_count ){
                    int32_t newi32 = i32 < 1 ? 1 : cpuinfo.online_count ;
                    sprintf( infobuf , "ninstance (%d) needs to be in range [%d,%d], the number is adjust to %d" ,
                                        i32 , 1 , cpuinfo.online_count , newi32 ) ;
                    pr_warning( infobuf ) ;
                    i32 = newi32 ;
                }
                pargs->threads = i32 ;
                break ;
            }
            case OPT_run :{
                string bench_name = string( optarg ) ;
                if( bench_funcs.count( bench_name ) ) {
                    bench_missions.emplace_back( (*bench_funcs.find( bench_name )).second , *(new bench_args_t()) ) ;
                    pargs = &( *bench_missions.rbegin() ).args ;
                    if( !strncasecmp( optarg , "cacheL1" , 7 ) ){
                        pargs->cache_size = cpuinfo.get_data_cache_size_level( 1 ) ;
                    } else if( !strncasecmp( optarg , "cacheL2" , 7 ) ){
                        pargs->cache_size = cpuinfo.get_data_cache_size_level( 2 ) ;
                    } else if( !strncasecmp( optarg , "cacheL3" , 7 ) ){
                        pargs->cache_size = cpuinfo.get_data_cache_size_level( 3 ) ;
                    }
                } else {
                    string warnstr = string( "no such benchmark named " ) + string( optarg ) ;
                    pr_warning( warnstr ) ;
                }
                break ;
            }
            case OPT_strength:{
                uint32_t u32 = atoi( optarg ) ;
                pargs->strength = u32 ;
                break ;
            }
            case OPT_time:{
                int32_t i32 = atoi( optarg ) ;
                if( i32 <= 0 ){
                    sprintf( infobuf , "time limit (%d) needs to be a positive number, ignored" , i32 ) ;
                    pr_warning( infobuf ) ;
                    i32 = 0 ;
                }
                pargs->time = (uint32_t) i32 ;
                break ;
            }
            // long options :
            case OPT_cacheL1:{
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 1 ) ;
                break ;
            }
            case OPT_cacheL2:{
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 2 ) ;
                break ;
            }
            case OPT_cacheL3:{
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 3 ) ;
                break ;
            }
            case OPT_cache_size :{
                uint32_t cache_size = atoi( optarg ) ;
                pargs->cache_size = cache_size ;
                break ;
            }
            case OPT_check :{
                break ;
            }
            case OPT_debug :{
                set_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) ;
                break ;
            }
            case OPT_period :{
                int32_t i32 = atoi( optarg ) ;
                if( i32 < 5000 ){
                    sprintf( infobuf , "If the period (%dus) is too small, the load control will be inaccurate" , i32 ) ;
                    pr_warning( infobuf ) ;
                }
                pargs->period = (uint32_t)i32 ;
                break ;
            }
            default :{
                pr_error( string( "Unknown argument" ) ) ;
                break ;
            }
        }
    }
    for( auto mission : bench_missions ){
        mission.args.flags |= global_flag ;
    }
}

int main( int argc , char **argv , char **envp ){
    main_pid = getpid() ;
    cpuinfo.read_cpuinfo() ;

    parse_opts( argc , argv ) ; 

    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO) ){
        mwc_t rndeng ;
        rndeng.print_info() ;
        cpuinfo.print_cpuinfo() ;
    }

    for( auto mission : bench_missions ){
        mission.func( mission.args ) ;
    }
}