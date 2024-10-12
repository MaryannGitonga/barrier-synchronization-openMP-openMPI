MPICC = mpicc.mpich

OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CFLAGS = -g -std=gnu99 -I. -Wall $(OMPFLAGS)
LDLIBS = $(OMPLIBS) -lm

all: combined1 combined2 combined3

SRC1 = combined1.c
SRC2 = combined2.c
SRC3 = combined3.c

combined1: combined1.c harness.o
	$(MPICC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

combined2: combined2.c harness.o 
	$(MPICC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

combined3: combined3.c harness.o
	$(MPICC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

%.o: %.c
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.dSYM combined1 combined2 combined3