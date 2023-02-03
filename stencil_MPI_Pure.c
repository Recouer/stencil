
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TEST(text) {fprintf(stderr, "test %s\n", text);}

#ifndef STENCIL_SIZE_X
#define STENCIL_SIZE_X 25
#endif
#ifndef STENCIL_SIZE_Y
#define STENCIL_SIZE_Y 30
#endif

#ifndef STENCIL_MAX_STEPS
#define STENCIL_MAX_STEPS 10000
#endif

/** number of buffers for N-buffering; should be at least 2 */
#define STENCIL_NBUFFERS 2

/** conduction coeff used in computation */
static const double alpha = 0.02;

/** threshold for convergence */
static const double epsilon = 0.0001;

/** latest computed buffer */
static int current_buffer = 0;



struct Global_Options{
    int X_step;
    int X_ind;

    int Y_step;
    int Y_ind;

    int XY_plane;
    int id;
    int line_length;

    int stencil_max_steps;

    double * values;
    double * X_computation_buffer;
    double * Y_computation_buffer;
};

struct Global_Options *global_options;

#define get_rank(X, Y) ((X) * global_options->line_length + (Y))

static void stencil_display(int b, int x0, int x1, int y0, int y1);

static void stencil_finish(int proc_number) {
    MPI_Status status;

    if (proc_number <= 1) return;

    if (global_options->X_ind + global_options->Y_ind == 0) {

        double * buffer = (double *) malloc(STENCIL_NBUFFERS * global_options->X_step * global_options->Y_step * sizeof(double));

        for (int i = 1; i < global_options->line_length * global_options->line_length; ++i) {
            MPI_Recv(buffer, STENCIL_NBUFFERS * global_options->X_step * global_options->Y_step, MPI_DOUBLE, i, 100, MPI_COMM_WORLD, &status);

            for (int b = 0; b < STENCIL_NBUFFERS; ++b) {
                for (int j = 1; j < global_options->X_step - 1; ++j) {
                    for (int k = 1; k < global_options->Y_step - 1; ++k) {
//                        printf("%d %d %d %d\n", (k - 1 + (i / global_options->line_length) * (global_options->Y_step - 2)), (j - 1 + (i % global_options->line_length) * (global_options->X_step - 2)), k, j);
//                        printf("%d %d\n", (b) * (global_options->XY_plane) + (j - 1 + (i % global_options->line_length) * (global_options->X_step - 2)) * global_options->Y_step + (k - 1 + (i / global_options->line_length) * (global_options->Y_step - 2)), j * global_options->Y_step + k);
                        global_options->values[(b) * (global_options->XY_plane) + (j - 1 + (i % global_options->line_length) * (global_options->X_step - 2)) * STENCIL_SIZE_Y + (k - 1 + (i / global_options->line_length) * (global_options->Y_step - 2))] = buffer[b * (global_options->X_step * global_options->Y_step) + j * global_options->Y_step + k];
                    }
                }
            }
        }
    } else {
        MPI_Send(global_options->values, STENCIL_NBUFFERS * global_options->X_step * global_options->Y_step, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD);
    }


    MPI_Finalize();
}

/** init stencil global_options->values to 0, borders to non-zero */
static void stencil_init() {
    int b, x, y;

    global_options->X_ind = global_options->id / global_options->line_length;
    global_options->Y_ind = global_options->id % global_options->line_length;

    global_options->X_step = (STENCIL_SIZE_X / global_options->line_length) + 2;
    global_options->Y_step = (STENCIL_SIZE_Y / global_options->line_length) + 2;

    if (global_options->id == 0) {
        global_options->XY_plane = STENCIL_SIZE_X * STENCIL_SIZE_Y;
        global_options->values = (double *) malloc(global_options->XY_plane * STENCIL_NBUFFERS * sizeof(double));
        global_options->X_computation_buffer = (double *) malloc(global_options->X_step * sizeof(double));
        global_options->Y_computation_buffer = (double *) malloc(global_options->Y_step * sizeof(double));
    }
    else {
        global_options->XY_plane = global_options->X_step * global_options->Y_step;
        global_options->values = (double *) malloc(global_options->XY_plane * STENCIL_NBUFFERS * sizeof(double));
        global_options->X_computation_buffer = (double *) malloc(global_options->X_step * sizeof(double));
        global_options->Y_computation_buffer = (double *) malloc(global_options->Y_step * sizeof(double));
    }



    for (b = 0; b < STENCIL_NBUFFERS; b++) {
        for (x = 0; x < global_options->X_step; x++) {
            for (y = 0; y < global_options->Y_step; y++) {
                global_options->values[b * global_options->XY_plane + x * global_options->Y_step + y] = 0.0;
            }
        }

        if (global_options->id == 0) {

            for(x = 0; x < STENCIL_SIZE_X; x++)
            {
                global_options->values[b * global_options->XY_plane + x * STENCIL_SIZE_Y] = x;
                global_options->values[b * global_options->XY_plane + x * STENCIL_SIZE_Y + STENCIL_SIZE_Y - 1] = STENCIL_SIZE_X - x - 1;
            }
            for(y = 0; y < STENCIL_SIZE_Y; y++)
            {
                global_options->values[b * global_options->XY_plane + y] = y;
                global_options->values[b * global_options->XY_plane + (STENCIL_SIZE_X - 1) * STENCIL_SIZE_Y + y] = STENCIL_SIZE_Y - y - 1;
            }
        }

        else {
            if (global_options->X_ind == 0) {
                for (x = 1; x < global_options->X_step - 1; x++) {
                    global_options->values[b * global_options->XY_plane + x * global_options->Y_step + 1] = x + (global_options->Y_ind * (global_options->X_step - 2)) + 1;
                }
            }

            if (global_options->X_ind == (global_options->line_length - 1)) {
                for (x = 1; x < global_options->X_step - 1; x++) {
                    global_options->values[b * global_options->XY_plane + x * global_options->Y_step + global_options->Y_step - 2] = STENCIL_SIZE_X - (x + (global_options->Y_ind * (global_options->X_step - 2)) - 1);
                }
            }

            if (global_options->Y_ind == 0) {
                for (y = 1; y < global_options->Y_step - 1; y++) {
                    global_options->values[b * global_options->XY_plane + global_options->Y_step + y] = (y + (global_options->X_ind * (global_options->Y_step - 2)));
                }
            }

            if (global_options->Y_ind == (global_options->line_length - 1)) {
                for (y = 1; y < global_options->Y_step - 1; y++) {
                    global_options->values[b * global_options->XY_plane + (global_options->X_step - 2) * global_options->Y_step + y] = STENCIL_SIZE_Y - (y + (global_options->X_ind * (global_options->Y_step - 2)) - 1);
                }
            }
        }
    }
}

/** display a (part of) the stencil global_options->values */
static void stencil_display(int b, int x0, int x1, int y0, int y1) {
    int x, y;
    printf("buffer: %d\n", b);
    printf("X:%d Y:%d %d %d \n", global_options->X_ind, global_options->Y_ind, global_options->X_step, global_options->Y_step);

    for (y = y0; y < y1; y++) {
        for (x = x0; x < x1; x++) {
//            printf("%8.5g %d", global_options->values[b * (y1 * x1) + x * y1 + y], b * (y1 * x1) + x * y1 + y);
            printf("%8.3g ", global_options->values[b * (y1 * x1) + x * y1 + y]);
        }
        printf("\n");
    }
}

static void compute_one_step(int size_y,
                             int x_offset, int x_delim,
                             int y_offset, int y_delim,
                             int * convergence,
                             int prev_buffer, int next_buffer) {
    int x, y;

    for (x = 1 + x_offset; x < global_options->X_step - 1 - x_delim; x++) {
        for (y = 1 + y_offset; y < global_options->Y_step - 1 - y_delim; y++) {
            global_options->values[next_buffer * global_options->XY_plane + x * size_y + y] =
                    alpha * global_options->values[prev_buffer * global_options->XY_plane + (x - 1) * size_y + y] +
                    alpha * global_options->values[prev_buffer * global_options->XY_plane + (x + 1) * size_y + y] +
                    alpha * global_options->values[prev_buffer * global_options->XY_plane + x * size_y + y - 1] +
                    alpha * global_options->values[prev_buffer * global_options->XY_plane + x * size_y + y + 1] +
                    (1.0 - 4.0 * alpha) * global_options->values[prev_buffer * global_options->XY_plane + x * size_y + y];
            if (*convergence && (fabs(global_options->values[prev_buffer * global_options->XY_plane + x * size_y + y] -
                                     global_options->values[next_buffer * global_options->XY_plane + x * size_y + y]) > epsilon)) {
                *convergence = 0;
            }
        }
    }
}

static void send_halo(int prev_buffer, int next_buffer, int step) {

    MPI_Request X_plus_request, X_minus_request, Y_plus_request, Y_minus_request;

    if (global_options->id == 0) {

        global_options->Y_computation_buffer[0] = 0;
        for (int i = 1; i < global_options->X_step; ++i)
            global_options->X_computation_buffer[i] =
                    global_options->values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + (i) * STENCIL_SIZE_Y - 1 - (global_options->line_length - 1) * (global_options->X_step - 2)];
        MPI_Isend(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

        MPI_Recv(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 1; i < global_options->X_step; ++i)
            global_options->values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + (i) * STENCIL_SIZE_Y - ((global_options->line_length - 1) * (global_options->Y_step - 2))] = global_options->X_computation_buffer[i];

        MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);

        global_options->Y_computation_buffer[0] = 0;
        for (int i = 0; i < global_options->Y_step - 1; ++i) global_options->Y_computation_buffer[i + 1] = global_options->values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + ((global_options->X_step - 3) * STENCIL_SIZE_Y) + (i)];
        MPI_Isend(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);

        MPI_Recv(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 1; i < global_options->Y_step; ++i)
            global_options->values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + ((global_options->X_step - 2) * STENCIL_SIZE_Y) + (i - 1)] = global_options->Y_computation_buffer[i];

        MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);
    } else {
        if (global_options->X_ind == 0) {
            for (int i = 1; i <= global_options->X_step; ++i) global_options->X_computation_buffer[i - 1] = global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step - 2];
            MPI_Isend(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

            MPI_Recv(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 1; i <= global_options->X_step; ++i) global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step - 1] = global_options->X_computation_buffer[i - 1];

            MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);
        }
        else if (global_options->X_ind == global_options->line_length - 1) {
            for (int i = 0; i < global_options->X_step; ++i) global_options->X_computation_buffer[i] = global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step + 1];
            MPI_Isend(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind - 1, global_options->Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, &X_minus_request);

            MPI_Recv(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind - 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->X_step; ++i) global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step] = global_options->X_computation_buffer[i];

            MPI_Wait(&X_minus_request, MPI_STATUS_IGNORE);
        }
        else {
            for (int i = 1; i <= global_options->X_step; ++i) global_options->X_computation_buffer[i - 1] = global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step - 2];
            MPI_Isend(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

            for (int i = 0; i < global_options->X_step; ++i) global_options->X_computation_buffer[i] = global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step + 1];
            MPI_Isend(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind - 1, global_options->Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, &X_minus_request);

            MPI_Recv(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind - 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->X_step; ++i) global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step] = global_options->X_computation_buffer[i];

            MPI_Recv(global_options->X_computation_buffer, global_options->X_step, MPI_DOUBLE, get_rank(global_options->X_ind + 1, global_options->Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 1; i <= global_options->X_step; ++i) global_options->values[prev_buffer * global_options->X_step * global_options->Y_step + (i) * global_options->Y_step - 1] = global_options->X_computation_buffer[i - 1];

            MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);
            MPI_Wait(&X_minus_request, MPI_STATUS_IGNORE);
        }


        if (global_options->Y_ind == 0) {
            for (int i = 0; i < global_options->Y_step; ++i) global_options->Y_computation_buffer[i] = global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + ((global_options->X_step - 2) * global_options->Y_step) + (i)];
            MPI_Isend(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);


            MPI_Recv(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->Y_step; ++i) global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + ((global_options->X_step - 1) * global_options->Y_step) + (i)] = global_options->Y_computation_buffer[i];

            MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);
        }
        else if (global_options->Y_ind == global_options->line_length - 1) {
            for (int i = 0; i < global_options->Y_step; ++i) global_options->Y_computation_buffer[i] = global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + (i) + global_options->Y_step];
            MPI_Isend(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind - 1), 2 + (4 * step + 3), MPI_COMM_WORLD, &Y_minus_request);

            MPI_Recv(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind - 1), 2 + (4 * step + 2), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->Y_step; ++i) global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + (i)] = global_options->Y_computation_buffer[i];

            MPI_Wait(&Y_minus_request, MPI_STATUS_IGNORE);
        }
        else {
            for (int i = 0; i < global_options->Y_step; ++i) global_options->Y_computation_buffer[i] = global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + ((global_options->X_step - 2) * global_options->Y_step) + (i)];
            MPI_Isend(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);


            for (int i = 0; i < global_options->Y_step; ++i) global_options->Y_computation_buffer[i] = global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + (i) + global_options->Y_step];
            MPI_Isend(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind - 1), 2 + (4 * step + 3), MPI_COMM_WORLD, &Y_minus_request);

            MPI_Recv(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind - 1), 2 + (4 * step + 2), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->Y_step; ++i) global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + (i)] = global_options->Y_computation_buffer[i];

            MPI_Recv(global_options->Y_computation_buffer, global_options->Y_step, MPI_DOUBLE, get_rank(global_options->X_ind, global_options->Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < global_options->Y_step; ++i) global_options->values[prev_buffer * (global_options->X_step * global_options->Y_step) + ((global_options->X_step - 1) * global_options->Y_step) + (i)] = global_options->Y_computation_buffer[i];

            MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);
            MPI_Wait(&Y_minus_request, MPI_STATUS_IGNORE);
        }
    }
}

/** compute the next stencil step, return 1 if computation has converged */
static int stencil_step(int step) {
    int convergence = 1;
    int prev_buffer = current_buffer;
    int next_buffer = (current_buffer + 1) % STENCIL_NBUFFERS;

    int x_offset = 0, y_offset = 0;
    int x_delim = 0, y_delim = 0;
    int size = STENCIL_SIZE_Y;

    send_halo(prev_buffer, next_buffer, step);

    if (global_options->id != 0) {
        x_offset = (global_options->Y_ind == 0), y_offset = (global_options->X_ind == 0);
        x_delim = (global_options->Y_ind == global_options->line_length - 1), y_delim = (global_options->X_ind == global_options->line_length - 1);
        size = global_options->Y_step;
    }

    compute_one_step(size, x_offset, x_delim, y_offset, y_delim, &convergence, prev_buffer, next_buffer);

    current_buffer = next_buffer;

    MPI_Allreduce(&convergence, &convergence, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

    return convergence;
}

int main(int argc, char *argv[]) {
    bool printHeader = false;
    bool printColor = false;
    bool printStencilDisplay = false;
    FILE *dataStd = stdout;

    int opt;
    while ((opt = getopt(argc, argv, "hpdc")) != -1) {
        switch (opt) {
            case 'h':
                printHeader = true;
                break;
            case 'p':
                printStencilDisplay = true;
                break;
            case 'd':
                dataStd = stderr;
                break;
            case 'c':
                printColor = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-hpdc] \n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int N;

    global_options = (struct Global_Options *) malloc(sizeof(struct Global_Options));


    MPI_Init(&argc, &argv);
    global_options->stencil_max_steps = STENCIL_MAX_STEPS;


    MPI_Comm_size(MPI_COMM_WORLD, &N);
    MPI_Comm_rank(MPI_COMM_WORLD, &global_options->id);

    global_options->line_length = (int) sqrt(N);


    stencil_init();

//    if (global_options->id == -1) {
//        printf("# init:\n");
//        stencil_display(current_buffer, 0, STENCIL_SIZE_X, 0, STENCIL_SIZE_Y);
//    }


    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    int s;

    for (s = 0; s < global_options->stencil_max_steps; s++) {
        int convergence = stencil_step(s);
        if (convergence) {
            break;
        }
    }


    stencil_finish(N);


    if (global_options->id == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t2);
        const double t_usec =
            (t2.tv_sec - t1.tv_sec) * 1E6 + (t2.tv_nsec - t1.tv_nsec) / 1E3;
        const long nbCells = (STENCIL_SIZE_X - 2) * (STENCIL_SIZE_Y - 2);
        const long nbOperationsByStep = 10 * nbCells;
        const double gigaflops = nbOperationsByStep * s * 1E6 / t_usec / 1E9;
        const double nbCellsByS = nbCells * s * 1E6 / t_usec;

        if (printHeader)
            printf(
                "steps,timeInµSec,height,width,nbCells,fpOpByStep,gigaflops,"
                "cellByS\n");

        if (printColor)
            fprintf(dataStd, "%d,%g,%d,%d,%ld,%ld,%g,\033[0;32m%g\033[0m\n", s,
                    t_usec, STENCIL_SIZE_X, STENCIL_SIZE_Y, nbCells,
                    nbOperationsByStep, gigaflops, nbCellsByS);
        else
            fprintf(dataStd, "%d,%g,%d,%d,%ld,%ld,%g,%g\n", s, t_usec,
                    STENCIL_SIZE_X, STENCIL_SIZE_Y, nbCells, nbOperationsByStep,
                    gigaflops, nbCellsByS);

        if (printStencilDisplay) {
            stencil_display(current_buffer, 0, STENCIL_SIZE_X, 0,
                            STENCIL_SIZE_Y);
            stencil_display((current_buffer + 1) % 2, 0, STENCIL_SIZE_X, 0,
                            STENCIL_SIZE_Y);
        }
    }

    return 0;
}
