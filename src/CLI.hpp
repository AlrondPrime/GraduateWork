#ifndef GRADUATEWORK_CLI_HPP
#define GRADUATEWORK_CLI_HPP

#include "MaskMultiply.hpp"
#include "pch.h"

class CLI
{
private:
    boost::filesystem::directory_entry root;
    int filename_postfix{1};
    boost::string_view filename_extension{".maskversion"};
    std::ofstream file_creator;

    static std::string getDateTime()
    {
        auto ptime{boost::posix_time::second_clock::local_time()};
        /// The locale is responsible for deleting the facet.
        auto facet = new boost::posix_time::time_facet{"(%Y-%m-%d)(%H-%M)#"};
        std::stringstream format_buffer;
        format_buffer.imbue(std::locale(std::cout.getloc(), facet));
        format_buffer << ptime;
        return format_buffer.str();
    }

    std::string createFilename()
    {
        return (getDateTime() + std::to_string(filename_postfix++)).append(filename_extension.data());
    }

public:
    explicit CLI(const boost::filesystem::path &root)
            : root(root)
    {}

    void add(const boost::filesystem::path &path)
    {
        if (!boost::filesystem::exists(root.path().filename() / path.filename()))
            if (!boost::filesystem::create_directories(root.path().filename() / path.filename()))
            {
                std::cout << "ERROR" << std::endl;
                return;
            }

        boost::filesystem::copy_file(path, root / path.filename() / path.filename(),
                                     boost::filesystem::copy_options::overwrite_existing);
    }

    void update(const boost::filesystem::path &path)
    {
        boost::filesystem::path previous{root.path().filename() / path.filename() / path.filename()};
        boost::filesystem::path mask{root.path().filename() / path.filename() / createFilename()};

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
        boost::filesystem::path previous{root / path.filename() / path.filename()};
        boost::filesystem::path mask{root / path.filename() / "(2023-03-28)(01-25)#1.maskversion"};

        boost::filesystem::copy_file(path, previous,
                                     boost::filesystem::copy_options::overwrite_existing);
        MaskMultiply::createMask8(previous, mask, absolute(path));
    }
};

#endif // GRADUATEWORK_CLI_HPP