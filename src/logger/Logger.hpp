#ifndef GRADUATEWORK_LOGGER_HPP
#define GRADUATEWORK_LOGGER_HPP

#include "../pch.h"
#include "../net/Message.hpp"

namespace logger {
    class Logger {
    public:
        class LogSession {
        public:
            explicit LogSession(const std::string_view &scope) : _scope(scope) {
                _ss << "[" << _scope << "] ";
            }

            LogSession(const LogSession &session) : _scope(session._scope) {
                _ss << "[" << _scope << "] ";
            }

            LogSession(LogSession &&session) noexcept: _scope(session._scope) {
                _ss << "[" << _scope << "] ";
            }

            virtual ~LogSession() {
                _ss << "\n";
                std::scoped_lock lock(_mutex);
                std::cout << _ss.str();
                std::cout.flush();
            }

            friend LogSession &&operator<<(LogSession &&session, const std::string &msg) {
                session._ss << msg;
                return std::move(session);
            }

            friend LogSession &&operator<<(LogSession &&session, const net::Message &msg) {
                session._ss << msg;
                return std::move(session);
            }

            friend LogSession &&operator<<(LogSession &&session, size_t val) {
                session._ss << val;
                return std::move(session);
            }

            friend LogSession &&operator<<(LogSession &&session, const ba::ip::tcp::endpoint &ep) {
                session._ss << ba::ip::detail::endpoint( ep.address(), ep.port()).to_string();
                return std::move(session);
            }

        private:
            std::stringstream _ss;
            const std::string_view &_scope;
            std::mutex _mutex;
        };

        Logger() = default;

        explicit Logger(std::string_view scope) : _scope(scope) {}

        LogSession operator()() {
            return LogSession{_scope};
        }

    private:
        const std::string_view _scope;
    };
}

#endif //GRADUATEWORK_LOGGER_HPP