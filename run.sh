#!/bin/bash

# Path vars
SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")
SCRIPT_NAME=$(basename -s .sh -- "$SCRIPT_PATH")

# Global vars
CSV_DIR=$SCRIPT_DIR/csv
ITER=20

# disable the turbo boost
# echo "1" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

make -j

# $ITER
CSV_FILENAME=seq.csv
echo "steps,time_µ_sec,height,width,nbCells,fpOpByStep,gigaflop_s,cell_s" >> $CSV_DIR/$CSV_FILENAME

./stencil_seq >> $CSV_DIR/$CSV_FILENAME

CSV_FILENAME=mpi_pure.csv
echo "steps,time_µ_sec,height,width,nbCells,fpOpByStep,gigaflop_s,cell_s" >> $CSV_DIR/$CSV_FILENAME

./stencil_MPI_Pure >> $CSV_DIR/$CSV_FILENAME
