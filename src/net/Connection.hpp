#ifndef NETWORKING_CONNECTION_HPP
#define NETWORKING_CONNECTION_HPP

#include <utility>

#include "../pch.h"
#include "../logger/Logger.hpp"
#include "ts_deque.hpp"

namespace net {
    struct FileInProcess {
        bfs::path _path{};
        std::ofstream _ofs{};
//        uintmax_t _file_size{};
        time_t _modification_time{};
    };

    class Connection :
            public std::enable_shared_from_this<Connection> {
    public:
        explicit Connection(ba::ip::tcp::socket socket, ba::io_context &io_context, const bfs::path &rootDir) :
                _socket(std::move(socket)), _io_context(io_context), _root_dir(rootDir) {
        }

        bool connected() const {
            return _socket.is_open();
        }

        /// @details Asynchronous function
        void disconnect() {
            if (_socket.is_open()) {
                ba::post(_io_context, [this]() { _socket.close(); });
            }
        }

        ba::ip::tcp::socket &socket() {
            return _socket;
        }

        /// @details Asynchronous function
        void sendMsg(const Message &msg) {
            log() << "Sending message:\n" << msg;
            ba::post(_io_context,
                     [this, msg]() {

                         bool writeInProcess = !_msg_queue_out.empty();
                         _msg_queue_out.push_back(msg);
                         if (!writeInProcess)
                             writeHeader();
                     });
        }

        /// @details Asynchronous function
        void sendMsg(Message &&msg) {
            log() << "Sending message:\n" << msg;
            ba::post(_io_context,
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

        void setOnMessageHandler(std::function<void(const Message &)> onMessageHandler) {
            _onMessageHandler = std::move(onMessageHandler);
        }

        void resetOnMessageHandler() {
            _onMessageHandler = nullptr;
        }

        void processIncoming() {
            _thr = std::thread([this]() { _processIncoming(); });
        }

    private:
        void writeFileHeader(const bfs::path &root, const bfs::path &relPath) {
//            auto file_size = bfs::file_size(root / relPath);
            auto modification_time = bfs::last_write_time(root / relPath);
            bj::object object{
                    {"path",              relPath.string()},
//                    {"file_size",         file_size},
                    {"modification_time", modification_time}
            };
            auto body{serialize(object)};
            Message msg{MsgType::FileHeader, std::move(body)};
            sendMsg(std::move(msg));

            writeFileBody(root, relPath);
        }

        void writeFileBody(const bfs::path &root, const bfs::path &relPath) {
            std::ifstream ifs{(root / relPath).string()};

            if (!ifs.is_open()) {
                log() << "File cannot be opened.";
                return;
            }

            std::string buffer;
            constexpr std::streamsize buffer_size{1024};
            bj::object body;
            Message msg{MsgType::FileTransfer};

            while (ifs) {
                buffer.resize(buffer_size);
                ifs.read(buffer.data(), buffer_size);
                buffer.resize(ifs.gcount());
                body.emplace("path", relPath.string());
                body.emplace("data", buffer);
                assert(body.size() == 2);
                msg.body(bj::serialize(body));
                body.clear();
                sendMsg(msg);
            }

            Message eof_msg{MsgType::EndOfFile};
            body.emplace("path", relPath.string());
            eof_msg.body(bj::serialize(body));
            body.clear();
            sendMsg(eof_msg);
        }

    public:
        /// @details Asynchronous function
        void readHeader() {
            ba::async_read(_socket,
                           ba::buffer(&_tempMsgIn.header(), net::HEADER_SIZE),
                           [this](boost::system::error_code ec, std::size_t length) {
                               if (!ec) {
//                                   log() << "Read Header Done";
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

    private:
        /// @details Asynchronous function
        void readBody() {
            ba::async_read(_socket,
                           ba::buffer(_tempMsgIn.data(), _tempMsgIn.bodyLength()),
                           [this](boost::system::error_code ec, std::size_t length) {
                               if (!ec) {
//                                   log() << "Read Body Done";
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
            ba::async_write(_socket,
                            ba::buffer(&_msg_queue_out.front().header(),
                                       net::HEADER_SIZE),
                            [this](boost::system::error_code ec, std::size_t length) {
                                if (!ec) {
                                    /*log() << "Write Header Done"
                                          << " with length = " << length;*/
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
            ba::async_write(_socket,
                            ba::buffer(_msg_queue_out.front().data(),
                                       _msg_queue_out.front().bodyLength()),
                            [this](boost::system::error_code ec, std::size_t length) {
                                if (!ec) {
                                    /*log() << "Write Body Done"
                                          << " with length = " << length;*/
                                    _msg_queue_out.pop_front();
                                    if (!_msg_queue_out.empty())
                                        writeHeader();
                                } else {
                                    log() << "Write Body Fail";
                                    _socket.close();
                                }
                            });
        }

        [[noreturn]] void _processIncoming() {
            while (true) {
                _msg_queue_in.wait();
                auto msg = _msg_queue_in.pop_front();

                _msgHandler(msg);
            }
        }

        void _msgHandler(const Message &msg) {
            switch (msg.header().msgType()) {
                case net::MsgType::FileHeader: {
                    log() << "Processing message:\n" << msg;
                    net::FileInProcess fileInProcess;
                    // Try to parse json string got from message body
                    try {
                        bj::object object{bj::parse(msg.body()).as_object()};
                        fileInProcess._path.assign(object.at("path").as_string());
//                        fileInProcess._file_size = static_cast<uintmax_t>(object.at("file_size").as_int64());
                        fileInProcess._modification_time = object.at("modification_time").as_int64();

                    }
                    catch (boost::exception &e) {
                        std::cerr << __FILE__ << "(" << __LINE__ << ")\n";
                        std::cerr << "Can't parse this: \'" << msg.body() << "\'\n";
                        std::cerr << diagnostic_information_what(e) << "\n";
                        return;
                    }

                    // Check if folder for storing current file versions exists
                    if (!bfs::exists((_root_dir / fileInProcess._path).parent_path()))
                        // ..if not, create this folder
                        if (!bfs::create_directories((_root_dir / fileInProcess._path).parent_path())) {
                            log() << "Can't create versions directory";
                            return;
                        }

                    fileInProcess._ofs.open((_root_dir / fileInProcess._path).string());
                    if (!fileInProcess._ofs.is_open()) {
                        log() << "Can't open ofstream";
                        return;
                    }

                    log() << "Opened file on write \'" << (_root_dir / fileInProcess._path).string() << "\'";
                    _files_in_process.push_back(std::move(fileInProcess));

                    break;
                }
                case net::MsgType::FileTransfer: {
                    log() << "Processing message:\n" << msg;
                    bfs::path path;
                    std::string data;

                    try {
                        bj::object object{bj::parse(msg.body()).as_object()};
                        path = object.at("path").as_string();
                        data = object.at("data").as_string();
                    }
                    catch (boost::exception &e) {
                        std::cerr << __FILE__ << "(" << __LINE__ << ")\n";
                        std::cerr << "Can't parse this: \'" << msg << "\'\n";
                        std::cerr << diagnostic_information_what(e) << "\n";
                    }

                    // Search for desired FileInProcess struct and store data in it
                    int index_found{-1};
                    for (int i{}; i < _files_in_process.size(); ++i) {
                        log() << __LINE__;
                        auto &item{_files_in_process.at(i)};
                        if (item._path.string() == path.string()) {
                            index_found = i;
                            if (!item._ofs.is_open()) {
                                err() << "Ofstream for file \'" << (_root_dir / item._path).string()
                                      << "\' isn't opened";
                                return;
                            }

                            item._ofs << data;

                            break;
                        }
                    }

                    if (index_found == -1) {
                        err() << "Can't find Struct for file \'" << path.string() << "\'";
                    }

                    break;
                }
                case MsgType::EndOfFile: {
                    log() << "Processing message:\n" << msg;
                    bfs::path path;

                    try {
                        bj::object object{bj::parse(msg.body()).as_object()};
                        path = object.at("path").as_string();
                    }
                    catch (boost::exception &e) {
                        std::cerr << __FILE__ << "(" << __LINE__ << ")\n";
                        std::cerr << "Can't parse this: \'" << msg << "\'\n";
                        std::cerr << diagnostic_information_what(e) << "\n";
                    }

                    // Search for desired FileInProcess struct and store data in it
                    int index_found{-1};
                    for (int i{}; i < _files_in_process.size(); ++i) {
                        auto &item{_files_in_process.at(i)};
                        if (item._path.string() == path.string()) {
                            index_found = i;
                            if (!item._ofs.is_open()) {
                                err() << "Ofstream for file \'" << (_root_dir / item._path).string()
                                      << "\' isn't opened";
                                return;
                            }

                            log() << "Whole file \'" << (_root_dir / item._path).string() << "\' transferred";
                            item._ofs.close();
                            bfs::last_write_time((_root_dir / item._path), item._modification_time);

                            break;
                        }
                    }

                    if (index_found == -1) {
                        err() << "Can't find Struct for file \'" << path.string() << "\'";
                        for (auto &item: _files_in_process) {
                            log() << "\t\'" << item._path.string() << "\'";
                        }
                    } else {
                        _files_in_process.erase(_files_in_process.begin() + index_found);
                    }

                    break;
                }

                default: {
                    break;
                }
            }


            // If Client or Server app set its own handler, call it
            if (_onMessageHandler)
                _onMessageHandler(msg);
            else
                log() << "Client didn't set any handler";
        }

    public:
        std::vector<FileInProcess> _files_in_process;
    private:
        ts_deque<Message> _msg_queue_in;
        ts_deque<Message> _msg_queue_out;
        Message _tempMsgIn;
        ba::ip::tcp::socket _socket;
        ba::io_context &_io_context;
        std::function<void(const Message &)> _onMessageHandler;
        std::thread _thr;
        logger::Log log{"Connection"};
        logger::Err err{"Connection"};
        bfs::path _root_dir;
    };
}

#endif //NETWORKING_CONNECTION_HPP