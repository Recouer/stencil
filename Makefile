STENCIL_SIZE_X ?= 1000
STENCIL_SIZE_Y ?= 1000
STENCIL_MAX_STEPS ?= 100
TILE_WIDTH ?= 6
TILE_HEIGHT ?= 6

CC      = mpicc
# CC      = gcc
CFLAGS += -DSTENCIL_SIZE_X=$(STENCIL_SIZE_X) -DSTENCIL_SIZE_Y=$(STENCIL_SIZE_Y)
CFLAGS += -DSTENCIL_MAX_STEPS=$(STENCIL_MAX_STEPS)
CFLAGS += -DTILE_WIDTH=$(TILE_WIDTH) -DTILE_HEIGHT=$(TILE_HEIGHT)
CFLAGS += -Wall -g -O4
CFLAGS += -fopenmp
LDLIBS += -lm -lrt

all: stencil_seq stencil_MPI_Pure stencil_OMP_for stencil_MPI_omp

clean:
	-rm -f stencil stencil_seq stencil_MPI_Pure stencil_OMP_for stencil_MPI_omp

diff: all
	./stencil_seq -dpc > stencil_seq.out
	OMP_NUM_THREADS=2 ./stencil_OMP_for -dpc > stencil_OMP_for.out
	mpirun -np 4 ./stencil_MPI_Pure -dpc > stencil_MPI_Pure.out
	OMP_NUM_THREADS=2 mpirun -np 4 ./stencil_MPI_omp -dpc > stencil_MPI_omp.out
	# diff stencil_seq.out stencil_OMP_for.out
	# diff stencil_seq.out stencil_MPI_Pure.out
	# diff stencil_seq.out stencil_MPI_omp.out

mrproper: clean
	-rm *~

archive: stencil.c Makefile
	( cd .. ; tar czf stencil.tar.gz stencil/ )

