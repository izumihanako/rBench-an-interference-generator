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
#define EXP7  142857142
#define EXP8  125000000

void expand8(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 , d = 1 ;
    static double ans = EXP8 + 1 ;
    for( ; w <= EXP8 ; ){
        x /= ( w / x ) ;
        y /= ( w / y ) ;
        z /= ( w / z ) ;
        a /= ( w / a ) ;
        b /= ( w / b ) ;
        c /= ( w / c ) ;
        d /= ( w / d ) ;
        w += ( w / w ) ;
    }
    double cal = x + y + z + w + a + b + c + d ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand7(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 , c = 1 ;
    static double ans = EXP7 + 1 ;
    for( ; w <= EXP7 ; ){
        x /= ( w / x ) ;
        y /= ( w / y ) ;
        z /= ( w / z ) ;
        a /= ( w / a ) ;
        b /= ( w / b ) ;
        c /= ( w / c ) ;
        w += ( w / w ) ;
    }
    double cal = x + y + z + w + a + b + c ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand6(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 , b = 1 ;
    static double ans = 166666668.000000 ;
    for( ; w <= EXP6 ; ){
        x /= ( w / x ) ;
        y /= ( w / y ) ;
        z /= ( w / z ) ;
        a /= ( w / a ) ;
        b /= ( w / b ) ;
        w += ( w / w ) ;
    }
    double cal = x + y + z + w + a + b ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand5(){
    register double x = 1 , y = 1 , z = 1 , w = 1 , a = 1 ;
    static double ans = 200000001.000000 ;
    for( ; w <= EXP5 ; ){
        x /= ( w / x ) ;
        y /= ( w / y ) ;
        z /= ( w / z ) ;
        a /= ( w / a ) ;
        w += ( w / w ) ;
    }
    double cal = x + y + z + w + a ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand4(){
    register double x = 1 , y = 1 , z = 1 , w = 1 ;
    static double ans = 250000001.000000 ;
    for( ; w <= EXP4 ; ){
        x /= ( w / x ) ;
        y /= ( w / y ) ;
        z /= ( w / z ) ;
        w += ( w / w ) ;
    }
    double cal = x + y + z + w ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand3(){
    register double x = 1 , y = 1 , z = 1 ;
    static double ans = 333333334.000000 ;
    for( ; z <= EXP3 ; ){
        x /= ( z / x ) ;
        y /= ( z / y ) ;
        z += ( z / z ) ;
    }
    double cal = x + y + z ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand2(){
    register double x = 1 , y = 1 ;
    static double ans = 500000001.000000 ;
    for( ; y <= EXP2 ; ){
        x /= ( y / x ) ;
        y += ( y / y ) ;
    }
    double cal = x + y ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

void expand1(){
    register double x = 1 ;
    static double ans = 1000000001 ;
    for( ; x <= EXP1 ; x ++ ){
        x += ( x / x ) ;
    }
    double cal = x ;
    if( cal == ans ){
        printf( "OK: %f\n" , cal ) ;
    } else printf( "ERROR %f, expect %f\n" , cal , ans ) ;
}

int main(){
    time_t st , ed ;
    while( true ){
        // st = clock() ;
        // expand1() ;
        // ed = clock() ;
        // printf( "expand1 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand2() ;
        // ed = clock() ;
        // printf( "expand2 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand3() ;
        // ed = clock() ;
        // printf( "expand3 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand4() ;
        // ed = clock() ;
        // printf( "expand4 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand5() ;
        // ed = clock() ;
        // printf( "expand5 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand6() ;
        // ed = clock() ;
        // printf( "expand6 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        // st = clock() ;
        // expand7() ;
        // ed = clock() ;
        // printf( "expand7 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;

        st = clock() ;
        expand8() ;
        ed = clock() ;
        printf( "expand8 : %.2f\n" , 1.0 * ( ed - st ) / CLOCKS_PER_SEC ) ;
    }

}
// g++ validate_cpu_int.cpp -o validate_cpu_int -std=c++11 -O0