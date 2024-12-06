#include <mpi/mpi.h>

#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get the task ID
    int task_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    if (task_id == 0) {
        int num_tasks;
        MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

        for (int i = 1; i < num_tasks; i++) {
            MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < num_tasks; i++) {
            int recv;
            MPI_Recv(&recv, 1, MPI_INT, i, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            std::cout << "Received " << recv << " from rank " << i << '\n';
        }
    } else {
        int recv;
        MPI_Recv(&recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        recv *= recv;
        MPI_Send(&recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Finish our MPI work
    MPI_Finalize();
    return 0;
}
