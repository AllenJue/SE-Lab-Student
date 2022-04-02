SIM=../pipe/psim

ISADIR = ../misc
YAS=$(ISADIR)/yas

.SUFFIXES: .ys .yo

.ys.yo:
	$(YAS) $*.ys

test:
	./optest.pl -s $(SIM)
	./jtest.pl -s $(SIM)
	./ctest.pl -s $(SIM)
	./htest.pl -s $(SIM)
	./mtest.pl -s $(SIM)

test-pipe-cache:
	./mtest.pl -c -s ../pipe-cache/pcsim

cache-bonus:
	./btest.pl -c -s ../pipe-cache/pcsim

op-bonus: 
	./boptest.pl -s ../pipe/psim

clean:
	rm -f *.o *~ *.yo *.ys

