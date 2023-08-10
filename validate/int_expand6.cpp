#include <ctime>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std ;

#define EXP1 10000000000
#define EXP2  5000000000
#define EXP3  3333333333
#define EXP4  2500000000
#define EXP5  2000000000
#define EXP6  1666666667
#define EXP7  1428571428
#define EXP8  1250000000

void expand6(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    static long long ans = 10000000003 ;
    for( ; w <= EXP6 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        a -= a - w ;
        b -= b - w ;
        w = w + 1 ;
    }
    long long cal = x + y + z + w + a + b ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

int main(){
    time_t st , ed ;
    while( true ){
        st = clock() ;
        expand6() ;
        ed = clock() ;
        printf( "expand6 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;
    }

}
// g++ int_expand6.cpp -o int_expand6.exe -std=c++11 -O0