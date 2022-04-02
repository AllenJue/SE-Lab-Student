# Modify these two lines to choose your compiler and compile time
# flags.

CC=gcc
CFLAGS= -Wall -Werror -O0 -ggdb

##################################################
# You shouldn't need to modify anything below here
##################################################

MISCDIR=../misc
INC=-I$(MISCDIR) 
LIBS=-lm
YAS=../misc/yas

all: ssim

# This rule builds the SEQ simulator (ssim)
ssim: ssim.c sim.h $(MISCDIR)/isa.c $(MISCDIR)/isa.h
	$(CC) $(CFLAGS) $(INC) -o ssim ssim.c $(MISCDIR)/isa.c $(LIBS)

# These are implicit rules for assembling .yo files from .ys files.
.SUFFIXES: .ys .yo
.ys.yo:
	$(YAS) $*.ys

clean:
	rm -f ssim *.o *~ *.exe *.yo *.ys
