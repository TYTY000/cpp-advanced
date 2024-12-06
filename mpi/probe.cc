#include <mpi/mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Request req;
    MPI_Status status;
    int flag;
    MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

    if (MPI_Test(&req, &flag, MPI_STATUS_IGNORE)) {
        if (flag) {
            int count;
            MPI_Get_count(&status, MPI_INT, &count);
            printf("Rank %d: Message of size %d received\n", rank, count);
        } else {
            printf("Rank %d: No message available\n", rank);
        }
    }

    MPI_Finalize();
    return 0;
}
