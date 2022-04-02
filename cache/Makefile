# Modify these two lines to choose your compiler and compile time
# flags.

CC=gcc
CFLAGS= -Wall -Werror -O0 -ggdb
MISCDIR=../misc
INC= -I$(MISCDIR) 

##################################################
# You shouldn't need to modify anything below here
##################################################

LIBS= -lm

all: csim test-cache 

csim: csim.c cache.c cache.h 
	$(CC) $(CFLAGS) $(INC) -o csim csim.c cache.c -lm

test-cache: csim test-csim.c
	$(CC) $(CFLAGS) -o test-csim test-csim.c

clean:
	rm -f test-csim csim *.o *.exe *~ 


