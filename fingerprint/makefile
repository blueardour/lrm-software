

CC=     gcc
CXX=    g++
CFLAGS=   -g -Wall -O2
CXXFLAGS= $(CFLAGS)
AR=     ar
LIBS= utils.o

all: fingerprint lookup index

.SUFFIXES:.c .o .cc

.c.o:
	    $(CC) -c $(CFLAGS) $< -o $@
.cc.o:
	    $(CXX) -c $(CXXFLAGS) $< -o $@

fingerprint: fingerprint.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

lookup: lookup.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

index: index.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

dump: dump.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

utils.o: utils.h

clean:
	rm *.o 
