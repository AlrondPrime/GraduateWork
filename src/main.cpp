#include "CLI.hpp"
#include "Compression alternatives/Bit_RLE.hpp"
#include "Compression alternatives/Byte_RLE.hpp"
#include "MaskMultiply.hpp"
#include "pch.h"

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
    boost::filesystem::current_path(boost::filesystem::current_path().parent_path());

    if (argc > 1)
    {
        try
        {
            CLI cli{boost::filesystem::current_path() / "versions"};

            if (argv[1] == "add"s)
                cli.add(argv[2]);
            else if (argv[1] == "update"s)
                cli.update(argv[2]);
            else if (argv[1] == "restore"s)
                cli.restore(argv[2]);
        }
        catch (boost::exception &e)
        {
            std::cout << boost::diagnostic_information(e) << std::endl;
            throw;
        }
    }

    return 0;
}
