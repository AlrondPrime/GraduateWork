#include "../pch.h"
#include "../Byte.hpp"

#ifndef GRADUATEWORK_BIT_RLE_HPP
#define GRADUATEWORK_BIT_RLE_HPP

class BitCodingModel
{


public:
    using value_type = Byte::value_type;

    BitCodingModel(std::basic_ifstream<char> &in, std::basic_ofstream<char> &out) : in(in), out(out)
    {
        buffer = in.get();
        value = buffer & (1 << 7);
        result.setValue(value);
        encodeBits(6);
    }

    BitCodingModel &operator<<(stream manipulator)
    {
        if (manipulator == stream::flush)
        {
            std::cout << (int) result.getValue() << ' ' << (int) result.getAmount() << '\n' << std::flush;
            //out << result;
        }
        return *this;
    }

    BitCodingModel &operator<<(value_type val)
    {
        buffer = val;
        encodeBits(7);

        return *this;
    }

    void encodeBits(int pos)
    {
        for (int i = pos; i >= 0; --i)
        {
            value = buffer & (1 << i);
            if (value == result.getValue() && result.getAmount() < 127)
            {
                result.incrementAmount();
            } else
            {
                std::cout << (int) result.getValue() << ' ' << (int) result.getAmount() << '\n' << std::flush;
                //out << result;
                result.setValue(value);
                result.resetAmount();
            }
        }
    }
private:
    std::basic_ifstream<char> &in;
    std::basic_ofstream<char> &out;

    Byte result;
    bool value;
    value_type buffer;
};

#endif //GRADUATEWORK_BIT_RLE_HPP
