#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std ;

#define EXP6  1666666667

void expand6(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    static double ans = 10000000003 ;
    for( ; w <= EXP6 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        a -= a - w ;
        b -= b - w ;
        w = w + 1 ;
    }
    double cal = x + y + z + w + a + b ;
    if( cal == ans ){
        printf( "OK: %lf\n" , cal ) ;
    } else printf( "ERROR %lf, expect %lf\n" , cal , ans ) ;
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
// g++ float_expand6.cpp -o float_expand6.exe -std=c++11 -O0
// taskset -c 20 ./float_expand6.exe