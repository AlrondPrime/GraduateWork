#ifndef GRADUATEWORK_VERSIONCONTROLLER_HPP
#define GRADUATEWORK_VERSIONCONTROLLER_HPP

#include "../pch.h"
#include "../FileMultiplier.hpp"
#include "../net/Client.hpp"
#include "../net/Message.hpp"
#include "PathResolver.hpp"

namespace vcs {
    class VersionController {
    public:
        explicit VersionController(const std::string &root) :
                _file_resolver(root), _version_resolver(root) {
            _net_client.root(root);
            _net_client.connectToServer(_host, _port);

            _net_client.mainLoop();
            _observer = std::thread{[this]() { _observe(); }};
        }

        const bfs::directory_entry &root() {
            return _net_client.root();
        }

        void root(const bfs::path &root) {
            _net_client.root(root);
        }

        /// @brief Add file under version control system
        void add(const bfs::path &pathToFile) {
            if (_blocked)
                return;
            log() << "Add";
            bfs::path versions_folder{_file_resolver.fileVersionsFolder(pathToFile)};
            // Check if folder for storing current file versionsDir exists
            if (!bfs::exists(versions_folder) || !bfs::is_directory(versions_folder))
                // ..if not, create this folder
                if (!bfs::create_directories(versions_folder)) {
                    std::cerr << "Can't create versionsDir folder" << std::endl;
                    return;
                }

            // Save current file state to calculate it's diff in future
            _copy(_file_resolver.main(pathToFile), _file_resolver.copy(pathToFile));
            _net_client.sendFile(_version_resolver.versionsDir(), pathToFile / pathToFile.filename());
        }

        /// @brief Update changed file
        /// @details Calculate current file diff and save it as version file
        void update(const bfs::path &pathToFile) {
            if (_blocked)
                return;
            log() << "Update";

            bfs::path versions_folder{_file_resolver.fileVersionsFolder(pathToFile)};
            // Check if folder for storing current file versionsDir exists
            if (!bfs::exists(versions_folder) || !bfs::is_directory(versions_folder))
                // ..if not, create this folder
                if (!bfs::create_directories(versions_folder)) {
                    std::cerr << "Can't create versionsDir folder" << std::endl;
                    return;
                }

            // Check if versionsDir folder of current file is empty
            if (bfs::is_empty(versions_folder)) {
                // ..if yes, just copy current file there
                add(pathToFile);
                return;
            }

            std::string previous_filename =
                    _getLastVersionFile(versions_folder)
                            .stem().string();
            int postfix = 0;
            // Check if previous filename was found
            if (!previous_filename.empty())
                //..if yes, just increment it's postfix
                postfix = strtol(&previous_filename.back(), nullptr, 10)
                          + 1;

            bfs::path filename{_createFilename(postfix)};
            _file_multiplier.multiplyFiles_8(_file_resolver.main(pathToFile),
                                             _file_resolver.copy(pathToFile),
                                             versions_folder / filename);
            _copy(_file_resolver.main(pathToFile), _file_resolver.copy(pathToFile));
            // Sending copy file
            _net_client.sendFile(_version_resolver.versionsDir(), pathToFile / pathToFile.filename());
            // Sending version file
            _net_client.sendFile(_file_resolver.versionsDir(), pathToFile / filename);
        }

        /**
         * @brief Restore file in storage folder to corresponding version
         * @param pathToVersion Path to .version file
         */
        void restore(const bfs::path &pathToVersion) {
            if (_blocked)
                return;
            log() << "Restore";

            std::vector<bfs::path> versions;
            bool found = false;
            for (auto &iter:
                    bfs::directory_iterator(_version_resolver.fileVersionsFolder(pathToVersion))) {
                if (!is_regular_file(iter))
                    continue;

                if (iter.path().extension() != _filename_ext)
                    continue;

                if (equivalent(iter.path(), pathToVersion))
                    found = true;

                if (found)
                    versions.insert(versions.begin(), iter);
            }
            assert(!versions.empty());

            for (auto &version: versions) {
                _file_multiplier.multiplyFiles_8(_version_resolver.copy(pathToVersion),
                                                 version,
                                                 _version_resolver.main(pathToVersion));
                _copy(_version_resolver.main(pathToVersion), _version_resolver.copy(pathToVersion));
            }

            for (auto &version: versions)
                _remove(version);

            // Sending command to remove unnecessary versions
            net::Message::MessageHeader messageHeader{net::MsgType::RestoreVersion};
            net::Message msg{messageHeader, pathToVersion.string()};
            _net_client.sendMsg(std::move(msg));
        }

        void _observe() {
            std::vector<bfs::path> paths;
            std::ifstream ifs{_observables.string()};
            std::string buffer;

            if (!ifs.is_open()) {
                std::cerr << "Not opened!" << std::endl;
                return;
            }
            while (std::getline(ifs, buffer)) {
                paths.emplace_back(buffer);
            }
            ifs.close();

            while (true) {
                // TODO: refactor
                if (!_blocked)
                    for (auto &item: paths) {
                        assert(bfs::exists(_file_resolver.main(item)));
                        assert(bfs::exists(_file_resolver.copy(item)));
                        if (bfs::last_write_time(_file_resolver.main(item))
                            != bfs::last_write_time(_file_resolver.copy(item))) {

                            _copy(_file_resolver.main(item),
                                  _file_resolver.copy(item));
                        }
                    }
                std::this_thread::sleep_for(1s);
            }
        }

        void addObservable(const bfs::path &path) {
            if (_blocked)
                return;
            std::ofstream ofs{_observables.string(), std::ios::ate};
            if (!ofs.is_open()) {
                std::cerr << "Not opened!" << std::endl;
                return;
            }
            ofs << path.string() << '\n';
            ofs.close();
        }

        void check_integrity(const bfs::path &) {
            _blocked = true;
            _net_client.setOnMessageHandler(
                    [this](const net::Message &message) { _msgHandler(message); });

            net::Message::MessageHeader messageHeader{net::MsgType::CheckIntegrity};
            _net_client.sendMsg(net::Message{messageHeader});
        }

        void _msgHandler(const net::Message &msg) {
            log() << "Handling " << to_string(msg.header().msgType());

            switch (msg.header().msgType()) {
                case net::MsgType::CheckIntegrity: {
                    bj::array paths;
                    bj::array paths_to_reply;

                    try {
                        paths = bj::parse(msg.body()).as_array();
                        for (auto &path: paths) {
                            if (!bfs::exists(_version_resolver.versionsDir() / path.as_string())
                            && !bfs::exists(_file_resolver.storageDir() / path.as_string())) {
                                log() << "Doesn't exist: \'"
                                      << path.as_string().c_str() << "\'";
                                paths_to_reply.push_back(path);
                            } else {
                                log() << "Found existing : \'"
                                      << path.as_string().c_str() << "\'";
                            }
                        }

                        if (paths_to_reply.empty())
                            log() << "All are here!";
                        else {
                            net::Message reply_message{net::MsgType::RequestFiles,
                                                       bj::serialize(paths_to_reply)};
                            _net_client.sendMsg(std::move(reply_message));
                        }
                    }
                    catch (boost::exception &e) {
                        std::cerr << "Corrupted File Header";
                        std::cerr << diagnostic_information_what(e) << std::endl;
                    }

                    break;
                }
                    // TODO: refactor later
                case net::MsgType::FileHeader: {
                    log() << msg;

                    // Try to parse json string got from message body
                    try {
                        bj::object object{bj::parse(msg.body()).as_object()};
                        _path.assign(_version_resolver.versionsDir() /
                                     object.at("path").as_string());
                        _file_size = static_cast<uintmax_t>(object.at("_file_size").as_int64());
                        packages_to_wait = _file_size / net::MAX_BODY_SIZE + 1;
                        _modification_time = object.at("_modification_time").as_int64();
                    }
                    catch (boost::exception &e) {
                        std::cerr << "Corrupted File Header\n";
                        std::cerr << diagnostic_information_what(e) << "\n";
                    }

                    // Check if folder for storing current file versions exists
                    if (!bfs::exists(_path.parent_path()))
                        // ..if not, create this folder
                        if (!bfs::create_directories(_path.parent_path())) {
                            log() << "Can't create versions directory";
                            return;
                        }

                    _ofs.open(_path.string());
                    if (!_ofs.is_open()) {
                        log() << "Can't open ofstream";
                        return;
                    } else
                        log() << "Opened file \'" << _path.string() << "\'";

                    break;
                }
                case net::MsgType::FileTransfer: {
                    log() << "Handling " << to_string(msg.header().msgType());
                    if (!_ofs.is_open()) {
                        log() << "Ofstream isn't opened";
                        return;
                    }
                    _ofs << msg.body();
                    --packages_to_wait;
                    if (packages_to_wait <= 0) {
                        log() << "Whole file transferred";
                        _ofs.close();
                        bfs::last_write_time(_path, _modification_time);
                        _path.assign("");
                        _file_size = 0;
                        _modification_time = 0;
                    }

                    break;
                }

                default: {
                    log() << "Handling default " << to_string(msg.header().msgType());
                    break;
                }
            }
        }

    private:
        // Directory for storing versions
        std::string_view _filename_ext{".version"};
        std::string_view _filename_pattern{"(%Y-%m-%d)(%H-%M)#"};

        std::string _getDateTime() {
            auto ptime{boost::posix_time::second_clock::local_time()};
            // The locale is responsible for deleting the facet.
            auto facet = new boost::posix_time::time_facet{_filename_pattern.data()};
            std::stringstream format_buffer;
            format_buffer.imbue(std::locale(std::cout.getloc(), facet));
            format_buffer << ptime;

            return format_buffer.str();
        }

        std::string _createFilename(int filenamePostfix = 1) {
            return (_getDateTime() + std::to_string(filenamePostfix))
                    .append(_filename_ext.data());
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
            log() << "Latest version file found:\'" << last_version.string() << "\'";

            return last_version;
        }

        /// @brief Simply copies a file
        bool _copy(const bfs::path &from, const bfs::path &to) {
            log() << "Copying file"
                  << "\n\tfrom\t\'" << from.string()
                  << "\'\n\tto\t\'" << to.string()
                  << "\'";
            assert(bfs::exists(from) && bfs::is_regular_file(from));
            assert(!bfs::exists(from) || bfs::is_regular_file(from));
            assert(bfs::exists(to.parent_path()) && bfs::is_directory(to.parent_path()));

            bool error_code = bfs::copy_file(from, to, bfs::copy_options::overwrite_existing);

            assert(bfs::last_write_time(from) == bfs::last_write_time(to));

            return error_code;
        }

        /// @brief Simply removes a file
        bool _remove(const bfs::path &path) {
            log() << "Deleting file \'" << path.string() << "\'";
            assert(bfs::exists(path) && bfs::is_regular_file(path));
            return bfs::remove(path);
        }

        logger::Logger log{"VCS"};
        std::string _host{"localhost"};
        uint16_t _port{60000};
        FileMultiplier _file_multiplier;

        net::Client _net_client;

        const vcs::FileResolver _file_resolver;
        const vcs::VersionResolver _version_resolver;
        std::thread _observer;
        bfs::path _observables{"../observables.txt"};

        // TODO: refactor
        bool _blocked{false};
        size_t packages_to_wait = 0;
        std::string _buffer;
        std::ofstream _ofs;
        bfs::path _path;
        uintmax_t _file_size;
        time_t _modification_time;
    };
}

#endif //GRADUATEWORK_VERSIONCONTROLLER_HPP