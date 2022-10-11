CFLAGS=-Wall -fPIC -I. -D_UNIX -D_LINUX -DHAVE_TIMEVAL -DPROVIDE_LOG_UTILITIES -g3 
LDFLAGS=-g3 -fPIC -L/home/mhobbis/solclient-7.22.0.11/lib -lsolclient 

CC = gcc
CPP = g++
LD = g++

LIBS = libsolacetcexit.so

all:libsolacetcexit.so

libsolacetcexit.so:solacetcexit.o tcsutils.o
	$(LD) -fPIC -shared $(LDFLAGS) -o $@ solacetcexit.o tcsutils.o 

%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o:%.cpp
	$(CPP) -c $(CFLAGS) -o $@ $<
	
%.o:%.cc
	$(CPP) -c $(CFLAGS) -o $@ $<
	
%.d:%.c
	/bin/sh -ec '$(CC) -M $(CFLAGS) $< | sed '\"s/$*.o/& $@/g'\" > $@'

deps: solacetcexit.d
	@echo Ready.

clean:
	rm -f core *.so *.o
