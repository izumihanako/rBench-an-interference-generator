#include "rbench.hpp"

int main(){
    mwc_t rndeng ;
    printf( "%u %u,  %u\n" , rndeng.get_seed() , rndeng.mwc32() ) ;
    cpuinfo_t cpuinfo ;
    cpuinfo.get_cpuinfo() ;
    cpuinfo.print_cpuinfo() ;
}