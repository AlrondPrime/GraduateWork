#ifndef NETWORKING_SERVER_HPP
#define NETWORKING_SERVER_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    using namespace boost;

    class Server {
    public:
        explicit Server(const uint16_t port) :
                _work(make_work_guard(_io_context)),
                _endpoint{asio::ip::tcp::v4(), port},
                _acceptor{_io_context, _endpoint} {

            buffer.resize(MAX_BODY_SIZE);
        }

        virtual ~Server() {
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            log() << "Stopped!";
        }

        // TODO: refactor later
        void mainLoop() {
            _io_context.run();
//            _context_thread = std::thread([this]() { _io_context.run(); });
        }

        void Start() {
            waitForClients();

            log() << "Started!";
        }

        void waitForClients() {
            _acceptor.async_accept(
                    [this](boost::system::error_code ec, asio::ip::tcp::socket socket) {
                        if (!ec) {
                            log() << "New Connection: " << boost::asio::ip::detail::endpoint(
                                    socket.remote_endpoint().address(), socket.remote_endpoint().port())
                                    .to_string();
                            _connections.push_back(std::make_shared<Connection>(std::move(socket), _io_context));
                            _connections.back()->setOnMessageHandler(
                                    [this](const Message &message) { msgHandler(message); });
                            _connections.back()->readHeader();
                            _connections.back()->processIncoming();
                        } else {
                            log() << "New Connection Error: " << ec.message();
                        }
//                        waitForClients();
                    });
        }

        void msgHandler(const Message &msg) {
//            log() << msg;
            switch (msg.header().msgType()) {
                case MsgType::FileHeader: {
                    log() << "Handling " << to_string(msg.header().msgType());
                    auto pos = msg.body().rfind('\n');
                    if (pos == std::string::npos) {
                        log() << "Corrupted File Header";
                    }

                    auto file_size = strtol(msg.body().c_str() + pos, nullptr, 10);
                    packages_to_wait = file_size / MAX_BODY_SIZE + 1;

                    bfs::path path{_root_dir.path().string() + "\\" +
                                   msg.body().substr(0, pos)};

/*                    if (!bfs::exists(path))
                        // ..if not, create this folder
                        if (!bfs::create_directories(path.parent_path())) {
                            log() << "ERROR at line " << __LINE__;
                            return;
                        }*/

                    ofs.open(path.string());
                    if (!ofs.is_open()) {
                        log() << "Can't open ofstream";
                    } else
                        log() << "Opened file \'" << path.string() << "\'";

                    break;
                }
                case MsgType::FileTransfer: {
                    log() << "Handling " << to_string(msg.header().msgType());
                    if (!ofs.is_open()) {
                        log() << "Ofstream isn't opened";
                    }
                    ofs << msg.body();
                    --packages_to_wait;
                    if (packages_to_wait <= 0) {
                        log() << "Whole file transferred";
                        ofs.close();
                    }

                    break;
                }
                default: {
                    log() << "Handling " << to_string(msg.header().msgType());
                    break;
                }
            }
        }

        const filesystem::directory_entry &root() {
            return _root_dir;
        }

        void root(const filesystem::path &root) {
            _root_dir.assign(root);
        }

    private:
        logger::Logger log{"Server"};
        asio::io_context _io_context;
        asio::executor_work_guard<boost::asio::io_context::executor_type> _work;
        asio::ip::tcp::endpoint _endpoint;
        asio::ip::tcp::acceptor _acceptor;
        std::thread _context_thread;
        ts_deque<std::shared_ptr<Connection>> _connections;
        size_t packages_to_wait = 0;
        std::ofstream ofs;
        std::string buffer;
        filesystem::directory_entry _root_dir;
    };
}

#endif //NETWORKING_SERVER_HPP