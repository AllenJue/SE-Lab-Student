# Modified S2022 by Zach Leeper
# Removed seq testing, since we provide the complete implementation for it.

CC=gcc
CFLAGS=-Wall -O2

ISADIR = ../misc
YAS=$(ISADIR)/yas
YIS=$(ISADIR)/yis
PIPE=../pipe/psim
SEQ=../seq/ssim

YOFILES = prog1.yo prog2.yo prog3.yo prog4.yo prog5.yo prog6.yo prog7.yo prog8.yo prog9.yo myprog.yo prog1-.yo prog2-.yo prog3-.yo prog4-.yo prog5-.yo prog6-.yo

PIPEFILES = prog1.pipe prog2.pipe prog3.pipe prog4.pipe prog5.pipe prog6.pipe prog7.pipe prog8.pipe prog9.pipe

PIPE-FILES = prog1.pipe- prog2.pipe- prog3.pipe- prog4.pipe- prog5.pipe- prog6.pipe-

SEQFILES = prog1.seq prog2.seq prog3.seq prog4.seq prog5.seq prog6.seq prog7.seq prog8.seq


.SUFFIXES:
.SUFFIXES: .c .s .o .ys .yo .yis .pipe .seq .pipe-

all: $(YOFILES) 

test: testpsim

testpsim-: $(PIPE-FILES)
	grep "ISA Check" *.pipe-
	rm $(PIPE-FILES)

testpsim: $(PIPEFILES)
	grep "ISA Check" *.pipe
	rm $(PIPEFILES)

testssim: $(SEQFILES)
	grep "ISA Check" *.seq
	rm $(SEQFILES)

.ys.yo:
	$(YAS) $*.ys

.yo.yis: $(YIS)
	$(YIS) $*.yo > $*.yis

.yo.pipe: $(PIPE)
	$(PIPE) $*.yo > $*.pipe

.yo.pipe-: $(PIPE)
	$(PIPE) $*-.yo > $*.pipe-

clean:
	rm -f *.o *.yis *~ *.yo *.pipe *.seq *.pipe- core
