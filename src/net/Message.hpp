#ifndef NETWORKING_MESSAGE_HPP
#define NETWORKING_MESSAGE_HPP

#include <utility>

#include "../pch.h"

namespace net {
    void print(std::ostream &os, const std::string &str, bool verbose = false) {
        os << "Message body:" << '\n';
        os << '\t';
        os << '\'';
        if (!verbose)
            os << str;
        else
            for (auto ch: str) {
                switch (ch) {
                    case '\0': {
                        os << "\\0";
                        break;
                    }
                    default:
                        os << ch;
                }
            }
        os << '\'';
    }

    enum class MsgType {
        PlainText,
        EmptyMessage,
        FileHeader,
        FileTransfer,
        RestoreVersion,
        CheckIntegrity,
        RequestFiles,
        RequestedFiles
    };

    constexpr const char *to_string(MsgType msgType) {
        switch (msgType) {
            case MsgType::PlainText:
                return "PlainText";
            case MsgType::EmptyMessage:
                return "EmptyMessage";
            case MsgType::FileHeader:
                return "FileHeader";
            case MsgType::FileTransfer:
                return "FileTransfer";
            case MsgType::RestoreVersion:
                return "RestoreVersion";
            case MsgType::CheckIntegrity:
                return "CheckIntegrity";
            case MsgType::RequestFiles:
                return "RequestFiles";
            case MsgType::RequestedFiles:
                return "RequestedFiles";
            default:
                throw std::runtime_error("Has no to_string cast for this enumeration item!");
        }
    }

    class Message {
    public:
        using bodyLength_type = size_t;

        class MessageHeader {
        public:
            MessageHeader() = default;

            explicit MessageHeader(MsgType msgType) :
                    _msg_type(msgType) {
            }

            explicit MessageHeader(bodyLength_type bodyLength) :
                    _body_length(bodyLength) {
            }

            explicit MessageHeader(MsgType msgType, bodyLength_type bodyLength) :
                    _msg_type(msgType), _body_length(bodyLength) {
            }

            MessageHeader(const MessageHeader &msgHeader) = default;

            MessageHeader(MessageHeader &&msgHeader) noexcept:
                    _msg_type(msgHeader._msg_type), _body_length(msgHeader._body_length) {
                msgHeader._msg_type = MsgType::EmptyMessage;
                msgHeader._body_length = 0;

            }

            MessageHeader &operator=(const MessageHeader &msgHeader) = default;

            [[nodiscard]] bodyLength_type bodyLength() const {
                return _body_length;
            }

            void bodyLength(bodyLength_type bodyLength) {
                _body_length = bodyLength;
            }

            [[nodiscard]] MsgType msgType() const {
                return _msg_type;
            }

            void msgType(MsgType msgType) {
                _msg_type = msgType;
            }

            friend std::ostream &operator<<(std::ostream &os, const MessageHeader &msgHeader) {
                os << "Message header:";
                os << "\n\tMsgType: " << to_string(msgHeader._msg_type);
                os << "\n\tBody length: " << msgHeader._body_length;
                os << '\n';

                return os;
            }

        private:
            MsgType _msg_type{MsgType::EmptyMessage};
            bodyLength_type _body_length{0};
        };

        Message() = default;

        explicit Message(MessageHeader msgHeader, std::string body = "") :
                _header(std::move(msgHeader)), _body(std::move(body)) {
            if (!_body.empty())
                _header.bodyLength(_body.size());
        }

        Message(const Message &msg) = default;

        Message(Message &&msg) noexcept:
                _header(std::move(msg._header)), _body(std::move(msg._body)) {
        }

        explicit Message(MsgType msgType, std::string body = "") :
                Message(MessageHeader{msgType}, std::move(body)) {

        }

        Message &operator=(const Message &msg) = default;

        MessageHeader &header() {
            return _header;
        }

        [[nodiscard]] const MessageHeader &header() const {
            return _header;
        }

        char *data() {
            return _body.data();
        }

        [[nodiscard]] const char *data() const {
            return _body.data();
        }

        void body(std::string body) {
            _body = std::move(body);
            _header.bodyLength(_body.size());
        }

        [[nodiscard]] const std::string &body() const {
            return _body;
        }

        [[nodiscard]] bodyLength_type bodyLength() const {
            return _header.bodyLength();
        }

        void resize(size_t bodyLength) {
            _body.resize(bodyLength);
        }

        friend std::ostream &operator<<(std::ostream &os, const Message &msg) {
            os << msg._header;
            print(os, msg._body);

            return os;
        }

    private:
        MessageHeader _header;
        std::string _body;
    };

    constexpr size_t HEADER_SIZE{sizeof(Message::MessageHeader)};
}

#endif //NETWORKING_MESSAGE_HPP