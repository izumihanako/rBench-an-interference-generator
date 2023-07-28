#include "rbench.hpp"

static pid_t main_pid ;
cpuinfo_t cpuinfo ;

static const struct option long_options[] = {
    { "run"           , required_argument , 0 , OPT_run           } ,
    { "ninstance"     , required_argument , 0 , OPT_ninstance     } ,
    { "instance"      , required_argument , 0 , OPT_ninstance     } ,
    { "limited"       , optional_argument , 0 , OPT_limited       } ,
    { "time"          , required_argument , 0 , OPT_time          } ,
    { "strength"      , required_argument , 0 , OPT_strength      } ,
    { "mem-bandwidth" , required_argument , 0 , OPT_mem_bandwidth } ,
    { "mb"            , required_argument , 0 , OPT_mem_bandwidth } ,
    { "examine"       , no_argument       , 0 , OPT_examine       } ,
    { "cacheL1"       , no_argument       , 0 , OPT_cacheL1       } ,
    { "cacheL2"       , no_argument       , 0 , opt_cacheL2       } ,
    { "cacheL3"       , no_argument       , 0 , opt_cacheL3       } ,
    { 0               , 0                 , 0 , 0                 }
} ;

void parse_opts( int argc , char **argv ){
    // usage of getopt 
    // https://linux.die.net/man/3/getopt_long
    ::optind = 0 ;
    int c , option_index ;
    while( true ){
        if( ( c = getopt_long( argc , argv , "?r:n:l::t:s:b:" , long_options , &option_index ) ) == -1 ){
            break ;
        }
        
    }
}

int main( int argc , char **argv , char **envp ){

    mwc_t rndeng ;
    printf( "%u %u,  %u\n" , rndeng.get_seed() , rndeng.mwc32() ) ;
    cpuinfo.get_cpuinfo() ;
    parse_opts( argc , argv ) ; 

    cpuinfo.print_cpuinfo() ;

    main_pid = getpid() ;

}