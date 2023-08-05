#include <ctime>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std ;

#define BIL100 10000000000
#define BIL50   5000000000
#define BIL33d3 3333333333
#define BIL25   2500000000
#define BIL20   2000000000
#define BIL16d6 1666666666
#define BIL14d2 1428571428
#define BIL12d5 1250000000

void expand8(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 , d = 1 ;
    for( ; x <= BIL12d5 ; ){
        x += 1.0 ; a += 1.0 ;
        y += 1.0 ; b += 1.0 ;
        z += 1.0 ; c += 1.0 ;
        w += 1.0 ; d += 1.0 ;
    }
    if( x + y + z + w + a + b + c + d == BIL100 + 8 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand7(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 ;
    for( ; x <= BIL14d2 ; ){
        x += 1.0 ; a += 1.0 ;
        y += 1.0 ; b += 1.0 ;
        z += 1.0 ; c += 1.0 ;
        w += 1.0 ; 
    }
    if( fabs( x + y + z + w + a + b + c - BIL100 - 3 ) < 1e-7 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand6(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    for( ; x <= BIL16d6 ; ){
        x += 1.0 ; a += 1.0 ;
        y += 1.0 ; b += 1.0 ;
        z += 1.0 ;
        w += 1.0 ;
    }
    if( fabs( x + y + z + w + a + b - BIL100 - 2 ) < 1e-7 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand5(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 ;
    for( ; x <= BIL20 ; ){
        x += 1.0 ; a += 1.0 ;
        y += 1.0 ;
        z += 1.0 ;
        w += 1.0 ;
    }
    if( fabs( x + y + z + w + a - BIL100 - 5 ) < 1e-7 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand4(){
    register double x = 1 , y = 1 , z = 1 , w = 1 ;
    for( ; x <= BIL25 ; ){
        x += 1.0 ;
        y += 1.0 ;
        z += 1.0 ;
        w += 1.0 ;
    }
    if( x + y + z + w == BIL100 + 4 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand3(){
    register double x = 1 , y = 1 , z = 1 ;
    for( ; x <= BIL33d3 ; ){
        x += 1.0 ;
        y += 1.0 ;
        z += 1.0 ;
    }
    if( x + y + z == BIL100 + 2 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand2(){
    register double x = 1 , y = 1 ;
    for( ; y <= BIL50 ; ){
        x += 1.0 ;
        y += 1.0 ;
    }
    if( x + y == BIL100 + 2 ){
        printf( "OK\n%.2f" , x ) ;
    } else printf( "ERROR\n" ) ;
}

void expand1(){
    register double x = 1  ;
    for( ; x <= BIL100 ; ){
        x += 1.0 ;
    }
    if( x == BIL100 + 1 ){
        printf( "OK\n%.2f" , x ) ;
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
// g+= 1.0 validate_cpu_float.cpp -o validate_cpu_float -std=c+= 1.011 -O0
// taskset -c 20 ./validate_cpu_float.exe