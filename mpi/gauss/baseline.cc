
#include <algorithm>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>
using M = std::vector<std::vector<float>>;

void printMatrix(const M& m) {
    for (const auto& row : m) {
        for (const auto& elem : row) {
            std::cout << elem << ' ';
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::vector<std::vector<float>> m;
    const int dim = 1 << 12;
    std::mt19937 gen(123);
    std::uniform_real_distribution dis(1.f, 2.f);
    std::generate_n(std::back_inserter(m), dim, [&] {
        std::vector<float> row;
        std::generate_n(std::back_inserter(row), dim, [&] { return dis(gen); });
        return row;
    });
    for (size_t row = 0; row < dim; row++) {
        auto pivot = m[row][row];
        for (size_t col = row; col < dim; col++) {
            m[row][col] /= pivot;
        }
        for (size_t i = row + 1; i < dim; i++) {
            auto scale = m[i][row];
            for (size_t j = 0; j < dim; j++) {
                m[i][j] -= m[row][j] * scale;
            }
        }
    }

    // printMatrix(m);
    return 0;
}
