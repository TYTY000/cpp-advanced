/******************************************************************************
 * FILE: mpi_bug3.c
 * DESCRIPTION:
 *   This program has a bug.
 * SOURCE: Blaise Barney
 * LAST REVISED: 04/13/05
 ******************************************************************************/
#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#define ARRAYSIZE 16000000
#define MASTER 0

float* data;

int main(int argc, char* argv[]) {
    int numtasks, taskid, rc, dest, offset, i, j, tag1, tag2, source, chunksize;
    float mysum, sum;
    float update(int myoffset, int chunk, int myid);
    MPI_Status status;

    /***** Initializations *****/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    if (numtasks % 4 != 0) {
        printf("Quitting. Number of MPI tasks must be divisible by 4.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(0);
    }
    printf("MPI task %d has started...\n", taskid);
    chunksize = (ARRAYSIZE / numtasks);
    tag2 = 1;
    tag1 = 2;

    /* Initialize the array */
    data = (float*)malloc(ARRAYSIZE * sizeof(float));
    if (data == NULL) {
        printf("Error allocating memory for data array.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    float check;
    /***** Master task only ******/
    if (taskid == MASTER) {
        sum = 0;
        for (i = 0; i < ARRAYSIZE; i++) {
            data[i] = i * 1.0;
            sum = sum + data[i];
        }
        printf("Initialized array sum = %e\n", sum);

        /* Send each task its portion of the array - master keeps 1st part */
        offset = chunksize;
        for (dest = 1; dest < numtasks; dest++) {
            MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunksize, MPI_FLOAT, dest, tag2,
                     MPI_COMM_WORLD);
            printf("Sent %d elements to task %d offset= %d\n", chunksize, dest,
                   offset);
            offset = offset + chunksize;
        }

        /* Master does its part of the work */
        offset = 0;
        mysum = update(offset, chunksize, taskid);
    } else {
        /* Receive my portion of array from the master task */
        source = MASTER;
        MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(data, chunksize, MPI_FLOAT, source, tag2, MPI_COMM_WORLD,
                 &status);
        mysum = update(offset, chunksize, taskid);
    } /* end of non-master */

    MPI_Reduce(&mysum, &check, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
    
    if (taskid == MASTER) {
        /* Get final sum and print sample results */
        // printf("Sample results: \n");
        // offset = 0;
        // for (i = 0; i < numtasks; i++) {
        //     for (j = 0; j < 5; j++) printf("  %e", data[offset + j]);
        //     printf("\n");
        //     offset = offset + chunksize;
        // }
        printf("*** Final sum= %e ***\n", check);
    }
    free(data);
    MPI_Finalize();

} /* end of main */

float update(int myoffset, int chunk, int myid) {
    int i;
    float mysum;
    /* Perform addition to each of my array elements and keep my sum */
    mysum = 0;
    for (i = myoffset; i < myoffset + chunk; i++) {
        data[i] = data[i] + i * 1.0;
        mysum = mysum + data[i];
    }
    printf("Task %d mysum = %e\n", myid, mysum);
    return (mysum);
}
