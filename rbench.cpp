#include "rbench.hpp"

int main(){
    mwc_t rndeng ;
    printf( "%u %u,  %u\n" , rndeng.get_seed() , rndeng.mwc32() ) ;
}