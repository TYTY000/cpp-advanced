#include <omp.h>
#include <stdio.h>

#include <iostream>
#include <limits>

int main() {
    int N1 = 1000, N2 = 1000;
    double array[N1][N2];

    double start = omp_get_wtime();
    omp_set_schedule(omp_sched_auto, 10);
#pragma omp parallel for collapse(2)
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < N2; j++) {
            array[i][j] = i + j;
        }
    }
    double end = omp_get_wtime();
    printf("Completed initialization with %f seconds.\n", end - start);

    std::cout << omp_get_thread_limit() << std::endl;
    std::cout << omp_get_max_threads() << std::endl;
    std::cout << omp_get_max_active_levels() << std::endl;
    std::cout << omp_get_max_teams() << std::endl;
    return 0;
}
