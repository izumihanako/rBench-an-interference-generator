#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int loopcount = 2000 ;
int roundtime = 1 ;

void printhead(){
    printf( "    .file \"rbench-cpu-l1i-kernel.asm\"\n" ) ;
    printf( "    .text\n" ) ;
    // printf( "    .global _start\n" ) ;
    printf( "    .type   _start, @function\n" ) ;
    printf( "_start:\n" ) ;
    printf( "    pushq %%rbp\n" ) ;
    printf( "    movq  %%rsp, %%rbp\n" ) ;
    printf( "    call  cpu_l1i_kernel\n" ) ;
    printf( "    movl  $0, %%eax\n" ) ;
    printf( "    leave\n" ) ;
    printf( "    ret\n\n" ) ;

    printf( "    .global cpu_l1i_kernel\n" ) ;
    printf( "    .type   cpu_l1i_kernel, @function\n" ) ;
    printf( "cpu_l1i_kernel:\n" ) ;
    printf( "    pushq %%rbp\n" ) ;
    printf( "    movq  %%rsp, %%rbp\n" ) ;
    printf( "    movabsq $%d, %%r9\n" , roundtime ) ;
    printf( "    movabsq $%lld, %%r10\n" , 1 + ( loopcount + 1 ) * roundtime * 64ll ) ;
    printf( "    .LBEGIN_FOR:\n") ;
}

void printloop( int id ) {
    int base = id * 64 ;
    for( int i = 1 ; i <= 64 ; i ++ ){
        printf( "    .LABAL%d:\n" , base + i ) ;
        printf( "    subq $0x1, %%r10\n" ) ;
        printf( "    jmp .LABAL%d\n" , base + i + 64 ) ;
    }
}

void printendloop( int id ){
    int base = id * 64 ;
    for( int i = 1 ; i < 64 ; i ++ ){
        printf( "    .LABAL%d:\n" , base + i ) ;
        printf( "    subq $0x1, %%r10\n" ) ;
        printf( "    jmp .LABAL%d\n" , i + 1 ) ;
    }
    printf( "    .LABAL%d:\n" , base + 64 ) ;
    printf( "    subq $0x1, %%r10\n" ) ;
    printf( "    dec %%r9\n" ) ;
    printf( "    jne .LBEGIN_FOR\n" ) ;
}

void printfoot(){
    // exit(0) : 
    // printf( "    movq %%r10, %%rdi\n" ) ;
    // printf( "    movq $60, %%rax\n" ) ;
    // printf( "    syscall\n" ) ;
    // ret :
    printf( "    movq %%r10, %%rax\n" ) ;
    printf( "    leave\n" ) ;
    printf( "    ret\n\n" ) ;
}

int main(){
    printhead() ;
    for( int i = 0 ; i < loopcount ; i ++ ){
        printloop( i ) ;
    }
    printendloop( loopcount ) ;
    printfoot() ;
}


/*
as -c jumpl1i.asm -o jumpl1i.o
ld jumpl1i.o -o jumpl1i.exe
echo $? 
should return 1

perf stat -B -r 1 \
          -o jumpl1iperf.txt \
          -e cycles -e instructions \
          -e L1-icache-load-misses \
          -e machine_clears.count -e mem_inst_retired.all_loads \
          -e mem_load_retired.fb_hit \
          -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss \
          -e mem_load_retired.l2_hit -e mem_load_retired.l2_miss \
          -e mem_load_retired.l3_hit -e mem_load_retired.l3_miss \
          taskset -c 0 ./jumpl1i

cgexec -g cpuset,cpu:SMC100 ./jumpl1i_clang 

gcc bubblecache.c -o bubblecache -std=gnu11 -O0 && \
perf stat -B -r 5 \
          -o jumpl1ibubble100.txt \
          -e cycles -e instructions \
          -e L1-icache-load-misses \
          -e machine_clears.count -e mem_inst_retired.all_loads \
          -e mem_load_retired.fb_hit \
          -e mem_load_retired.l1_hit -e mem_load_retired.l1_miss \
          -e mem_load_retired.l2_hit -e mem_load_retired.l2_miss \
          -e mem_load_retired.l3_hit -e mem_load_retired.l3_miss \
          taskset -c 0 ./bubblecache
*/