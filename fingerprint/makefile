

CC=     gcc
CXX=    g++
CFLAGS=   -g -Wall -O2
CXXFLAGS= $(CFLAGS)
AR=     ar
LIBS= utils.o fingerprint.o

all: index sort

.SUFFIXES:.c .o .cc

.c.o:
	    $(CC) -c $(CFLAGS) $< -o $@
.cc.o:
	    $(CXX) -c $(CXXFLAGS) $< -o $@

aln: aln.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

index: index.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

sort: sort.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

dump: dump.o $(LIBS)
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

utils.o: utils.h

fingerprint.o: fingerprint.h

clean:
	rm *.o 
