#include <mpi/mpi.h>
#include <stdio.h>

#include <iostream>
#include <random>
#include <utility>

constexpr float p = 0.4;
constexpr int nodes = 5;

int main(int argc, char *argv[]) {
    int rank, numtasks;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::uniform_real_distribution dis(0.f, 1.f);
    auto get = [&] { return dis(gen); };
    std::vector<std::vector<int>> edgesMatrix;
    edgesMatrix.resize(nodes);
    for (size_t i = 0; i < nodes; i++) {
        for (size_t j = i + 1; j < nodes; j++) {
            if (get() < p) {
                edgesMatrix[i].push_back(j);
                edgesMatrix[j].push_back(i);
            }
        }
    }
    std::vector<int> I;
    std::vector<int> E;
    int edge_count = 0;
    for (size_t i = 0; i < nodes; ++i) {
        I.push_back(edge_count);
        for (int neighbor : edgesMatrix[i]) {
            E.push_back(neighbor);
            edge_count++;
        }
    }
    I.erase(I.begin());
    for (const auto e : I) {
        std::cout << e << '\t';
    }
    std::cout << '\n';
    for (const auto e : E) {
        std::cout << e << '\t';
    }
    std::cout << '\n';
    I.push_back(edge_count);  // 最后一个节点
    // 将生成的vector转换为C风格数组
    int indices[nodes];
    std::copy(I.begin(), I.end(), indices);
    int edges[E.size()];
    std::copy(E.begin(), E.end(), edges);

    MPI_Init(&argc, &argv);
    MPI_Comm graphcomm;

    // Create Graph topology
    MPI_Graph_create(MPI_COMM_WORLD, 5, indices, edges, 0, &graphcomm);
    MPI_Comm_rank(graphcomm, &rank);
    MPI_Comm_size(graphcomm, &numtasks);

    if (numtasks != 5) {
        printf("Must specify MP_PROCS= %d. Terminating.\n", 5);
        MPI_Finalize();
        exit(0);
    }

    int neighbors[4];
    int num_neighbors;
    MPI_Graph_neighbors_count(graphcomm, rank, &num_neighbors);
    MPI_Graph_neighbors(graphcomm, rank, num_neighbors, neighbors);

    printf("Rank %d has %d neighbors: ", rank, num_neighbors);
    for (int i = 0; i < num_neighbors; i++) {
        printf("%d ", neighbors[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}
