#include <omp.h>

#include <iostream>

int counter = 0;
#pragma omp threadprivate(counter)

int increment_counter() {
    counter++;
    return counter;
}

int main() {
// counter = 42;
// #pragma omp parallel num_threads(4) copyin(counter)
// {
#pragma omp parallel num_threads(8)
    {
#pragma omp single copyprivate(counter)
        { counter = 42; }
        for (int i = 0; i < 5; ++i) {
            int result = increment_counter();
#pragma omp critical
            std::cout << "Thread " << omp_get_thread_num()
                      << " incremented counter to " << result << std::endl;
        }
    }

    return 0;
}
