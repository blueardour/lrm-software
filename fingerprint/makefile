

CC=     gcc
CXX=    g++
CFLAGS=   -g -Wall -O2
CXXFLAGS= $(CFLAGS)
AR=   ar

LIBS= utils.o fingerprint.o hash.o
OBJS = aln.o sort.o index.o $(LIBS)
TARGET= aln sort index printfinger

all: $(TARGET)

$(TARGET): %: %.o $(LIBS)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

-include $(OBJS:.o=.d)

%.d:%.c
	@set -e; \
	rm -f $@; \
	$(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@-rm *.o
