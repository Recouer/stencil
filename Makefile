STENCIL_SIZE_X ?= 24
STENCIL_SIZE_Y ?= 30
TILE_WIDTH ?= 6
TILE_HEIGHT ?= 6

CC      = mpicc
# CC      = gcc
CFLAGS += -DSTENCIL_SIZE_X=$(STENCIL_SIZE_X) -DSTENCIL_SIZE_Y=$(STENCIL_SIZE_Y)
CFLAGS += -DTILE_WIDTH=$(TILE_WIDTH) -DTILE_HEIGHT=$(TILE_HEIGHT)
CFLAGS += -Wall -g -O4
CFLAGS += -fopenmp
LDLIBS += -lm -lrt

all: stencil_seq stencil_MPI_Pure stencil_OMP_for stencil_MPI_omp

clean:
	-rm -f stencil stencil_seq stencil_MPI_Pure stencil_OMP_for stencil_MPI_omp

diff: all
	./stencil_seq -dc > stencil_seq.out
	./stencil_OMP_for -dc > stencil_OMP_for.out
	mpirun -np 4 ./stencil_MPI_Pure -dc > stencil_MPI_Pure.out
	mpirun -np 4 ./stencil_MPI_omp -dc > stencil_MPI_omp.out
	diff stencil_seq.out stencil_OMP_for.out
	diff stencil_seq.out stencil_MPI_Pure.out
	diff stencil_seq.out stencil_MPI_omp.out

mrproper: clean
	-rm *~

archive: stencil.c Makefile
	( cd .. ; tar czf stencil.tar.gz stencil/ )

