#include "pch.h"

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, std::array<char, 8> array)
{
    std::copy(
            array.begin(),
            array.begin() + array.size(),
            std::ostream_iterator<char>{out}
    );
    return out;
}

class MaskMultiply
{
private:
    using BUFFER_TYPE = uint64_t;
    static constexpr int BUFFER_SIZE = sizeof(BUFFER_TYPE);
    using byte_type = char;

    std::array<byte_type, BUFFER_SIZE> intToArray(std::uint64_t int_from)
{
    std::array<byte_type, BUFFER_SIZE> array_to{};
    int_from = __builtin_bswap64(int_from);
    copy(
            reinterpret_cast<byte_type *>(&int_from),
            reinterpret_cast<byte_type *>(&int_from) + BUFFER_SIZE,
            array_to.begin()
    );

    return array_to;
}

BUFFER_TYPE arrayToInt(const std::array<byte_type, BUFFER_SIZE> &array_from)
{
    BUFFER_TYPE int_to = __builtin_bswap64(*reinterpret_cast<const BUFFER_TYPE *>(array_from.data()));

    return int_to;
}

public:
    MaskMultiply(std::ifstream &first,
                 std::ifstream &second,
                 std::ofstream &out)
            : m_first(first), m_second(second), m_out(out)
    {}

    void createMask1()
    {
        char first_buffer{}, second_buffer{}, out_buffer{};

        while (m_first.read(&first_buffer, sizeof first_buffer) &&
               m_second.read(&second_buffer, sizeof second_buffer)
                )
        {
            out_buffer = static_cast<char>(first_buffer & second_buffer);
            m_out << out_buffer;
        }

        if (m_first.eof())
        {
            m_out << m_second.rdbuf();
        } else if (m_second.eof())
        {
            m_out << m_first.rdbuf();
        } else
        {
            std::cout << "Something went wrong" << '\n';
        }
    }

private:
    std::ifstream &m_first;
    std::ifstream &m_second;
    std::ofstream &m_out;
};


