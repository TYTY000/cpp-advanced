/******************************************************************************
 * FILE: mpi_vector.c
 * DESCRIPTION:
 *   MPI tutorial example code: Vector Derived Datatype
 * AUTHOR: Blaise Barney
 * LAST REVISED: 04/13/05
 ******************************************************************************/
#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#define SIZE 4

template <typename T>
void send_data(const T& data, int dest, int tag, MPI_Comm comm) {
    MPI_Send(data.data(), data.size(), MPI_FLOAT, dest, tag, comm);
}

template <typename T>
void recv_data(T& data, int source, int tag, MPI_Comm comm) {
    MPI_Recv(data.data(), data.size(), MPI_FLOAT, source, tag, comm, MPI_STATUS_IGNORE);
}

int main(int argc, char* argv[]) {
    int numtasks, rank, source = 0, dest, tag = 1, i;
    std::vector<std::vector<float>> a{{1.0, 2.0, 3.0, 4.0},
                                      {5.0, 6.0, 7.0, 8.0},
                                      {9.0, 10.0, 11.0, 12.0},
                                      {13.0, 14.0, 15.0, 16.0}};
    std::vector<float> b(SIZE, {});

    MPI_Status stat;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    if (numtasks == SIZE) {
        if (rank == 0) {
            int i{};
            for (const auto& v : a) send_data(v, i++, tag, MPI_COMM_WORLD);
        }

        recv_data(b, 0, tag, MPI_COMM_WORLD);
        printf("rank= %d  b= %3.1f %3.1f %3.1f %3.1f\n", rank, b[0], b[1], b[2],
               b[3]);
    } else
        printf("Must specify %d processors. Terminating.\n", SIZE);

    MPI_Finalize();
}
