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
    const int begin = blockRow * task_id;
    const int end = begin + blockRow;

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

    MPI_Scatter(m.get(), dim * blockRow, MPI_FLOAT, cache.get(), dim * blockRow,
                MPI_FLOAT, 0, MPI_COMM_WORLD);

    std::vector<MPI_Request> requests(num_tasks);

    for (size_t row = 0; row < end; row++) {
        auto rank = row / blockRow;
        if (rank == task_id) {
            auto i = row % blockRow;
            auto pivot = cache[i * dim + row];
            for (size_t j = row; j < dim; j++) {
                cache[i * dim + j] /= pivot;
            }

            for (size_t r = rank + 1; r < num_tasks; r++) {
                MPI_Isend(cache.get() + dim * i, dim, MPI_FLOAT, r, 0,
                          MPI_COMM_WORLD, &requests[r]);
            }

            for (size_t x = i + 1; x < blockRow; x++) {
                auto scale = cache[x * dim + row];
                for (size_t y = row; y < dim; y++) {
                    cache[x * dim + y] -= scale * cache[i * dim + y];
                }
            }

            for (size_t r = rank + 1; r < num_tasks; r++) {
                MPI_Wait(&requests[r], MPI_STATUS_IGNORE);
            }
        } else {
            MPI_Recv(pivotRow.get(), dim, MPI_FLOAT, rank, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            for (size_t i = 0; i < blockRow; i++) {
                auto scale = cache[i * dim + row];

                for (size_t j = row; j < dim; j++) {
                    cache[i * dim + j] -= scale * pivotRow[j];
                }
            }
        }
    }

    MPI_Gather(cache.get(), dim * blockRow, MPI_FLOAT, m.get(), dim * blockRow,
               MPI_FLOAT, 0, MPI_COMM_WORLD);
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
