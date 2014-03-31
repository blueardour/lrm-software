

CC=     gcc
CXX=    g++
CFLAGS=   -g -Wall -O2
CXXFLAGS= $(CFLAGS)
AR=   ar

LIBS= utils.o fingerprint.o
TARGET= aln sort index printfinger

all: $(LIBS) $(TARGET)

aln: aln.o  fingerprint.h
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

sort: sort.o  fingerprint.h
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

index: index.o  fingerprint.h
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

printfinger: printfinger.o  fingerprint.h
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)


.SUFFIXES:.c .o .cc

.c.o:	fingerprint.h
	    $(CC) -c $(CFLAGS) $< -o $@
.cc.o:  fingerprint.h
	    $(CXX) -c $(CXXFLAGS) $< -o $@


clean:
	@rm *.o
