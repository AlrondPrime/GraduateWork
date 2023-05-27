#ifndef NETWORKING_SERVER_HPP
#define NETWORKING_SERVER_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    class Server {
    public:
        explicit Server(const uint16_t port) :
                _work(make_work_guard(_io_context)),
                _endpoint{ba::ip::tcp::v4(), port},
                _acceptor{_io_context, _endpoint} {

            _buffer.resize(MAX_BODY_SIZE);
        }

        virtual ~Server() {
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            log() << "Stopped!";
        }

        void mainLoop() {
            _io_context.run();
//            _context_thread = std::thread([this]() { _io_context.run(); });
        }

        void Start() {
            _waitForClients();

            log() << "Started!";
        }

        const bfs::directory_entry &root() {
            return _root_dir;
        }

        void root(const bfs::path &root) {
            _root_dir.assign(root);
        }

    private:
        void _waitForClients() {
            _acceptor.async_accept(
                    [this](boost::system::error_code ec, ba::ip::tcp::socket socket) {
                        if (!ec) {
                            log() << "New Connection: " << ba::ip::detail::endpoint(
                                    socket.remote_endpoint().address(), socket.remote_endpoint().port())
                                    .to_string();
                            _connections.push_back(std::make_shared<Connection>(
                                    std::move(socket), _io_context));
                            _connections.back()->setOnMessageHandler(
                                    [this](const Message &message) { _msgHandler(message); });
                            _connections.back()->readHeader();
                            _connections.back()->processIncoming();
                        } else {
                            log() << "New Connection Error: " << ec.message();
                        }
                        _waitForClients();
                    });
        }

        void _msgHandler(const Message &msg) {
            log() << msg;

            switch (msg.header().msgType()) {
                case MsgType::FileHeader: {
                    log() << "Handling " << to_string(msg.header().msgType());

                    // Try to parse json string got from message body
                    try {
                        bj::object object{bj::parse(msg.body()).as_object()};
                        _path.assign(_root_dir.path() / ".versions" /
                                     object.at("path").as_string());
                        _file_size = static_cast<uintmax_t>(object.at("_file_size").as_int64());
                        packages_to_wait = _file_size / MAX_BODY_SIZE + 1;
                        _modification_time = object.at("_modification_time").as_int64();
                    }
                    catch (boost::exception &e) {
                        std::cerr << "Corrupted File Header";
                        std::cerr << diagnostic_information_what(e) << std::endl;
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
                case MsgType::FileTransfer: {
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
                case MsgType::RestoreVersion: {
                    log() << "Handling " << to_string(msg.header().msgType());
                    bfs::path path{_root_dir.path() / msg.body()};
                    if (!bfs::exists(path)) {
                        log() << "Path or directory \'" << path.string() << "\' does not exist";
                        return;
                    }

                    std::vector<bfs::path> versions;
                    bool found = false;

                    for (auto &iter: bfs::directory_iterator(path.parent_path())) {
                        if (!is_regular_file(iter))
                            continue;

                        if (iter.path().extension() != path.extension())
                            continue;

                        if (iter.path().filename() == path.filename())
                            found = true;

                        if (found)
                            versions.insert(versions.begin(), iter);
                    }

                    for (auto &version: versions)
                        bfs::remove(version);

                    break;
                }
                case MsgType::CheckIntegrity: {
                    bj::array paths;
                    for (auto &path:
                    bfs::recursive_directory_iterator(_root_dir / ".versions")) {
                        if (bfs::is_regular_file(path) &&
                            /* Check if it is not hidden folder and
                             * is subdir of root server folder
                             */
                            (++path.path().lexically_relative(_root_dir).begin())->string().front() != '.') {

                            log() << "Emplace back \'"
                            << path.path().lexically_relative(_root_dir / ".versions").string() << "\'";
                            paths.emplace_back(path.path().lexically_relative(_root_dir / ".versions").string());
                        }
                    }

                    Message reply_message{MsgType::CheckIntegrity, bj::serialize(paths)};
                    _connections.back()->sendMsg(std::move(reply_message));

                    break;
                }

                case MsgType::RequestFiles: {
                    bj::array paths;

                    try {
                        paths = bj::parse(msg.body()).as_array();
                        for (auto &path: paths) {
                            if (!bfs::exists(_root_dir / path.as_string())) {
                                log() << "Doesn't exist: \'"
                                      << path.as_string().c_str() << "\'";
                            } else {
                                log() << "Found existing, sending : \'"
                                      << path.as_string().c_str() << "\'";
                                _connections.back()->sendFile(_root_dir, path.as_string());
                            }
                        }
                    }
                    catch (boost::exception &e) {
                        std::cerr << "Corrupted File Header";
                        std::cerr << diagnostic_information_what(e) << std::endl;
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
        logger::Logger log{"Server"};
        ba::io_context _io_context;
        ba::executor_work_guard<ba::io_context::executor_type> _work;
        ba::ip::tcp::endpoint _endpoint;
        ba::ip::tcp::acceptor _acceptor;
        std::thread _context_thread;
        ts_deque<std::shared_ptr<Connection>> _connections;
        bfs::directory_entry _root_dir;

        // TODO: refactor later
        size_t packages_to_wait = 0;
        std::string _buffer;
        std::ofstream _ofs;
        bfs::path _path;
        uintmax_t _file_size;
        time_t _modification_time;
    };
}

#endif //NETWORKING_SERVER_HPP