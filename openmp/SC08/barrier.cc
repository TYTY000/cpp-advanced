#include <omp.h>
#include <stdio.h>

int main() {
    int x = 0;

    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        
        if (thread_id == 0) {
            x = 1;
            #pragma omp barrier // 同步点1
            printf("Thread %d: x = %d\n", thread_id, x); // 必须在同步点1之后
        } else if (thread_id == 1) {
            #pragma omp barrier // 同步点1
            x = 2;
            printf("Thread %d: x = %d\n", thread_id, x); // 必须在同步点1之后
        }
    }

    return 0;
}

