# Modify these two lines to choose your compiler and compile time
# flags.

CC=gcc
CFLAGS= -Wall -Werror -O0 -ggdb
MISCDIR=../misc
CACHEDIR=../cache
INC= -I$(MISCDIR) -I$(CACHEDIR)
##################################################
# You shouldn't need to modify anything below here
##################################################

LIBS= -lm
YAS = ../misc/yas

all: pcsim

# This rule builds the PIPE simulator
pcsim: $(CACHEDIR)/cache.c $(CACHEDIR)/cache.h pcsim.c $(MISCDIR)/isa.c
	$(CC) $(CFLAGS) -DCACHE_ENABLED $(INC) -o pcsim pcsim.c $(MISCDIR)/isa.c $(CACHEDIR)/cache.c $(LIBS)

# These are implicit rules for assembling .yo files from .ys files.
.SUFFIXES: .ys .yo
.ys.yo:
	$(YAS) $*.ys


clean:
	rm -f pcsim *.o *.exe *~ 


