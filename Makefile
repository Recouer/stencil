STENCIL_SIZE_X ?= 25
STENCIL_SIZE_Y ?= 30
TILE_WIDTH ?= 6
TILE_HEIGHT ?= 6

CC      = mpicc
# CC      = gcc
CFLAGS += -DSTENCIL_SIZE_X=$(STENCIL_SIZE_X) -DSTENCIL_SIZE_Y=$(STENCIL_SIZE_Y)
CFLAGS += -DTILE_WIDTH=$(TILE_WIDTH) -DTILE_HEIGHT=$(TILE_HEIGHT)
CFLAGS += -Wall -g -O4
# CFLAGS += -fopenmp
LDLIBS += -lm -lrt

all: stencil stencil_seq stencil_MPI_Pure

clean:
	-rm -f stencil stencil_seq stencil_MPI_Pure

diff: all
	./stencil -dc > stencil_mpi.out
	./stencil_seq -dc > stencil_seq.out
	diff stencil_seq.out stencil_mpi.out

mrproper: clean
	-rm *~

archive: stencil.c Makefile
	( cd .. ; tar czf stencil.tar.gz stencil/ )

