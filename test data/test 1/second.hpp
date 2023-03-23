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
