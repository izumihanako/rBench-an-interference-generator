#include "rbench.hpp"

static pid_t main_pid ;
cpuinfo_t cpuinfo ;

static const struct option long_options[] = {
    { "run"     , required_argument , 0 , OPT_run     } ,
    { "threads" , required_argument , 0 , OPT_threads } ,
    { "limited" , optional_argument , 0 , OPT_limited } ,
    { "examine" , no_argument       , 0 , OPT_examine } ,
    { "cacheL1" , no_argument       , 0 , OPT_cacheL1 } ,
    { "cacheL2" , no_argument       , 0 , opt_cacheL2 } ,
    { "cacheL3" , no_argument       , 0 , opt_cacheL3 } ,
    { 0         , 0                 , 0 , 0 }
} ;

void parse_opts( int argc , char **argv ){
    
}

int main( int argc , char **argv , char **envp ){

    mwc_t rndeng ;
    printf( "%u %u,  %u\n" , rndeng.get_seed() , rndeng.mwc32() ) ;
    cpuinfo.get_cpuinfo() ;
    parse_opts( argc , argv ) ; 

    cpuinfo.print_cpuinfo() ;

    main_pid = getpid() ;

}