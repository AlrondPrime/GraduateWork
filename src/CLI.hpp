#ifndef GRADUATEWORK_CLI_HPP
#define GRADUATEWORK_CLI_HPP

#include "pch.h"
#include "MaskMultiply.hpp"

class CLI
{
private:
    boost::filesystem::directory_entry root;
public:
    explicit CLI(boost::filesystem::directory_entry root) : root(std::move(root))
    {
        boost::filesystem::current_path(this->root);
    }

    void add(const boost::filesystem::path &path)
    {
        boost::filesystem::create_directories("versions" / path.filename());

        boost::filesystem::copy_file(path, "versions" / path.filename() / path.filename(),
                                     boost::filesystem::copy_options::overwrite_existing);
    }

    void update(const boost::filesystem::path &path)
    {
        boost::filesystem::path previous{"versions" / path.filename() / path.filename()};
        boost::filesystem::path mask{"versions" / path.filename() / "mask.hpp"};

        if (!boost::filesystem::exists(previous) || boost::filesystem::is_empty(previous.parent_path()))
            add(path);
        else
        {
            MaskMultiply::createMask8(absolute(path), previous, mask);
            boost::filesystem::remove(previous);
        }
    }

    void restore(const boost::filesystem::path &path)
    {
        boost::filesystem::path previous{"versions" / path.filename() / path.filename()};
        boost::filesystem::path mask{"versions" / path.filename() / "mask.hpp"};
        MaskMultiply::createMask8(absolute(path), mask, previous);
    }
};

#endif //GRADUATEWORK_CLI_HPP