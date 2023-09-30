# configure options
#----------------------#

# support AVX512 or not, value can be "true" or "false"
AVX512SUPPORT = false
# link tool
LD = g++  
# C&C++ compiler
CC = g++
# .asm compiler 
AS = as

#----------------------#


VERSION=0.0.1

ROOTDIR = $(PWD)
VALIDATEDIR  = $(ROOTDIR)/validate

KERNEL=$(shell uname -s)
NODENAME=$(shell uname -n)

CFLAGS = -Wall -Wextra -Wconversion -DVERSION='"$(VERSION)"' -std=gnu++11 -mcmodel=medium
LDLIBS = -pthread -fopenmp

# Default -O1 if optimization level not defined
ifeq "$(findstring -O,$(CFLAGS))" ""
	CFLAGS += -O1
endif

ifeq ($(AVX512SUPPORT),false)
	AVX512FLAG = 
	DAVX512 = 
else 
	AVX512FLAG = -mavx512f -mavx512bw -mavx512vl -mavx512cd -mavx512dq
	DAVX512 = -DAVX512_SUPPORTED
endif

# Header files
HEADERS = \
	rbench.hpp \
	core-net.hpp 

#  Source of interferences(SOI) source file
SOI_SRC = \
	rbench.cpp \
	rbench-core.cpp \
	rbench-cpu-cache.cpp \
	rbench-cpu-int.cpp \
	rbench-cpu-float.cpp \
	rbench-cpu-tlb.cpp \
	rbench-cpu-l1i.cpp \
	rbench-mem-bw.cpp \
	rbench-udp.cpp \
	rbench-simd-avx.cpp \
	rbench-simd-avx512.cpp

# Source of interferences(SOI) core file
CORE_SRC = \
	core-mwc.cpp \
	core-lfsr.cpp \
	core-net.cpp 

SRC = $(CORE_SRC) $(SOI_SRC)
OBJS = $(SRC:.cpp=.o) rbench-cpu-l1i-kernel.o
ASMS = \
	rbench-cpu-l1i-kernel.asm
BINNAME = rbench.exe

all : rbench validate 

# dependencies micro : $<
# aim micro : $@
rbench-cpu-l1i-kernel.o : rbench-cpu-l1igen.cpp
	$(CC) $(CFLAGS) $< -o rbench-cpu-l1igen.exe
	./rbench-cpu-l1igen.exe > rbench-cpu-l1i-kernel.asm
	$(AS) -c rbench-cpu-l1i-kernel.asm -o $@ 

rbench-simd-avx.o : rbench-simd-avx.cpp $(HEADERS)
	$(CC) $(CFLAGS) -mavx -mavx2 -c rbench-simd-avx.cpp -o rbench-simd-avx.o

rbench-simd-avx512.o : rbench-simd-avx512.cpp $(HEADERS)
	$(CC) $(CFLAGS) -mavx -mavx2 $(AVX512FLAG) -c rbench-simd-avx512.cpp -o rbench-simd-avx512.o $(DAVX512)

%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

rbench: $(OBJS)
	$(LD) $(OBJS) -o ${BINNAME} $(LDLIBS)

validate: FORCE
	cd $(VALIDATEDIR) ; make

.PHONY: FORCE
FORCE: ;

.PHONY: clean
clean:
	rm -f ${DESTDIR}$(BINNAME)
	rm -f ${DESTDIR}$(OBJS)
	rm -f ${DESTDIR}$(ASMS)
	rm -f ${DESTDIR}*.exe
	cd $(VALIDATEDIR) ; make clean