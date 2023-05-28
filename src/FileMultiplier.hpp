#ifndef GRADUATEWORK_FILEMULTIPLIER_HPP
#define GRADUATEWORK_FILEMULTIPLIER_HPP

#include "pch.h"
#include "logger/Logger.hpp"

using BUFFER_TYPE = uint64_t;
static constexpr int BUFFER_SIZE = sizeof(BUFFER_TYPE);
using byte_type = char;

std::array<byte_type, BUFFER_SIZE> intToArray(BUFFER_TYPE int_from) {
    std::array<byte_type, BUFFER_SIZE> array_to{};
//        int_from = __builtin_bswap64(int_from);
    copy(reinterpret_cast<byte_type *>(&int_from),
         reinterpret_cast<byte_type *>(&int_from) + BUFFER_SIZE,
         array_to.begin()
    );

    return array_to;
}

BUFFER_TYPE arrayToInt(const std::array<byte_type, BUFFER_SIZE> &array_from) {
//        BUFFER_TYPE int_to = __builtin_bswap64(*reinterpret_cast<const BUFFER_TYPE *>(array_from.data()));
    BUFFER_TYPE int_to = *reinterpret_cast<const BUFFER_TYPE *>(array_from.data());

    return int_to;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, std::array<byte_type, 8> array) {
    out << '\"';
    std::copy(
            array.begin(),
            array.begin() + array.size(),
            std::ostream_iterator<char>{out}
    );
    out << '\"';
    return out;
}

std::array<byte_type, BUFFER_SIZE>
operator^(const std::array<byte_type, BUFFER_SIZE> &array_from1,
          const std::array<byte_type, BUFFER_SIZE> &array_from2) {
    BUFFER_TYPE int_to1{arrayToInt(array_from1)};
    BUFFER_TYPE int_to2{arrayToInt(array_from2)};
    BUFFER_TYPE int_from{int_to1 ^ int_to2};

    return intToArray(int_from);
}

class FileMultiplier {
public:
    FileMultiplier()
    = default;

    /*FileMultiplier(std::ifstream &first, std::ifstream &second, std::ofstream &out)
            : first_stream(std::move(first)), second_stream(std::move(second)), out_stream(std::move(out)) {}*/

    /** @brief Produces diff file which represents changes between first and second files
     * @details
     * The first two files do not change
     * <br>
     * The third file changes
     * <br>
     * Optimizes operations by simultaneously processing 8 bytes at a time
     * @param curr Last version of the file
     * @param prev Previous version of the file
     * @param diff Resultant file which represents changes between first and second files
     */
    void multiplyFiles_8(const boost::filesystem::path &curr,
                                const boost::filesystem::path &prev,
                                const boost::filesystem::path &diff) {
#ifdef _DEBUG
        log() << "Multiplying files:"
                    << "\n\tcurr=\t\'" << curr.string()
                    << "\'\n\tprev=\t\'" << prev.string()
                    << "\'\n\tdiff=\t\'" << diff.string()
                    << "\'";
#endif

        if (!exists(curr) || !exists(prev)) {
            std::cerr << "Any of input files does not exist" << std::endl;
            return;
        }

        std::ifstream first_stream(absolute(curr).string());
        std::ifstream second_stream(absolute(prev).string());
        std::ofstream out_stream(absolute(diff).string());
        std::array<byte_type, BUFFER_SIZE> first_buffer{}, second_buffer{}, out_buffer{};
        BUFFER_TYPE first_value{}, second_value{}, out_value{};

        while (true) {
            first_buffer.fill('\0');
            second_buffer.fill('\0');
            out_buffer.fill('\0');
            first_stream.read(first_buffer.data(), first_buffer.size());
            second_stream.read(second_buffer.data(), second_buffer.size());
            // Multiply char buffers as digit values
//            first_value = arrayToInt(first_buffer);
//            second_value = arrayToInt(second_buffer);
//            out_value = first_value ^ second_value;
//            out_buffer = intToArray(out_value);
            out_buffer = first_buffer ^ second_buffer;

            if (!first_stream.eof() && !second_stream.eof()) {
                // Both buffers are fully filled, just output them
                std::copy(out_buffer.begin(),
                          out_buffer.end(),
                          std::ostream_iterator<char>{out_stream}
                );
            } else {
                // Both buffers are not fully filled, find the logical ending and output data before it
                auto iter = find_if_not(
                        out_buffer.rbegin(),
                        out_buffer.rend(),
                        [](uint8_t val) { return val == '\0'; });

                if (iter != out_buffer.rend())
                    std::copy(out_buffer.begin(),
                              iter.base(),
                              std::ostream_iterator<char>{out_stream}
                    );
                break;
            }
        }


        if (first_stream.eof()) {
            log() << "First stream ended";
            out_stream << second_stream.rdbuf();
        } else if (second_stream.eof()) {
            log() << "Second stream ended";
            out_stream << first_stream.rdbuf();
        }
    }

    /** @brief Produces mask file which represents changes between first and second files
     * @details Processes 1 byte at a time
     */
    /*void multiplyFiles_1() {
        char first_buffer{}, second_buffer{}, out_buffer{};

        while (first_stream.read(&first_buffer, sizeof first_buffer) &&
               second_stream.read(&second_buffer, sizeof second_buffer)) {

            out_buffer = static_cast<char>(first_buffer & second_buffer);
            out_stream << out_buffer;
        }

        if (first_stream.eof()) {
            out_stream << second_stream.rdbuf();
        } else if (second_stream.eof()) {
            out_stream << first_stream.rdbuf();
        } else {
            log() << "Something went wrong" << '\n';
        }
    }*/

private:
/*    std::ifstream first_stream;
    std::ifstream second_stream;
    std::ofstream out_stream;*/
    logger::Logger log{"FileMultiplier"};
};

#endif //GRADUATEWORK_FILEMULTIPLIER_HPP