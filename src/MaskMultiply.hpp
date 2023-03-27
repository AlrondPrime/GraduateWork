#ifndef GRADUATEWORK_MASKMULTIPLY_HPP
#define GRADUATEWORK_MASKMULTIPLY_HPP

#include "pch.h"

class MaskMultiply
{
private:
    using BUFFER_TYPE = uint64_t;
    static constexpr int BUFFER_SIZE = sizeof(BUFFER_TYPE);
    using byte_type = char;

    static std::array<byte_type, BUFFER_SIZE> intToArray(std::uint64_t int_from)
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

    static BUFFER_TYPE arrayToInt(const std::array<byte_type, BUFFER_SIZE> &array_from)
    {
        BUFFER_TYPE int_to = __builtin_bswap64(*reinterpret_cast<const BUFFER_TYPE *>(array_from.data()));

        return int_to;
    }

public:
    MaskMultiply()
    = default;

    MaskMultiply(std::ifstream &first, std::ifstream &second, std::ofstream &out)
            : first_stream(std::move(first)), second_stream(std::move(second)), out_stream(std::move(out))
    {}

    /** @brief Produces mask file which represents changes between first and second files
     * @details Optimizes operations by simultaneously processing 8 bytes at a time
     * @param curr Last version of the file
     * @param prev Previous version of the file
     * @param mask Resultant file which represents changes between first and second files
     */
    static void createMask8(const boost::filesystem::path &curr, const boost::filesystem::path &prev,
                            const boost::filesystem::path &mask)
    {
        std::ifstream first_stream(absolute(curr).string());
        std::ifstream second_stream(absolute(prev).string());
        std::ofstream out_stream(absolute(mask).string());
        std::array<byte_type, BUFFER_SIZE> first_buffer{}, second_buffer{}, out_buffer{};
        BUFFER_TYPE first_value{}, second_value{}, out_value{};

        while (true)
        {
            first_buffer.fill('\0');
            second_buffer.fill('\0');
            out_buffer.fill('\0');
            first_stream.read(first_buffer.data(), first_buffer.size());
            second_stream.read(second_buffer.data(), second_buffer.size());
            first_value = arrayToInt(first_buffer);
            second_value = arrayToInt(second_buffer);
            out_value = (first_value ^ second_value);
            out_buffer = intToArray(out_value);


            if (!first_stream.eof() && !second_stream.eof())
            {
                std::copy(out_buffer.begin(),
                          out_buffer.end(),
                          std::ostream_iterator<char>{out_stream}
                );
            } else
            {
                auto p = find_if_not(out_buffer.rbegin(),
                                     out_buffer.rend(),
                                     [](uint8_t val)
                                     { return val == '\0'; });

                if (p != out_buffer.rend())
                    std::copy(out_buffer.begin(),
                              p.base(),
                              std::ostream_iterator<char>{out_stream}
                    );
                break;
            }
        }


        if (first_stream.eof())
        {
            out_stream << second_stream.rdbuf();
        } else if (second_stream.eof())
        {
            out_stream << first_stream.rdbuf();
        } else
        {
            std::cout << "Something went wrong" << '\n';
        }
    }

    /** @brief Produces mask file which represents changes between first and second files
     * @details Processes 1 byte at a time
     */
    void createMask1()
    {
        char first_buffer{}, second_buffer{}, out_buffer{};

        while (first_stream.read(&first_buffer, sizeof first_buffer) &&
               second_stream.read(&second_buffer, sizeof second_buffer)
                )
        {
            out_buffer = static_cast<char>(first_buffer & second_buffer);
            out_stream << out_buffer;
        }

        if (first_stream.eof())
        {
            out_stream << second_stream.rdbuf();
        } else if (second_stream.eof())
        {
            out_stream << first_stream.rdbuf();
        } else
        {
            std::cout << "Something went wrong" << '\n';
        }
    }

private:
    std::ifstream first_stream;
    std::ifstream second_stream;
    std::ofstream out_stream;
};

#endif //GRADUATEWORK_MASKMULTIPLY_HPP