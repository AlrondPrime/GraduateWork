#include "pch.h"
#include "MaskMultiply.hpp"
#include "Compression alternatives/Byte_RLE.hpp"
#include "Compression alternatives/Bit_RLE.hpp"
#include "CLI.hpp"

using namespace std;
static_assert(sizeof(uint8_t) == 1, "");

/*
 * char - unsigned by default
 * 0 - signed
 * 1 - unsigned
 * */

int main(int argc, char **argv)
{
    std::ios_base::sync_with_stdio(false);

//    CLI cli{boost::filesystem::directory_entry(boost::filesystem::current_path().parent_path())};
//    cli.add("test data/test 1/first.hpp");
//    cli.update("test data/test 1/first.hpp");

    boost::filesystem::current_path(boost::filesystem::current_path().parent_path());
    MaskMultiply::createMask8("test data/test 2/curr.hpp",
                                "test data/test 2/prev.hpp", "test data/test 2/mask.hpp");
    MaskMultiply::createMask8("test data/test 2/curr.hpp",
                                "test data/test 2/mask.hpp", "test data/test 2/prev.hpp");

    return 0;
}
