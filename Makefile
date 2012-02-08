# compile for  xiami.com client
# Copyright 2011 SNDA
#


#PLATFORM=arm

ifeq ($(PLATFORM), arm)
CC = /opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc
LD = /opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ld
LDFLAG = -L./arm -lasound
CFLAGS += -I./arm
else
CXX = g++
CC = gcc
LD = ld
LDFLAG = -lasound
endif

PROG = fmradio

SRCS = fmradio.c qndriver.c  qnio.c

OBJS = fmradio.o qndriver.o  qnio.o

CFLAGS += -Wall -O2

all: $(PROG)

$(PROG): $(OBJS)
	$(CC)  $(CFLAGS)  -o $@ $^ $(LDFLAG)

install:
	install  $(PROG) /bin/$(PROG) 
clean:
	rm -f $(PROG) $(OBJS) 

.PHONY: all install clean

