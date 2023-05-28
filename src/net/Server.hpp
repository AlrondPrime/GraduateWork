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

        const bfs::path &root() {
            return _versions_dir;
        }

        void root(const bfs::path &root) {
            _versions_dir.assign(root / ".versions");
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
                                    std::move(socket), _io_context, _versions_dir));
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
            log() << "Handling " << to_string(msg.header().msgType());

            switch (msg.header().msgType()) {
                case MsgType::RestoreVersion: {
                    bfs::path path{_versions_dir.path() / msg.body()};
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
                            bfs::recursive_directory_iterator(_versions_dir)) {
                        if (bfs::is_regular_file(path) &&
                            // Check if it is not hidden folder and
                            // is subdir of root server folder
                            (++path.path().lexically_relative(_versions_dir).begin())->string().front() != '.') {

                            log() << "Emplace back \'"
                                  << path.path().lexically_relative(_versions_dir).string() << "\'";
                            paths.emplace_back(path.path().lexically_relative(_versions_dir).string());
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
                            if (!bfs::exists(_versions_dir / path.as_string())) {
                                log() << "Doesn't exist: \'"
                                      << path.as_string().c_str() << "\'";
                            } else {
                                log() << "Found existing, sending : \'"
                                      << path.as_string().c_str() << "\'";
                                _connections.back()->sendFile(_versions_dir, path.as_string());
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
                    log() << "Handling default";
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
        bfs::directory_entry _versions_dir;
    };
}

#endif //NETWORKING_SERVER_HPP