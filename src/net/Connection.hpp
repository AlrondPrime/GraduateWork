#ifndef NETWORKING_CONNECTION_HPP
#define NETWORKING_CONNECTION_HPP

#include "../pch.h"
#include "../logger/Logger.hpp"
#include "ts_deque.hpp"

namespace net {
    using namespace boost;

    class Connection :
            public std::enable_shared_from_this<Connection> {
    public:
        explicit Connection(asio::ip::tcp::socket socket, asio::io_context &io_context) :
                _socket(std::move(socket)), _io_context(io_context) {
        }

        bool connected() const {
            return _socket.is_open();
        }

        /// @details Asynchronous function
        void disconnect() {
            if (_socket.is_open()) {
                asio::post(_io_context, [this]() { _socket.close(); });
            }
        }

        asio::ip::tcp::socket &socket() {
            return _socket;
        }

        /// @details Asynchronous function
        void sendMsg(const Message &msg) {
            asio::post(_io_context,
                       [this, msg]() {

                           bool writeInProcess = !_msg_queue_out.empty();
                           _msg_queue_out.push_back(msg);
                           if (!writeInProcess)
                               writeHeader();
                       });
        }

        /// @details Asynchronous function
        void sendMsg(Message &&msg) {
            asio::post(_io_context,
                       [this, msg]() {
                           bool writeInProcess = !_msg_queue_out.empty();
                           _msg_queue_out.push_back(msg);
                           if (!writeInProcess)
                               writeHeader();
                       });
        }

        void sendFile(const bfs::path &root, const bfs::path &relPath) {
            if (!exists(root / relPath) || !is_regular_file(root / relPath)) {
                log() << "File \'" << (root / relPath).string() << "\' not found";
                return;
            }

            writeFileHeader(root, relPath);
        }

        void writeFileHeader(const bfs::path &root, const bfs::path &relPath) {
            auto file_size = bfs::file_size(root / relPath);
            auto body{relPath.string() + "\n" + std::to_string(file_size)};
            Message msg{Message::MessageHeader{MsgType::FileHeader, MAX_BODY_SIZE},
                        std::move(body)};
            sendMsg(std::move(msg));

            writeFileBody(root / relPath);
        }

        void writeFileBody(const bfs::path &path) {
            std::ifstream ifs{path.string()};

            if (!ifs.is_open()) {
                log() << "File cannot be opened.";
                return;
            }

            std::string buffer;
            buffer.resize(MAX_BODY_SIZE);
            Message msg{Message::MessageHeader{MsgType::FileTransfer, MAX_BODY_SIZE}};

            while (ifs) {
                ifs.read(buffer.data(), MAX_BODY_SIZE);
                msg.body() = buffer;
                std::fill(buffer.begin(), buffer.end(), '\0');
                sendMsg(msg);
            }
        }

        /// @details Asynchronous function
        void readHeader() {
            asio::async_read(_socket,
                             asio::buffer(&_tempMsgIn.header(), net::HEADER_SIZE),
                             [this](system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     log() << "Read Header Done";
                                     if (_tempMsgIn.bodyLength() > 0) {
                                         _tempMsgIn.resize(_tempMsgIn.bodyLength());
                                         readBody();
                                     } else {
                                         _msg_queue_in.push_back(_tempMsgIn);
                                         readHeader();
                                     }
                                 } else {
                                     log() << "Read Header Fail";
                                     _socket.close();
                                     if (_thr.joinable())
                                         _thr.join();
                                     else
                                         log() << "Not joinable!";
                                 }
                             });
        }

        /// @details Asynchronous function
        void readBody() {
            asio::async_read(_socket,
                             asio::buffer(_tempMsgIn.data(), _tempMsgIn.bodyLength()),
                             [this](system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     log() << "Read Body Done";
                                     _msg_queue_in.push_back(_tempMsgIn);
                                     readHeader();
                                 } else {
                                     log() << "Read Body Fail";
                                     _socket.close();
                                 }
                             });
        }

        /// @details Asynchronous function
        void writeHeader() {
            asio::async_write(_socket,
                              asio::buffer(&_msg_queue_out.front().header(),
                                           net::HEADER_SIZE),
                              [this](system::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      log() << "Write Header Done"
                                                << " with length = " << length;
                                      if (_msg_queue_out.front().bodyLength() > 0) {
                                          writeBody();
                                      } else {
                                          _msg_queue_out.pop_front();
                                          if (!_msg_queue_out.empty())
                                              writeHeader();
                                      }
                                  } else {
                                      log() << "Write Header Fail";
                                      _socket.close();
                                  }
                              });
        }

        /// @details Asynchronous function
        void writeBody() {
            asio::async_write(_socket,
                              asio::buffer(_msg_queue_out.front().data(),
                                           _msg_queue_out.front().bodyLength()),
                              [this](system::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      log() << "Write Body Done"
                                                << " with length = " << length;
                                      _msg_queue_out.pop_front();
                                      if (!_msg_queue_out.empty())
                                          writeHeader();
                                  } else {
                                      log() << "Write Body Fail";
                                      _socket.close();
                                  }
                              });
        }

        void setOnMessageHandler(std::function<void(const Message &)> onMessageHandler) {
            _onMessageHandler = std::move(onMessageHandler);
        }

        void resetOnMessageHandler() {
            _onMessageHandler = nullptr;
        }

        void processIncoming() {
            _thr = std::thread([this]() { _processIncoming(); });
        }

        [[noreturn]] void _processIncoming() {
            while (true) {
                _msg_queue_in.wait();
                auto msg = _msg_queue_in.pop_front();

                if (_onMessageHandler)
                    _onMessageHandler(msg);
                else
                    log() << "Have no handler";
            }
        }

    private:
        ts_deque<Message> _msg_queue_in;
        ts_deque<Message> _msg_queue_out;
        Message _tempMsgIn;
        asio::ip::tcp::socket _socket;
        asio::io_context &_io_context;
        std::function<void(const Message &)> _onMessageHandler;
        std::thread _thr;
        logger::Logger log{"Connection"};
    };
}

#endif //NETWORKING_CONNECTION_HPP