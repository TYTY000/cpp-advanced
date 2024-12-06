#include <numbers>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <omp.h>

#include <random>

TEST_CASE("MC") {
    const long long samples = 100000000LL;
    long long in_circle = 0;

    double start = omp_get_wtime();
#pragma omp parallel
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
#pragma omp for reduction(+ : in_circle)
        for (long long i = 0; i < samples; i++) {
            double x = dis(gen);
            double y = dis(gen);
            if (x * x + y * y <= 1) {
                ++in_circle;
            }
        }
    }
    double end = omp_get_wtime();
    double pi_est = 4.0 * in_circle / samples;
    std::cout << "Time taken: " << (end - start) << " seconds" << std::endl;
    std::cout << "PI est: " << pi_est << std::endl;
    REQUIRE(pi_est == doctest::Approx(std::numbers::pi).epsilon(0.03));
}
