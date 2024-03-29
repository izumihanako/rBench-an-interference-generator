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

void expand8(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 , d = 1 ;
    static long long ans = 10000000001 ;
    for( ; w <= EXP8 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        a -= a - w ;
        b -= b - w ;
        c -= c - w ;
        d -= d - w ;
        w = w + 1 ;
    }
    long long cal = x + y + z + w + a + b + c + d ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

void expand7(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 ;
    static long long ans = 9999999997 ;
    for( ; w <= EXP7 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        a -= a - w ;
        b -= b - w ;
        c -= c - w ;
        w = w + 1 ;
    }
    long long cal = x + y + z + w + a + b + c ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}


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

void expand5(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 ;
    static long long ans = 10000000001 ;
    for( ; w <= EXP5 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        a -= a - w ;
        w = w + 1 ;
    }
    long long cal = x + y + z + w + a ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

void expand4(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 ;
    static long long ans = 10000000001 ;
    for( ; w <= EXP4 ; ){
        x -= x - w ;
        y -= y - w ;
        z -= z - w ;
        w = w + 1 ;
    }
    long long cal = x + y + z + w ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

void expand3(){
    register long long x = 1 , y = 1 , z = 1 ;
    static long long ans = 10000000000 ;
    for( ; z <= EXP3 ; ){
        x -= x - z ;
        y -= y - z ;
        z = z + 1 ;
    }
    long long cal = x + y + z ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

void expand2(){
    register long long x = 1 , y = 1 ;
    static long long ans = 10000000001 ;
    for( ; y <= EXP2 ; ){
        x -= x - y ;
        y = y + 1 ;
    }
    long long cal = x + y ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

void expand1(){
    register long long x = 1 ;
    static long long ans = 10000000001 ;
    for( ; x <= EXP1 ; x ++ ){
        x = x + 1 ;
    }
    long long cal = x ;
    if( cal == ans ){
        printf( "OK: %lld\n" , cal ) ;
    } else printf( "ERROR %lld, expect %lld\n" , cal , ans ) ;
}

int main(){
    time_t st , ed ;
    while( true ){
        st = clock() ;
        expand1() ;
        ed = clock() ;
        printf( "expand1 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand2() ;
        ed = clock() ;
        printf( "expand2 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand3() ;
        ed = clock() ;
        printf( "expand3 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand4() ;
        ed = clock() ;
        printf( "expand4 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand5() ;
        ed = clock() ;
        printf( "expand5 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand6() ;
        ed = clock() ;
        printf( "expand6 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand7() ;
        ed = clock() ;
        printf( "expand7 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand8() ;
        ed = clock() ;
        printf( "expand8 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;
    }
}
// g++ validate_cpu_int.cpp -o validate_cpu_int -std=c++11 -O0