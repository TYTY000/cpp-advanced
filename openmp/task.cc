#include <omp.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <vector>
#define N 1000000  // 数组大小

void quicksort(int *array, int left, int right) {
    if (left < right) {
        int pivot = array[right];
        int l = left;
        int r = right - 1;
        while (l <= r) {
            while (l <= r && array[l] < pivot) l++;
            while (l <= r && array[r] > pivot) r--;
            if (l <= r) {
                int temp = array[l];
                array[l] = array[r];
                array[r] = temp;
                l++;
                r--;
            }
        }
        array[right] = array[l];
        array[l] = pivot;

#pragma omp task shared(array) if (right - left > 1000)
        quicksort(array, left, l - 1);

#pragma omp task shared(array) if (right - left > 1000)
        quicksort(array, l + 1, right);
    }
}

int main() {
    int *array = (int *)malloc(N * sizeof(int));

    // 初始化数组
    for (int i = 0; i < N; i++) {
        array[i] = rand() % N;
    }

    double start_time = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single nowait
        { quicksort(array, 0, N - 1); }
    }

    double end_time = omp_get_wtime();
    printf("Quicksort completed in %f seconds.\n", end_time - start_time);

    free(array);

    std::vector<double> arr(N);

    // 初始化随机数生成器
    std::srand(std::time(0));

    // 生成随机数
    std::generate(arr.begin(), arr.end(),
                  []() { return static_cast<double>(std::rand()) / RAND_MAX; });

    double start = omp_get_wtime();

    // 计算数组的和
    double total_sum = std::accumulate(arr.begin(), arr.end(), 0.0);

    double end = omp_get_wtime();
    printf("STL computed in %f seconds.\n", end - start);

    return 0;
}
