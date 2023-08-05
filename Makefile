VERSION=0.0.1

ROOTDIR = $(PWD)
VALIDATEDIR  = $(ROOTDIR)/validate

KERNEL=$(shell uname -s)
NODENAME=$(shell uname -n)

CFLAGS = -Wall -Wextra -Wconversion -DVERSION='"$(VERSION)"' -std=gnu++11
LD = g++
LDLIBS = -pthread
CC = g++

# Default -O2 if optimization level not defined
ifeq "$(findstring -O,$(CFLAGS))" ""
	CFLAGS += -O1
endif

# Header files
HEADERS = \
	rbench.hpp

#  Source of interferences(SOI) source file
SOI_SRC = \
	rbench.cpp \
	rbench-core.cpp \
	rbench-cpu-cache.cpp \
	rbench-cpu-int.cpp \
	rbench-cpu-float.cpp \
	rbench-cpu-tlb.cpp 

# Source of interferences(SOI) core file
CORE_SRC = \
	core-mwc.cpp \
	core-lfsr.cpp 

SRC = $(CORE_SRC) $(SOI_SRC)
OBJS += $(SRC:.cpp=.o)
BINNAME = rbench.exe

all : rbench validate 

# dependencies micro : $<
# aim micro : $@
%.o : %.cpp
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
	rm -f ${DESTDIR}*.exe
	cd $(VALIDATEDIR) ; make clean