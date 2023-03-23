#include "pch.h"

class MaskMultiply_
{
public:
    MaskMultiply_(std::basic_ifstream<char> &first,
                  std::basic_ifstream<char> &second,
                  std::basic_ofstream<char> &out)
            : in1(first), in2(second), out(out)
    {
        assert(first.gcount() == second.gcount() && "Files must have equal sizes");
        char first_buffer, second_buffer, out_buffer;

        std::bitset<8> first_bitset;
        std::bitset<8> second_bitset;
        std::bitset<8> out_bitset;

        while ((first >> first_buffer))
        {
            second >> second_buffer;
            out_buffer = first_buffer & second_buffer;
            out << out_buffer;

            first_bitset = first_buffer;
            second_bitset = second_buffer;
            out_bitset = out_buffer;
            std::cout << first_bitset.to_string() << '\n';
            std::cout << second_bitset.to_string() << '\n';
            std::cout << out_bitset.to_string() << '\n';
            std::cout << '\n';
        }
    }

private:
    std::basic_ifstream<char> &in1;
    std::basic_ifstream<char> &in2;
    std::basic_ofstream<char> &out;
};
MaskMultiply
{
    static constexpr unsigned long long s_1Kb = 1 * 1000 * 8;
    static constexpr unsigned long long s_500Kb = 500 * 1000 * 8;
    static constexpr unsigned long long s_1Mb = 1 * 1000 * 1000 * 8;
    static constexpr unsigned long long s_100Mb = 100 * 1000 * 1000 * 8;

    std::vector<unsigned long long> arr1;
    std::vector<unsigned long long> arr2;
    std::vector<unsigned long long> arr3;

    MaskMultiply()
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