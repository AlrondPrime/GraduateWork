#ifndef GRADUATEWORK_VERSIONCONTROLLER_HPP
#define GRADUATEWORK_VERSIONCONTROLLER_HPP

#include "../pch.h"
#include "../Log.hpp"
#include "../FileMultiplier.hpp"
#include "Resolver.hpp"
#include "../net/Client.hpp"
#include "../net/Message.hpp"

namespace vcs {
    class VersionController {
    public:
        VersionController() {
            _net_client.root(_root_dir);
            _net_client.connectToServer(_host, _port);

            _net_client.mainLoop();
//            _net_client.sendFile("Data.txt");
        }

        /// @brief Add file under version control system
        void add(const bfs::path &pathToFile) {
#ifdef _DEBUG
            log() << "Add";
#endif
            bfs::path versions_folder{fileResolver.versionsFolder(pathToFile)};
            // Check if folder for storing current file versions exists
            if (!bfs::exists(versions_folder) ||
                !bfs::is_directory(versions_folder))
                // ..if not, create this folder
                if (!bfs::create_directories(versions_folder)) {
                    std::cerr << "ERROR at line " << __LINE__ << std::endl;
                    return;
                }

            // Save current file state to calculate it's diff in future
            _copy(fileResolver.storage(pathToFile), fileResolver.copy(pathToFile));
            _net_client.sendFile(pathToFile);
        }

        /// @brief Update changed file
        /// @details Calculate current file diff and save it as version file
        void update(const bfs::path &pathToFile) {
#ifdef _DEBUG
            log() << "Update";
#endif
            // Check if versions folder of current file is empty
            if (bfs::is_empty(fileResolver.versionsFolder(pathToFile))) {
                // ..if yes, just copy current file there
                add(pathToFile);
                return;
            }

            std::string previous_filename =
                    _getLastVersionFile(fileResolver.versionsFolder(pathToFile))
                            .stem().string();
            int postfix = 0;
            // Check if previous version was found
            if (!previous_filename.empty())
                //..if yes, just increment it's postfix
                postfix = strtol(&previous_filename.back(), nullptr, 10) + 1;

            bfs::path version{_createFilename(postfix)};
            _file_multiplier.multiplyFiles_8(fileResolver.storage(pathToFile),
                                            fileResolver.copy(pathToFile),
                                            fileResolver.versionsFolder(pathToFile) / version);
            _copy(fileResolver.storage(pathToFile), fileResolver.copy(pathToFile));
        }

        /**
         * @brief Restore file in storage to corresponding version
         * @param pathToVersion Path to .version file
         */
        void restore(const bfs::path &pathToVersion) {
#ifdef _DEBUG
            log() << "Restore";
#endif
            bool found = false;
            std::vector<bfs::path> versions;

            for (auto &iter: bfs::directory_iterator(versionResolver.versionsFolder(pathToVersion))) {
                if (!is_regular_file(iter))
                    continue;

                if (iter.path().extension() != _filename_ext)
                    continue;

                if (iter.path().filename() == pathToVersion.filename())
                    found = true;

                if (found) {
                    versions.insert(versions.begin(), iter);
                }
            }

            for (auto &version: versions) {
                _file_multiplier.multiplyFiles_8(versionResolver.copy(pathToVersion),
                                                version,
                                                versionResolver.storage(pathToVersion));
                _copy(versionResolver.storage(pathToVersion), versionResolver.copy(pathToVersion));
            }

            for (auto &version: versions) {
                _remove(version);
            }
        }

    private:
        /// Directory for storing versions
        boost::string_view _filename_ext{".version"};
        boost::string_view _filename_pattern{"(%Y-%m-%d)(%H-%M)#"};

        std::string _getDateTime() {
            auto ptime{boost::posix_time::second_clock::local_time()};
            /// The locale is responsible for deleting the facet.
            auto facet = new boost::posix_time::time_facet{_filename_pattern.data()};
            std::stringstream format_buffer;
            format_buffer.imbue(std::locale(std::cout.getloc(), facet));
            format_buffer << ptime;
            return format_buffer.str();
        }

        std::string _createFilename(int filenamePostfix = 1) {
            return (_getDateTime() + std::to_string(filenamePostfix)).append(_filename_ext.data());
        }

        bfs::path _getLastVersionFile(const bfs::path &dir) {
            bfs::path last_version;
            time_t last_write_time_{0};

            for (auto &iter: bfs::directory_iterator(dir)) {
                if (iter.path().extension() != _filename_ext)
                    continue;

                if (bfs::last_write_time(iter) > last_write_time_) {
                    last_write_time_ = bfs::last_write_time(iter);
                    last_version = iter;
                }
            }
#ifdef _DEBUG
            log() << "Latest version file found:\'" << last_version.string() << "\'";
#endif
            return last_version;
        }

        /// @brief Simply copies a file
        bool _copy(const bfs::path &from, const bfs::path &to) {
#ifdef _DEBUG
            log() << "Copying file"
                   << "\n\tfrom\t\'" << from.string()
                   << "\'\n\tto\t\'" << to.string()
                   << "\'";
#endif
            return bfs::copy_file(from, to,
                                  bfs::copy_options::overwrite_existing);
        }

        /// @brief Simply removes a file
        bool _remove(const bfs::path &p) {
#ifdef _DEBUG
            log() << "Deleting file \'" << p.string() << "\'";
#endif
            return bfs::remove(p);
        }

        logger::Logger log{"VCS"};
        std::string _host{"localhost"};
        uint16_t _port{60000};
        std::string _root_dir{R"(C:\Users\alrondprime\CLionProjects\GraduateWork\ClientStorage)"};
        FileMultiplier _file_multiplier;

        net::Client _net_client;
    };
}

#endif //GRADUATEWORK_VERSIONCONTROLLER_HPP