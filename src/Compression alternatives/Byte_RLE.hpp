#include "../pch.h"

#ifndef GRADUATEWORK_BYTE_RLE_HPP
#define GRADUATEWORK_BYTE_RLE_HPP

class ByteCodingModel
{
    std::basic_ifstream<char> &in;
    std::basic_ofstream<char> &out;

    uint8_t buffer;
    uint8_t amount = 1;

public:
    ByteCodingModel(std::basic_ifstream<char> &in, std::basic_ofstream<char> &out) : in(in), out(out)
    {
        buffer = this->in.get();
    }

    ByteCodingModel &operator<<(stream manipulator)
    {
        if (manipulator == stream::flush)
        {
            out << buffer << amount;
            std::cout << (int) buffer << ' ' << (int) amount << '\n' << std::flush;
        }
            return *this;
    }

    ByteCodingModel &operator<<(uint8_t val)
    {
        if (val == buffer && amount < 255)
        {
            ++amount;
        } else
        {
            out << buffer << amount;
            std::cout << (int)buffer << ' ' << (int)amount << '\n' << std::flush;
            amount = 1;
            buffer = val;
        }
        return *this;
    }


};

#endif //GRADUATEWORK_BYTE_RLE_HPP