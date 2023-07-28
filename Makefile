VERSION=0.0.1

KERNEL=$(shell uname -s)
NODENAME=$(shell uname -n)

CFLAGS = -Wall -Wextra -Wconversion -DVERSION='"$(VERSION)"' -std=gnu++11
LD = g++
LDLIBS = -pthread
CC = g++

# Default -O2 if optimization level not defined
ifeq "$(findstring -O,$(CFLAGS))" ""
	CFLAGS += -O2
endif

# Header files
HEADERS = \
	rbench-core.hpp \
	rbench.hpp

#  Source of interferences(SOI) source file
SOI_SRC = \
	rbench.cpp \
	rbench-core.cpp

# Source of interferences(SOI) core file
CORE_SRC = \
	core-mwc.cpp \
	core-lfsr.cpp 

SRC = $(CORE_SRC) $(SOI_SRC)
OBJS += $(SRC:.cpp=.o)
BINNAME = rbench.exe

all : rbench

# dependencies micro : $<
# aim micro : $@
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

rbench: $(OBJS)
	$(LD) $(OBJS) -o ${BINNAME} $(LDLIBS)

.PHONY: clean
clean:
	rm -f ${DESTDIR}$(BINNAME)
	rm -f ${DESTDIR}$(OBJS)
	rm -f ${DESTDIR}*.exe