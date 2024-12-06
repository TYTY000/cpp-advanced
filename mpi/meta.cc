#include <mpi/mpi.h>
#include <iostream>
#include <vector>
#include <type_traits>
#include <cstring>

// 定义模板结构用于类型标记
template<typename T, bool IsSimple>
struct TypeTag {};

// 特化发送函数，用于将数据转换为字节并发送（简单类型）
template<typename T>
void send_data_impl(const T& data, int dest, int tag, MPI_Comm comm, TypeTag<T, true>) {
    static_assert(std::is_trivially_copyable_v<T>, "Data must be trivially copyable");
    size_t data_size = sizeof(T);
    MPI_Send(&data, data_size, MPI_BYTE, dest, tag, comm);
}

// 特化接收函数，用于接收字节并恢复数据（简单类型）
template<typename T>
void recv_data_impl(T& data, int source, int tag, MPI_Comm comm, TypeTag<T, true>) {
    static_assert(std::is_trivially_copyable_v<T>, "Data must be trivially copyable");
    size_t data_size = sizeof(T);
    MPI_Recv(&data, data_size, MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);
}

// 特化发送函数，用于向量数据（复杂类型）
template<typename T>
void send_data_impl(const std::vector<T>& data, int dest, int tag, MPI_Comm comm, TypeTag<std::vector<T>, false>) {
    size_t data_size = data.size() * sizeof(T);
    MPI_Send(data.data(), data_size, MPI_BYTE, dest, tag, comm);
}

// 特化接收函数，用于向量数据（复杂类型）
template<typename T>
void recv_data_impl(std::vector<T>& data, int source, int tag, MPI_Comm comm, TypeTag<std::vector<T>, false>) {
    MPI_Status status;
    MPI_Probe(source, tag, comm, &status);
    int count;
    MPI_Get_count(&status, MPI_BYTE, &count);
    data.resize(count / sizeof(T));
    MPI_Recv(data.data(), count, MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);
}

// 基础模板函数声明，带有默认参数
template<typename T>
void send_data(const T& data, int dest, int tag, MPI_Comm comm) {
    send_data_impl(data, dest, tag, comm, TypeTag<T, std::is_trivially_copyable_v<T>>{});
}

template<typename T>
void recv_data(T& data, int source, int tag, MPI_Comm comm) {
    recv_data_impl(data, source, tag, comm, TypeTag<T, std::is_trivially_copyable_v<T>>{});
}

// 自定义数据结构
struct MyData {
    int id;
    double value;
};

// 发送函数重载，用于 MyData 类型（简单类型）
void send_data(const MyData& data, int dest, int tag, MPI_Comm comm) {
    send_data_impl(data, dest, tag, comm, TypeTag<MyData, true>{});
}

void recv_data(MyData& data, int source, int tag, MPI_Comm comm) {
    recv_data_impl(data, source, tag, comm, TypeTag<MyData, true>{});
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        MyData data = {42, 3.14};
        std::vector<int> vec = {1, 2, 3, 4};

        for (int i = 1; i < size; ++i) {
            send_data(data, i, 0, MPI_COMM_WORLD);
            send_data(vec, i, 1, MPI_COMM_WORLD);
        }
    } else {
        MyData data;
        std::vector<int> vec;

        recv_data(data, 0, 0, MPI_COMM_WORLD);
        recv_data(vec, 0, 1, MPI_COMM_WORLD);

        std::cout << "Rank " << rank << " received: id = " << data.id << ", value = " << data.value << std::endl;
        std::cout << "Rank " << rank << " received vector: ";
        for (int v : vec) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}

