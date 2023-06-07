#ifndef NETWORKING_CLIENT_HPP
#define NETWORKING_CLIENT_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    class Client {
    public:
        Client() :
                _work(make_work_guard(_io_context)),
                _connection(ba::ip::tcp::socket(_io_context), _io_context,
                        // TODO: REFACTOR LATER!!!
                            R"(..\ClientStorage\.versions)") {

            _connection.setOnMessageHandler(
                    [this](const Message &message) { _msgHandler(message); });
        }

        ~Client() {
            _connection.disconnect();
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            log() << "Disconnected!";
        }

        void connectToServer(const std::string &host, const uint16_t port) {
            ba::ip::tcp::resolver resolver(_io_context);
            _endpoints = resolver.resolve(host, std::to_string(port));

            ba::async_connect(_connection.socket(), _endpoints,
                              [this](std::error_code ec, const ba::ip::tcp::endpoint &endpoint) {
                                  if (!ec) {
                                      log() << "Connected to Server!";
                                      _connected = true;
                                      _connection.readHeader();
                                      _connection.processIncoming();
                                  } else {
                                      log() << "Can't connect to Server!";
                                  }
                              });
        }

        void mainLoop() {
//            _io_context.run();
            _context_thread = std::thread([this]() { _io_context.run(); });

        }

        void sendMsg(const Message &msg) {
            if (!_connected) {
                log() << "Doesn't connected, sleeping";
                std::this_thread::sleep_for(3s);
                if (!_connected) {
                    log() << "Still isn't connected, going to fail";
                }
            }
            _connection.sendMsg(msg);
        }

        void sendMsg(Message &&msg) {
            if (!_connected) {
                log() << "Doesn't connected, sleeping";
                std::this_thread::sleep_for(3s);
                if (!_connected) {
                    log() << "Still isn't connected, going to fail";
                }
            }
            _connection.sendMsg(std::move(msg));
        }

        void sendFile(const bfs::path &root, const bfs::path &relPath) {
            log() << "Sending file \'" << root.string() << "\' / \'" << relPath.string() << "\'";
            _connection.sendFile(root, relPath);
        }

        void setOnMessageHandler(std::function<void(const Message &)> onMessageHandler) {
            _connection.setOnMessageHandler(std::move(onMessageHandler));
        }

        void resetOnMessageHandler() {
            _connection.resetOnMessageHandler();
        }

        void _msgHandler(const Message &msg) {

        }

        const bfs::directory_entry &root() {
            return _root_dir;
        }

        void root(const bfs::path &root) {
            _root_dir.assign(root);
        }

        const Connection &connection() {
            return _connection;
        }

    private:
        logger::Log log{"Client"};
        ba::io_context _io_context;
        ba::executor_work_guard<boost::asio::io_context::executor_type> _work;
        ba::ip::tcp::resolver::results_type _endpoints;
        // Connection field must be below io_context field!
        Connection _connection;
        bfs::directory_entry _root_dir;
        std::thread _context_thread;
        bool _connected{false};
    };
}

#endif //NETWORKING_CLIENT_HPP