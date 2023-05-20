#ifndef GRADUATEWORK_TESTMASK_HPP
#define GRADUATEWORK_TESTMASK_HPP

#include "pch.h"

    class MaskMultiply_test
{
    static constexpr unsigned long long s_1Kb = 1 * 1000 * 8;
    static constexpr unsigned long long s_500Kb = 500 * 1000 * 8;
    static constexpr unsigned long long s_1Mb = 1 * 1000 * 1000 * 8;
    static constexpr unsigned long long s_100Mb = 100 * 1000 * 1000 * 8;

    std::vector<unsigned long long> arr1;
    std::vector<unsigned long long> arr2;
    std::vector<unsigned long long> arr3;

    MaskMultiply_test()
    {
        arr1.resize(s_100Mb / 8);
        arr2.resize(arr1.size());
        arr3.resize(arr1.size());
    }

    static int getRandomBitValue()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> distribution(0, 1);

        return distribution(gen);
    }

    static void measure(const std::function<void()> &f, size_t count)
    {
        long long buffer = 0;
        std::chrono::time_point<std::chrono::steady_clock> starts_time;
        std::chrono::time_point<std::chrono::steady_clock> ends_time;
        std::chrono::milliseconds elapsed;
        for (auto i = 0; i < count; ++i)
        {
            starts_time = std::chrono::steady_clock::now();
            f();
            ends_time = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(ends_time - starts_time);
            buffer += elapsed.count();
        }
        std::cout << "count: " << count << ", ";
        std::cout << "average " << buffer / count << " ms\n";
    }

    void compute()
    {
#pragma omp parallel for default(none) shared(arr1, arr2, arr3)
        for (auto i = 0; i < arr3.size(); ++i)
            arr3[i] = arr1[i] & arr2[i];
    }

    void test()
    {
        auto starts_time = std::chrono::steady_clock::now();

        std::cout << "Filling\n";
#pragma omp parallel for default(none) shared(arr1, arr2, arr3)
        for (auto i = 0; i < arr3.size(); ++i)
            for (auto j = 0; j < 8; ++j)
            {
                arr1[i] = getRandomBitValue() << j;
                arr2[i] = getRandomBitValue() << j;
            }

        std::cout << "Calculating\n";
        measure(std::function<void()>([this]
                                      { compute(); }), 100);

        auto ends_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(ends_time - starts_time);
        std::cout << "Total " << elapsed.count() << " ms\n";
    }
};

#endif //GRADUATEWORK_TESTMASK_HPP