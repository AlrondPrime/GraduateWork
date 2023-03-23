#include "pch.h"
#include "MaskMultiply.hpp"
#include "Compression alternatives/Byte_RLE.hpp"
#include "Compression alternatives/Bit_RLE.hpp"
#include "boost/filesystem.hpp"

using namespace std;
static_assert(sizeof(uint8_t) == 1, "");

/*
 * char - unsigned by default
 * 0 - signed
 * 1- unsigned
 * */


int main(int argc, char **argv)
{
    std::ios_base::sync_with_stdio(false);

    std::ifstream first{"../test data/test 1/first.hpp", std::ios::binary};
    std::ifstream second{"../test data/test 1/second.hpp", std::ios::binary};
    std::ofstream out{"../test data/test 1/out.hpp", std::ios::binary};

    MaskMultiply mask(first, second, out);
    mask.createMask1();
    out.close();
    std::ifstream check{"../test data/test 1/out.hpp", std::ios::binary};
    std::cout << check.rdbuf();


    return 0;
}
