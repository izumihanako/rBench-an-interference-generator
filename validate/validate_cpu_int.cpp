#include <ctime>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std ;

#define BIL100 100000000000
#define BIL50   50000000000
#define BIL33d3 33333333333
#define BIL25   25000000000
#define BIL20   20000000000
#define BIL16d6 16666666666
#define BIL14d2 14285714285
#define BIL12d5 12500000000

void expand8(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 , d = 1 ;
    for( ; x <= BIL12d5 ; ){
        x ++ ; a ++ ;
        y ++ ; b ++ ;
        z ++ ; c ++ ;
        w ++ ; d ++ ;
    }
    if( x + y + z + w + a + b + c + d == BIL100 + 8 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand7(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 ;
    for( ; x <= BIL14d2 ; ){
        x ++ ; a ++ ;
        y ++ ; b ++ ;
        z ++ ; c ++ ;
        w ++ ; 
    }
    if( x + y + z + w + a + b + c == BIL100 + 2 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand6(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    for( ; x <= BIL16d6 ; ){
        x ++ ; a ++ ;
        y ++ ; b ++ ;
        z ++ ;
        w ++ ;
    }
    if( x + y + z + w + a + b == BIL100 + 2 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand5(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 , a = 1 ;
    for( ; x <= BIL20 ; ){
        x ++ ; a ++ ;
        y ++ ;
        z ++ ;
        w ++ ;
    }
    if( x + y + z + w + a == BIL100 + 5 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand4(){
    register long long x = 1 , y = 1 , z = 1 , w = 1 ;
    for( ; x <= BIL25 ; ){
        x ++ ;
        y ++ ;
        z ++ ;
        w ++ ;
    }
    if( x + y + z + w == BIL100 + 4 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand3(){
    register long long x = 1 , y = 1 , z = 1 ;
    for( ; x <= BIL33d3 ; ){
        x ++ ;
        y ++ ;
        z ++ ;
    }
    if( x + y + z == BIL100 + 2 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand2(){
    register long long x = 1 , y = 1 ;
    for( ; y <= BIL50 ; ){
        x ++ ;
        y ++ ;
    }
    if( x + y == BIL100 + 2 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand1(){
    register long long x = 1  ;
    for( ; x <= BIL100 ; ){
        x ++ ;
    }
    if( x == BIL100 + 1 ){
        printf( "OK\n%lld" , x ) ;
    } else printf( "ERROR\n" ) ;
}

int main(){
    time_t st , ed ;
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
// g++ validate_cpu_int.cpp -o validate_cpu_int -std=c++11 -O0