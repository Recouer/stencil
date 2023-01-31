
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef STENCIL_SIZE_X
#define STENCIL_SIZE_X 25
#endif
#ifndef STENCIL_SIZE_Y
#define STENCIL_SIZE_Y 30
#endif

/** number of buffers for N-buffering; should be at least 2 */
#define STENCIL_NBUFFERS 2

/** conduction coeff used in computation */
static const double alpha = 0.02;

/** threshold for convergence */
static const double epsilon = 0.0001;

/** max number of steps */
static const int stencil_max_steps = 5000000;

double * values;
double * X_computation_buffer;
double * Y_computation_buffer;

/** latest computed buffer */
static int current_buffer = 0;
int X_step, Y_step, X_ind, Y_ind, XY_plane, id, line_length;

#define get_rank(X, Y) ((X) * line_length + (Y))

static void stencil_display(int b, int x0, int x1, int y0, int y1);

static void stencil_finish(int N, int line_length) {
    MPI_Status status;

    if (X_ind + Y_ind == 0) {

        double * buffer = (double *) malloc(X_step * Y_step * sizeof(double));

        for (int i = 1; i < N; ++i) {
            MPI_Recv(buffer, STENCIL_NBUFFERS * X_step * Y_step, MPI_DOUBLE, i, 100, MPI_COMM_WORLD, &status);

            for (int b = 0; b < STENCIL_NBUFFERS; ++b) {
                for (int j = 1; j < X_step - 1; ++j) {
                    for (int k = 1; k < Y_step - 1; ++k) {
//                        printf("%d %d %d %d\n", (k - 1 + (i / line_length) * (Y_step - 2)), (j - 1 + (i % line_length) * (X_step - 2)), k, j);
//                        printf("%d %d\n", (b) * (XY_plane) + (j - 1 + (i % line_length) * (X_step - 2)) * Y_step + (k - 1 + (i / line_length) * (Y_step - 2)), j * Y_step + k);
                        values[(b) * (XY_plane) + (j - 1 + (i % line_length) * (X_step - 2)) * STENCIL_SIZE_Y + (k - 1 + (i / line_length) * (Y_step - 2))] = buffer[b * (X_step * Y_step) + j * Y_step + k];
                    }
                }
            }
        }
    } else {
        MPI_Send(values, STENCIL_NBUFFERS * X_step * Y_step, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD);
    }


    MPI_Finalize();
}

/** init stencil values to 0, borders to non-zero */
static void stencil_init(int line_length, int id) {
    int b, x, y;

    X_ind = id / line_length;
    Y_ind = id % line_length;

    X_step = (STENCIL_SIZE_X / line_length) + 2;
    Y_step = (STENCIL_SIZE_Y / line_length) + 2;

    if (id == 0) {
        XY_plane = STENCIL_SIZE_X * STENCIL_SIZE_Y;
        values = (double *) malloc(XY_plane * STENCIL_NBUFFERS * sizeof(double));
        X_computation_buffer = (double *) malloc(X_step * sizeof(double));
        Y_computation_buffer = (double *) malloc(Y_step * sizeof(double));
    }
    else {
        XY_plane = X_step * Y_step;
        values = (double *) malloc(XY_plane * STENCIL_NBUFFERS * sizeof(double));
        X_computation_buffer = (double *) malloc(X_step * sizeof(double));
        Y_computation_buffer = (double *) malloc(Y_step * sizeof(double));
    }



    for (b = 0; b < STENCIL_NBUFFERS; b++) {
        for (x = 0; x < X_step; x++) {
            for (y = 0; y < Y_step; y++) {
                values[b * XY_plane + x * Y_step + y] = 0.0;
            }
        }

        if (id == 0) {

            for(x = 0; x < STENCIL_SIZE_X; x++)
            {
                values[b * XY_plane + x * STENCIL_SIZE_Y] = x;
                values[b * XY_plane + x * STENCIL_SIZE_Y + STENCIL_SIZE_Y - 1] = STENCIL_SIZE_X - x - 1;
            }
            for(y = 0; y < STENCIL_SIZE_Y; y++)
            {
                values[b * XY_plane + y] = y;
                values[b * XY_plane + (STENCIL_SIZE_X - 1) * STENCIL_SIZE_Y + y] = STENCIL_SIZE_Y - y - 1;
            }
        }

        else {
            if (X_ind == 0) {
                for (x = 1; x < X_step - 1; x++) {
                    values[b * XY_plane + x * Y_step + 1] = x + (Y_ind * (X_step - 2)) + 1 + 100;
                }
            }

            if (X_ind == (line_length - 1)) {
                for (x = 1; x < X_step - 1; x++) {
                    values[b * XY_plane + x * Y_step + Y_step - 2] = STENCIL_SIZE_X - (x + (Y_ind * (X_step - 2)) - 1) + 200;
                }
            }

            if (Y_ind == 0) {
                for (y = 1; y < Y_step - 1; y++) {
                    values[b * XY_plane + Y_step + y] = (y + (X_ind * (Y_step - 2))) + 300;
                }
            }

            if (Y_ind == (line_length - 1)) {
                for (y = 1; y < Y_step - 1; y++) {
                    values[b * XY_plane + (X_step - 2) * Y_step + y] = STENCIL_SIZE_Y - (y + (X_ind * (Y_step - 2)) - 1) + 400;
                }
            }
        }
    }
}

/** display a (part of) the stencil values */
static void stencil_display(int b, int x0, int x1, int y0, int y1) {
    int x, y;
    printf("buffer: %d\n", b);
    printf("X:%d Y:%d %d %d \n", X_ind, Y_ind, X_step, Y_step);

    for (y = y0; y < y1; y++) {
        for (x = x0; x < x1; x++) {
//            printf("%8.5g %d", values[b * (y1 * x1) + x * y1 + y], b * (y1 * x1) + x * y1 + y);
            printf("%8.3g ", values[b * (y1 * x1) + x * y1 + y]);
        }
        printf("\n");
    }
}

/** compute the next stencil step, return 1 if computation has converged */
static int stencil_step(int step) {
    int convergence = 1;
    int prev_buffer = current_buffer;
    int next_buffer = (current_buffer + 1) % STENCIL_NBUFFERS;
    int x, y;
    MPI_Request X_plus_request, X_minus_request, Y_plus_request, Y_minus_request;

    if (id == 0) {
        Y_computation_buffer[0] = 0;
        for (int i = 1; i < X_step; ++i) X_computation_buffer[i] = values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + (i) * STENCIL_SIZE_Y - 1 - (line_length - 1) * (X_step - 2)];
        MPI_Isend(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

        MPI_Recv(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 1; i < X_step; ++i)
            values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + (i) * STENCIL_SIZE_Y - ((line_length - 1) * (Y_step - 2))] = X_computation_buffer[i];

        MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);

        Y_computation_buffer[0] = 0;
        for (int i = 0; i < Y_step - 1; ++i) Y_computation_buffer[i + 1] = values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + ((X_step - 3) * STENCIL_SIZE_Y) + (i)];
        MPI_Isend(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);

        MPI_Recv(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 1; i < Y_step; ++i)
            values[prev_buffer * (STENCIL_SIZE_Y * STENCIL_SIZE_X) + ((X_step - 2) * STENCIL_SIZE_Y) + (i - 1)] = Y_computation_buffer[i];

        MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);



        for (x = 1; x < X_step - 1; x++) {
            for (y = 1; y < Y_step - 1; y++) {
                values[next_buffer * XY_plane + x * STENCIL_SIZE_Y + y] =
                        alpha * values[prev_buffer * XY_plane + (x - 1) * STENCIL_SIZE_Y + y] +
                        alpha * values[prev_buffer * XY_plane + (x + 1) * STENCIL_SIZE_Y + y] +
                        alpha * values[prev_buffer * XY_plane + x * STENCIL_SIZE_Y + y - 1] +
                        alpha * values[prev_buffer * XY_plane + x * STENCIL_SIZE_Y + y + 1] +
                        (1.0 - 4.0 * alpha) * values[prev_buffer * XY_plane + x * STENCIL_SIZE_Y + y];
                if (convergence && (fabs(values[prev_buffer * XY_plane + x * STENCIL_SIZE_Y + y] -
                                         values[next_buffer * XY_plane + x * STENCIL_SIZE_Y + y]) > epsilon)) {
                    convergence = 0;
                }
            }
        }
    }

    else {
        if (X_ind == 0) {
            for (int i = 1; i <= X_step; ++i) X_computation_buffer[i - 1] = values[prev_buffer * X_step * Y_step + (i) * Y_step - 2];
            MPI_Isend(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

            MPI_Recv(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 1; i <= X_step; ++i) values[prev_buffer * X_step * Y_step + (i) * Y_step - 1] = X_computation_buffer[i - 1];

            MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);
        }
        else if (X_ind == line_length - 1) {
            for (int i = 0; i < X_step; ++i) X_computation_buffer[i] = values[prev_buffer * X_step * Y_step + (i) * Y_step + 1];
            MPI_Isend(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind - 1, Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, &X_minus_request);

            MPI_Recv(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind - 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < X_step; ++i) values[prev_buffer * X_step * Y_step + (i) * Y_step] = X_computation_buffer[i];

            MPI_Wait(&X_minus_request, MPI_STATUS_IGNORE);
        }
        else {
            for (int i = 1; i <= X_step; ++i) X_computation_buffer[i - 1] = values[prev_buffer * X_step * Y_step + (i) * Y_step - 2];
            MPI_Isend(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, &X_plus_request);

            for (int i = 0; i < X_step; ++i) X_computation_buffer[i] = values[prev_buffer * X_step * Y_step + (i) * Y_step + 1];
            MPI_Isend(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind - 1, Y_ind), 2 + (4 * step + 1), MPI_COMM_WORLD, &X_minus_request);

            MPI_Recv(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind - 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < X_step; ++i) values[prev_buffer * X_step * Y_step + (i) * Y_step] = X_computation_buffer[i];

            MPI_Recv(X_computation_buffer, X_step, MPI_DOUBLE, get_rank(X_ind + 1, Y_ind), 2 + 4 * step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 1; i <= X_step; ++i) values[prev_buffer * X_step * Y_step + (i) * Y_step - 1] = X_computation_buffer[i - 1];

            MPI_Wait(&X_plus_request, MPI_STATUS_IGNORE);
            MPI_Wait(&X_minus_request, MPI_STATUS_IGNORE);
        }


        if (Y_ind == 0) {
            for (int i = 0; i < Y_step; ++i) Y_computation_buffer[i] = values[prev_buffer * (X_step * Y_step) + ((X_step - 2) * Y_step) + (i)];
            MPI_Isend(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);


            MPI_Recv(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < Y_step; ++i) values[prev_buffer * (X_step * Y_step) + ((X_step - 1) * Y_step) + (i)] = Y_computation_buffer[i];

            MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);
        }
        else if (Y_ind == line_length - 1) {
            for (int i = 0; i < Y_step; ++i) Y_computation_buffer[i] = values[prev_buffer * (X_step * Y_step) + (i) + Y_step];
            MPI_Isend(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind - 1), 2 + (4 * step + 3), MPI_COMM_WORLD, &Y_minus_request);

            MPI_Recv(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind - 1), 2 + (4 * step + 2), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < Y_step; ++i) values[prev_buffer * (X_step * Y_step) + (i)] = Y_computation_buffer[i];

            MPI_Wait(&Y_minus_request, MPI_STATUS_IGNORE);
        }
        else {
            for (int i = 0; i < Y_step; ++i) Y_computation_buffer[i] = values[prev_buffer * (X_step * Y_step) + ((X_step - 2) * Y_step) + (i)];
            MPI_Isend(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 2), MPI_COMM_WORLD, &Y_plus_request);


            for (int i = 0; i < Y_step; ++i) Y_computation_buffer[i] = values[prev_buffer * (X_step * Y_step) + (i) + Y_step];
            MPI_Isend(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind - 1), 2 + (4 * step + 3), MPI_COMM_WORLD, &Y_minus_request);

            MPI_Recv(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind - 1), 2 + (4 * step + 2), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < Y_step; ++i) values[prev_buffer * (X_step * Y_step) + (i)] = Y_computation_buffer[i];

            MPI_Recv(Y_computation_buffer, Y_step, MPI_DOUBLE, get_rank(X_ind, Y_ind + 1), 2 + (4 * step + 3), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < Y_step; ++i) values[prev_buffer * (X_step * Y_step) + ((X_step - 1) * Y_step) + (i)] = Y_computation_buffer[i];

            MPI_Wait(&Y_plus_request, MPI_STATUS_IGNORE);
            MPI_Wait(&Y_minus_request, MPI_STATUS_IGNORE);
        }

        int x_offset = (Y_ind == 0), y_offset = (X_ind == 0);
        int x_delim = (Y_ind == line_length - 1), y_delim = (X_ind == line_length - 1);

        for (x = 1 + x_offset; x < X_step - 1 - x_delim; x++) {
            for (y = 1 + y_offset; y < Y_step - 1 - y_delim; y++) {
                values[next_buffer * XY_plane + x * Y_step + y] =
                        alpha * values[prev_buffer * XY_plane + (x - 1) * Y_step + y] +
                        alpha * values[prev_buffer * XY_plane + (x + 1) * Y_step + y] +
                        alpha * values[prev_buffer * XY_plane + x * Y_step + y - 1] +
                        alpha * values[prev_buffer * XY_plane + x * Y_step + y + 1] +
                        (1.0 - 4.0 * alpha) * values[prev_buffer * XY_plane + x * Y_step + y];
                if (convergence && (fabs(values[prev_buffer * XY_plane + x * Y_step + y] -
                                         values[next_buffer * XY_plane + x * Y_step + y]) > epsilon)) {
                    convergence = 0;
                }
            }
        }
    }

    current_buffer = next_buffer;
    MPI_Allreduce(&convergence, &convergence, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

    return convergence;
}

int main(int argc, char **argv) {
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

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &N);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    line_length = (int) sqrt(N);

    stencil_init(line_length, id);
    if (id == -1) {
        printf("# init:\n");
        stencil_display(current_buffer, 0, STENCIL_SIZE_X, 0, STENCIL_SIZE_Y);
    }


    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    int s;

    for (s = 0; s < stencil_max_steps; s++) {
        int convergence = stencil_step(s);
        if (convergence) {
            break;
        }
    }

    stencil_finish(N, line_length);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    if (id == 0) {
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
