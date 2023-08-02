#include "rbench.hpp"

mutex global_pr_mtx ;
static pid_t main_pid ;
cpuinfo_t cpuinfo ;
uint32_t global_flag ;
vector<thread> glob_threads ;
int32_t glob_thr_cnt ;

// help options
static const help_info_t help_entrys[] = {
    { NULL           , "cache-size N"   , NULL         , "(stressor) specify the size of the cache buffer of the cache stressor as N" } ,
    { NULL           , "check"          , NULL         , "(global) run preset system check tasks. If this option is present, all other options will be ignored" } ,
    { NULL           , "debug"          , NULL         , "(global) output debug information" } ,
    { "l N"          , "limited N"      , NULL         , "(stressor) if limited, benchmark will stop after N rounds instead of running forever" } ,
    { "b W"          , "mem-bandwidth W", "mb W"       , "(stressor) for memBw stressor, stress mem bw for W MB/s" },
    { "n N"          , "ninstance N"    , "instance N" , "(stressor) start N instances of benchmark" } ,
    { NULL           , "parallel"       , NULL         , "(global) run in parallel mode"} ,
    { NULL           , "period N"       , NULL         , "(stressor) If specified, the time granularity is N microseconds" } ,
    { "r NAME"       , "run NAME"       , NULL         , "(stressor) run the specified benchmark. Supported test items are cacheL1, cacheL2, "
                                                         "cacheL3, cache, cpu-int, "    } ,
    { "s P"          , "strength N"     , NULL         , "(stressor) set load strength to P% for every instance (run P% time per time granularity)" } ,
    { "t N"          , "time N"         , NULL         , "(stressor) if specified, benchmark will stop after N seconds instead of running forever" } ,
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
    { "cache-size"    , required_argument , 0 , OPT_cache_size    } ,
    { "check"         , no_argument       , 0 , OPT_check         } ,
    { "debug"         , no_argument       , 0 , OPT_debug         } ,
    { "parallel"      , no_argument       , 0 , OPT_parallel      } ,
    { "period"        , required_argument , 0 , OPT_period        } ,
    { 0               , 0                 , 0 , 0                 }
} ;

static const map<string , bench_func_t > bench_funcs = {
    pair< string , bench_func_t >( "cache"   , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL1" , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL2" , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cacheL3" , &cache_bench_entry ) ,
    pair< string , bench_func_t >( "cpu-int" , &cpu_int_bench_entry ) ,
} ;

struct bench_task_t{
    bench_func_t func ;
    bench_args_t args ;
    bench_task_t( const bench_func_t &func_ = NULL , const bench_args_t &args_ = bench_args_t() ){
        func = func_ ;
        args = args_ ;
    }
} ;
vector<bench_task_t> bench_tasks ;

void print_usage_help(){
    printf( "Usage :\n" ) ;
	const int cols = 80 ;
	for ( int32_t i = 0 ; help_entrys[i].description ; i++ ) {
		char opt_short[10] = "";
		int wd = 0;
		bool first = true ;
		const char *ptr, *space = NULL;
		const char *start = help_entrys[i].description;

		if( help_entrys[i].opt_short )
			snprintf( opt_short, sizeof(opt_short), "-%s," , help_entrys[i].opt_short ) ;
		printf( "%-9s--%-20s" , opt_short , help_entrys[i].opt_long ) ;
        if( help_entrys[i].opt_alter )
            printf( "\n%-9s--%-20s" , " " , help_entrys[i].opt_alter ) ;

		for (ptr = start; *ptr; ptr++) {
			if (*ptr == ' ')
				space = ptr;
			wd++;
			if (wd >= cols - 20) {
				const size_t n = (size_t)(space - start);

				if (!first)
					(void)printf("%-31s", "");
				first = false;
				(void)printf("%*.*s\n", (int)n, (int)n, start);
				start = space + 1;
				wd = 0;
			}
		}
		if (start != ptr) {
			const int n = (int)(ptr - start);
			if (!first)
				(void)printf("%-31s", "");
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
        if( get_arg_flag( global_flag , FLAG_IS_CHECK ) ) break ;
        if( ( c = getopt_long( argc , argv , "?r:n:l::t:s:b:" , long_options , &option_index ) ) == -1 ){
            break ;
        }
        switch( c ){
            case OPT_limited:{
                set_arg_flag( pargs->flags , FLAG_IS_LIMITED ) ;
                if( !optarg ) pargs->limit_round = 100 ;
                else {
                    int64_t i64 = atoll( optarg ) ;
                    pargs->limit_round = i64 ;
                }
                break ;
            }
            case OPT_ninstance:{
                int32_t i32 = (int32_t)atoi( optarg ) ;
                if( i32 < 1 || i32 > cpuinfo.online_count ){
                    int32_t newi32 = i32 < 1 ? 1 : cpuinfo.online_count ;
                    sprintf( infobuf , "ninstance (%d) needs to be in range [%d,%d], the number is adjust to %d" ,
                                        i32 , 1 , cpuinfo.online_count , newi32 ) ;
                    pr_warning( infobuf ) ;
                    i32 = newi32 ;
                }
                pargs->threads = (uint16_t) i32 ;
                break ;
            }
            case OPT_run :{
                string bench_name = string( optarg ) ;
                if( bench_funcs.count( bench_name ) ) {
                    bench_tasks.emplace_back( (*bench_funcs.find( bench_name )).second , *(new bench_args_t()) ) ;
                    pargs = &( *bench_tasks.rbegin() ).args ;
                    pargs->bench_name = bench_name ;

                    // cache size level setting
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
                int32_t i32 = atoi( optarg ) ;
                if( i32 < 1 || i32 > 100 ){
                    int16_t newi32 = i32 < 1 ? 1 : 100 ;
                    sprintf( infobuf , "strength (%d) needs to be in range [%d,%d], the number is adjust to %d" ,
                                        i32 , 1 , 100 , newi32 ) ;
                    pr_warning( infobuf ) ;
                    i32 = newi32 ;
                }
                pargs->strength = (uint8_t) i32 ;
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
            case OPT_cache_size :{
                uint32_t cache_size = atoi( optarg ) ;
                pargs->cache_size = cache_size ;
                break ;
            }
            case OPT_check :{
                set_arg_flag( global_flag , FLAG_IS_CHECK ) ;
                set_arg_flag( global_flag , FLAG_IS_LIMITED ) ;
                // int speed
                bench_tasks.emplace_back(  (*bench_funcs.find( string( "cpu-int" ) ) ).second , *(new bench_args_t()) ) ;
                pargs = &( *bench_tasks.rbegin() ).args ;
                pargs->bench_name = string( "cpu-int" ) ;
                pargs->limit_round = ONE_THOUSAND * 20 ;
                // cacheL1 speed
                bench_tasks.emplace_back( (*bench_funcs.find( string( "cacheL1" ) ) ).second , *(new bench_args_t()) ) ;
                pargs = &( *bench_tasks.rbegin() ).args ;
                pargs->bench_name = string( "cacheL1" ) ;
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 1 ) ;
                pargs->limit_round = ONE_THOUSAND * 100 ;
                // cacheL2 speed
                bench_tasks.emplace_back( (*bench_funcs.find( string( "cacheL2" ) ) ).second , *(new bench_args_t()) ) ;
                pargs = &( *bench_tasks.rbegin() ).args ;
                pargs->bench_name = string( "cacheL2" ) ;
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 2 ) ;
                pargs->limit_round = ONE_THOUSAND * 100  ;
                // cacheL3 speed
                bench_tasks.emplace_back( (*bench_funcs.find( string( "cacheL3" ) ) ).second , *(new bench_args_t()) ) ;
                pargs = &( *bench_tasks.rbegin() ).args ;
                pargs->bench_name = string( "cacheL3" ) ;
                pargs->cache_size = cpuinfo.get_data_cache_size_level( 3 ) ;
                pargs->limit_round = ONE_THOUSAND * 100  ;
                break ;
            }
            case OPT_debug :{
                set_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) ;
                break ;
            }
            case OPT_parallel :{
                set_arg_flag( global_flag , FLAG_IS_RUN_PARALLEL ) ;
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
    for( auto &task : bench_tasks ){
        task.args.flags |= global_flag ;
    }
}

int main( int argc , char **argv , char **envp ){
    main_pid = getpid() ;
    cpuinfo.read_cpuinfo() ;
    glob_threads.resize( 1000 ) ;
    glob_thr_cnt = 0 ;

    parse_opts( argc , argv ) ; 

    if( get_arg_flag( global_flag , FLAG_PRINT_DEBUG_INFO ) ||
        get_arg_flag( global_flag , FLAG_IS_CHECK ) ){
        mwc_t rndeng ;
        rndeng.print_info() ;
        cpuinfo.print_cpuinfo() ;
    }

    for( auto task : bench_tasks ){
        task.func( task.args ) ;
    }
    glob_threads.resize( glob_thr_cnt ) ;
    for( auto &thr : glob_threads ){
        thr.join() ;
    }
}