#include <mpi/mpi.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int task_id{}, num_tasks{};
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    const int num_elem = 1 << 10;
    const int blockSize = num_elem / num_tasks;

    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    std::vector<int> sends;
    // std::unique_ptr<int[]> sends;
    if (task_id == 0) {
        sends.resize(num_elem);
        // sends = std::make_unique<int[]>(num_elem);
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::uniform_int_distribution dis(1, 1);

        std::generate(sends.begin(), sends.end(), [&] { return dis(gen); });
    }
    // auto recvs{std::make_unique<int[]>(blockSize)};
    std::vector<int> recvs;
    recvs.resize(blockSize);

    MPI_Scatter(sends.data(), blockSize, MPI_INT, recvs.data(), blockSize,
                MPI_INT, 0, MPI_COMM_WORLD);
    auto partialSum = std::reduce(recvs.begin(), recvs.end());

    int globalSum{};
    MPI_Reduce(&partialSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (task_id == 0) {
        std::cout << globalSum << '\n';
    }

    MPI_Finalize();
    return 0;
}
