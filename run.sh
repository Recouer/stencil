#!/bin/bash

# Path vars
SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")
SCRIPT_NAME=$(basename -s .sh -- "$SCRIPT_PATH")

# Global vars
CSV_DIR=$SCRIPT_DIR/csv
ITER=20
# SIZEs=(25,30 50,60 75,90 100,120 125,150 150,180 175,210 200,240 225,270 250,300 275,330 300,360 325,390 350,420 375,450 400,480 425,510 450,540 475,570 500,600 625,750 750,900 1000,1200)
SIZEs=(10,10 20,20 30,30 40,40 50,50 60,60 70,70 80,80 90,90 100,100 200,200 400,400 800,800 1000,1000 1500,1500 2000,2000)
BLOCS_SIZEs=(8 16 32 64 128 512)
STENCIL_MAX_STEPS=200

# disable the turbo boost
# echo "1" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

make -j

module load compiler/gcc/10.3.0 mpi/openmpi/3.1.4-all;

CSV_FILENAME=seq.csv
echo "steps,timeInµSec,height,width,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME

IFS=',';
for i in "${SIZEs[@]}";
do
    set -- $i
    make clean -s
  STENCIL_SIZE_X=$1 STENCIL_SIZE_Y=$2 STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS make stencil_seq || exit 1

  for (( i=1; i <= $ITER; i++ )); do
    salloc -proutage --exclusive -N1 -n1 -n1 ./stencil_seq >> $CSV_DIR/$CSV_FILENAME
  done
done

CSV_FILENAME=omp.csv
echo "steps,timeInµSec,height,width,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME

IFS=',';
for i in "${SIZEs[@]}";
do
    set -- $i
    make clean -s
  STENCIL_SIZE_X=$1 STENCIL_SIZE_Y=$2 STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS make stencil_OMP_for || exit 1

  for (( i=1; i <= $ITER; i++ )); do
    salloc -proutage --exclusive -N1 -n1 -n4 /bin/env OMP_NUM_THREADS=4 OMP_SCHEDULE=static ./stencil_OMP_for >> $CSV_DIR/$CSV_FILENAME
  done
done

CSV_FILENAME=omp_halos.csv
echo "steps,timeInµSec,height,width,tiledW,tiledH,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME

IFS=',';
for i in "${SIZEs[@]}";
do
    set -- $i
    make clean -s
    STENCIL_SIZE_X=$1 STENCIL_SIZE_Y=$2 STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS TILE_WIDTH=10 TILE_HEIGHT=10 make stencil_OMP_for_halos || exit 1
    for (( i=1; i <= $ITER; i++ )); do
      salloc -proutage --exclusive -N1 -n1 -n4 /bin/env OMP_NUM_THREADS=4 OMP_SCHEDULE=static ./stencil_OMP_for_halos >> $CSV_DIR/$CSV_FILENAME
    done
done

CSV_FILENAME=mpiPure.csv
echo "steps,timeInµSec,height,width,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME

IFS=',';
for i in "${SIZEs[@]}";
do
    set -- $i
    make clean -s
    STENCIL_SIZE_X=$1 STENCIL_SIZE_Y=$2 STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS make stencil_MPI_Pure || exit 1
    for (( i=1; i <= $ITER; i++ )); do
      salloc -proutage --exclusive -N4 -n4 mpirun --map-by ppr:4:node ./stencil_MPI_Pure >> $CSV_DIR/$CSV_FILENAME
    done
done

CSV_FILENAME=mpiOmp.csv
echo "steps,timeInµSec,height,width,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME

IFS=',';
for i in "${SIZEs[@]}";
do
    set -- $i
    make clean -s
    STENCIL_SIZE_X=$1 STENCIL_SIZE_Y=$2 STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS make stencil_MPI_omp || exit 1
    for (( i=1; i <= $ITER; i++ )); do
      salloc -proutage --exclusive -N4 -n4 -c4 mpirun --map-by ppr:4:node ./stencil_MPI_omp >> $CSV_DIR/$CSV_FILENAME
    done
done

# CSV_FILENAME=mpi.csv
# echo "steps,timeInµSec,height,width,tiledW,tiledH,nbCells,fpOpByStep,gigaflops,cellByS" > $CSV_DIR/$CSV_FILENAME
# for TW in "${BLOCS_SIZEs[@]}";
# do
#   for TH in "${BLOCS_SIZEs[@]}";
#   do
#     make clean -s
#     TILE_WIDTH=$TW TILE_HEIGHT=$TH STENCIL_MAX_STEPS=$STENCIL_MAX_STEPS make stencil && mpirun ./stencil >> $CSV_DIR/$CSV_FILENAME
#   done
# done
