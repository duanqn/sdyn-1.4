CC=gcc
CLIFLAG = 
ECFLAGS=-O3 -g $(CLIFLAG)
CFLAGS=-D_XOPEN_SOURCE=600 -I. $(ECFLAGS)
AR=ar
ARFLAGS=rc
RANLIB=ranlib

PATCH_DEST=../ggggc
PATCHES=

COLLECTOROBJ = collector-ms.o
#COLLECTOROBJ = collector-gembc.o
OBJS=allocate.o $(COLLECTOROBJ) globals.o roots.o threads.o \
     collections/list.o collections/map.o
TESTOBJS = collector-ms_test.o

all: libggggc.a

libggggc.a: $(OBJS)
	$(AR) $(ARFLAGS) libggggc.a $(OBJS)
	$(RANLIB) libggggc.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

push:
	$(CC) $(CFLAGS) pushgen.c -o pushgen
	./pushgen > ggggc/push.h
	rm -f pushgen

clean:
	rm -f $(OBJS) $(TESTOBJS) libggggc.a deps

test: collector-test collector-test2

collector-test: $(OBJS) collector-ms_test.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

collector-test2: $(OBJS) collector-ms_test2.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

patch:
	for i in *.c *.h collections/*.c ggggc/*.h ggggc/collections/*.h; \
	do \
	    if [ ! -e $(PATCH_DEST)/$$i -o $$i -nt $(PATCH_DEST)/$$i ]; \
	    then \
	        mkdir -p $(PATCH_DEST)/`dirname $$i`; \
	        cp $$i $(PATCH_DEST)/$$i; \
	        rm -f $(PATCH_DEST)/Makefile; \
	    fi; \
	done
	[ -e $(PATCH_DEST)/Makefile ] || \
	    for p in $(PATCHES); do ( cd patches/$$p; cat `cat series` ); done | \
	        ( cd $(PATCH_DEST); patch -p1 )
	cp Makefile $(PATCH_DEST)/Makefile

include deps

deps:
	-$(CC) -MM *.c > deps
