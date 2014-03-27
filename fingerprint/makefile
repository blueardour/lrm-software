

CC=     gcc
CXX=    g++
CFLAGS=   -g -Wall -O2
CXXFLAGS= $(CFLAGS)
AR=   ar

LIBS= utils.o fingerprint.o
TARGET= aln sort index printfinger

all: $(LIBS) $(TARGET)

aln: aln.o
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

sort: sort.o
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

index: index.o
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)

printfinger: printfinger.o
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS)


.SUFFIXES:.c .o .cc

.c.o:
	    $(CC) -c $(CFLAGS) $< -o $@
.cc.o:
	    $(CXX) -c $(CXXFLAGS) $< -o $@


clean:
	@rm *.o
