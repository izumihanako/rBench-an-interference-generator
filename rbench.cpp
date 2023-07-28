#include "rbench.hpp"

static pid_t main_pid ;
cpuinfo_t cpuinfo ;

static const help_info_t help_entrys[] = {
    { NULL           , "cacheL1"        , NULL         , "run cacheL1 stressor" } ,
    { NULL           , "cacheL2"        , NULL         , "run cacheL2 stressor" } ,
    { NULL           , "cacheL3"        , NULL         , "run cacheL3 stressor" } ,
    { NULL           , "cache-size N"   , NULL         , "run cache stressor with cache size N" } ,
    { NULL           , "examine"        , NULL         , "output examine information" } ,
    { "l N"          , "limited N"      , NULL         , "if limited, benchmark will stop after N rounds instead of running forever" } ,
    { "b W"          , "mem-bandwidth W", "mb W"       , "for memBw stressor, stress mem bw for W MB/s" },
    { "n N"          , "ninstance N"    , "instance N" , "start N instances of benchmark" } ,
    { "r NAME"       , "run NAME"       , NULL         , "run the specified benchmark"    } ,
    { "s P"          , "strength N"     , NULL         , "stress CPU by P %, for every instance" } ,
    { "t N"          , "time N"         , NULL         , "if specified, benchmark will stop after N seconds instead of running forever" } ,
    { NULL           , NULL             , NULL         , NULL }
} ;

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
    { "examine"       , no_argument       , 0 , OPT_examine       } ,
    { 0               , 0                 , 0 , 0                 }
} ;

void print_usage_help(){
	size_t i;
	const int cols = 80 ;

	for ( i = 0 ; help_entrys[i].description ; i++ ) {
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
    while( true ){
        if( ( c = getopt_long( argc , argv , "?r:n:l::t:s:b:" , long_options , &option_index ) ) == -1 ){
            break ;
        }
        switch( c ){
            case OPT_run :
            break ;
        }

    }
}

int main( int argc , char **argv , char **envp ){

    mwc_t rndeng ;
    printf( "%u %u,  %u\n" , rndeng.get_seed() , rndeng.mwc32() ) ;
    cpuinfo.get_cpuinfo() ;
    parse_opts( argc , argv ) ; 

    // cpuinfo.print_cpuinfo() ;

    // main_pid = getpid() ;

}