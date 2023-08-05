suffix = .exe

all: validate_cacheL1.exe validate_cacheL2.exe validate_cacheL3.exe \
	validate_cpu_float.exe validate_cpu_int.exe validate_tlb.exe

validate_cacheL1.exe: validate_cacheL1.c
	gcc validate_cacheL1.c -o validate_cacheL1$(suffix) -std=gnu11 -O0

validate_cacheL2.exe: validate_cacheL2.c
	gcc validate_cacheL2.c -o validate_cacheL2$(suffix) -std=gnu11 -O0

validate_cacheL3.exe: validate_cacheL3.c
	gcc validate_cacheL3.c -o validate_cacheL3$(suffix) -std=gnu11 -O0

validate_cpu_float.exe: validate_cpu_float.cpp
	g++ validate_cpu_float.cpp -o validate_cpu_float$(suffix) -std=c++11 -O0

validate_cpu_int.exe: validate_cpu_int.cpp
	g++ validate_cpu_int.cpp -o validate_cpu_int$(suffix) -std=c++11 -O0

validate_tlb.exe: validate_tlb.c
	gcc validate_tlb.c -o validate_tlb$(suffix) -std=gnu11 -O0

.PHONY: clean
clean:
	rm -f ${DESTDIR}*.exe
	rm -f ${DESTDIR}*.o 