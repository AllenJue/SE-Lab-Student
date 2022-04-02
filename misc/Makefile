CC=gcc
CFLAGS= -Wall -Werror -O0 -ggdb
YAS=./yas

all: yis

# These are implicit rules for making .yo files from .ys files.
# E.g., make sum.yo
.SUFFIXES: .ys .yo
.ys.yo:
	$(YAS) $*.ys

isa.o: isa.c isa.h
	$(CC) $(CFLAGS) -c isa.c

yis.o: yis.c isa.h
	$(CC) $(CFLAGS) -c yis.c

yis: yis.o isa.o
	$(CC) $(CFLAGS) yis.o isa.o -o yis

clean:
	rm -f *.o *.yo *.exe yis


