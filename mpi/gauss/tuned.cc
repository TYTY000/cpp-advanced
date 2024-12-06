#include <mpi/mpi.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>

void printMatrix(const float* m, int dim) {
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            std::cout << m[i * dim + j] << ' ';
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int num_tasks;

    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    const int dim = 1 << 12;
    // Assumes this divides evenly
    const int blockRow = dim / num_tasks;

    int task_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    std::unique_ptr<float[]> m;
    auto cache = std::make_unique<float[]>(dim * blockRow);
    auto pivotRow = std::make_unique<float[]>(dim);
    const int start = blockRow * task_id;
    const int end = start + blockRow;

    if (task_id == 0) {
        std::mt19937 gen(123);
        std::uniform_real_distribution dis(1.f, 2.f);
        m = std::make_unique<float[]>(dim * dim);
        std::generate(m.get(), m.get() + dim * dim, [&] { return dis(gen); });
    }
    double startT;
    if (task_id == 0) {
        startT = MPI_Wtime();
    }

    // MPI_Scatter(m.get(), dim * blockRow, MPI_FLOAT, cache.get(), dim *
    // blockRow,
    //             MPI_FLOAT, 0, MPI_COMM_WORLD);
    for (size_t i = 0; i < blockRow; i++) {
        MPI_Scatter(m.get() + i * dim * num_tasks, dim, MPI_FLOAT,
                    cache.get() + i * dim, dim, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    // std::vector<MPI_Request> requests(num_tasks);

    for (size_t row = 0; row < blockRow; row++) {
        for (size_t rank = 0; rank < num_tasks; rank++) {
            auto global_col = row * num_tasks + rank;
            if (rank == task_id) {
                auto pivot = cache[row * dim + global_col];
                for (size_t col = global_col; col < dim; col++) {
                    cache[row * dim + col] /= pivot;
                }

                MPI_Bcast(cache.get() + row * dim, dim, MPI_FLOAT, rank,
                          MPI_COMM_WORLD);
                for (size_t i = row + 1; i < blockRow; i++) {
                    auto scale = cache[i * dim + global_col];

                    for (size_t j = global_col; j < dim; j++) {
                        cache[i * dim + j] -= scale * cache[row * dim + j];
                    }
                }
            } else {
                MPI_Bcast(pivotRow.get(), dim, MPI_FLOAT, rank, MPI_COMM_WORLD);

                auto localStart = (task_id < rank) ? row + 1 : row;
                for (size_t i = localStart; i < blockRow; i++) {
                    auto scale = cache[i * dim + global_col];

                    for (size_t j = global_col; j < dim; j++) {
                        cache[i * dim + j] -= scale * pivotRow[j];
                    }
                }
            }
        }
    }

    // MPI_Gather(cache.get(), dim * blockRow, MPI_FLOAT, m.get(), dim *
    // blockRow, MPI_FLOAT, 0, MPI_COMM_WORLD);
    for (size_t i = 0; i < blockRow; i++) {
        MPI_Gather(cache.get() + i * dim, dim, MPI_FLOAT,
                   m.get() + num_tasks * i * dim, dim, MPI_FLOAT, 0,
                   MPI_COMM_WORLD);
    }
    // Print result from rank 0
    if (task_id == 0) {
        double endT = MPI_Wtime();
        std::cout << endT - startT << " Seconds!\n";
        // print_matrix(matrix.get(), dim);
    }

    // if (task_id == 0) {
    //     printMatrix(m.get(), dim);
    // }
    MPI_Finalize();

    return 0;
}
