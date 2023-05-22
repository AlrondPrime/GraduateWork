#ifndef NETWORKING_CLIENT_HPP
#define NETWORKING_CLIENT_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    using namespace boost;

    class Client {
    public:
        Client() :
                _work(make_work_guard(_io_context)),
                _connection(asio::ip::tcp::socket(_io_context), _io_context) {

            _connection.setOnMessageHandler(
                    [this](const Message &message) { msgHandler(message); });
        }

        ~Client() {
            _connection.disconnect();
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            log() << "Disconnected!";
        }

        void connectToServer(const std::string &host, const uint16_t port) {
            asio::ip::tcp::resolver resolver(_io_context);
            _endpoints = resolver.resolve(host, std::to_string(port));

            asio::async_connect(_connection.socket(), _endpoints,
                                [this](std::error_code ec, const asio::ip::tcp::endpoint &endpoint) {
                                    if (!ec) {
                                        log() << "Connected to Server!";
//                                        _connection.readHeader();
                                    }
                                });
        }

        // TODO: refactor later
        void mainLoop() {
//            _io_context.run();
          _context_thread = std::thread([this]() { _io_context.run(); });

        }

        void sendMsg(const Message &msg) {
            _connection.sendMsg(msg);
        }

        void sendFile(const bfs::path &root, const bfs::path &relPath) {
            log() << "Sending file \'" << root.string() << "\' / \'" << relPath.string() << "\'";
            _connection.sendFile(root, relPath);
        }

        void msgHandler(const Message &msg) {
            log() << "[Client]" << msg;
        }

        const filesystem::directory_entry &root() {
            return _root_dir;
        }

        void root(const filesystem::path &root) {
            _root_dir.assign(root);
        }

    private:
        logger::Logger log{"Client"};
        asio::io_context _io_context;
        asio::executor_work_guard<boost::asio::io_context::executor_type> _work;
        asio::ip::tcp::resolver::results_type _endpoints;
        std::thread _context_thread;
        Connection _connection;
        filesystem::directory_entry _root_dir;
    };
}

#endif //NETWORKING_CLIENT_HPP