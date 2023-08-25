#include <ctime>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std ;

#define EXP1 1000000000
#define EXP2  500000000
#define EXP3  333333333
#define EXP4  250000000
#define EXP5  200000000
#define EXP6  166666667

void expand6(){
    register int x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    static int ans = 166666668 ;
    for( ; w <= EXP6 ; ){
        x = x / ( x ) ;
        y = y / ( y ) ;
        z = z / ( z ) ;
        a = a / ( a ) ;
        b = b / ( b ) ;
        w = w + ( w / w ) ;
    }
    int cal = x + y + z + w + a + b ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
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
// g++ int_divexpand6.cpp -o int_divexpand6.exe -std=c++11 -O0