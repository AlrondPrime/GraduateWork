#ifndef GRADUATEWORK_LOGGER_HPP
#define GRADUATEWORK_LOGGER_HPP

#include "../pch.h"
#include "../net/Message.hpp"

enum class log {
    endl,
};


namespace logger {
    class LogSession {
    public:
        explicit LogSession(const std::string &scope) : _scope(scope) {
            _ss << "[" << _scope << "] ";
        }

        explicit LogSession(const LogSession &session) : _scope(session._scope) {
//            _ss << _scope;
        }

        explicit LogSession(LogSession &&session) : _scope(session._scope) {
//            _ss << _scope;
        }

        virtual ~LogSession() {
            std::scoped_lock lock(_mutex);
            std::cout << _ss.str() << std::endl;
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

    private:
        std::stringstream _ss;
        const std::string &_scope;
        std::mutex _mutex;
    };

    class Logger {
    public:
        Logger() = default;

        explicit Logger(std::string scope) : _scope(std::move(scope)) {}

        LogSession operator()() {
            return LogSession{_scope};
        }

    private:
        std::string _scope;
    };

    /*class Logger {
        public:
            Logger() = default;

            explicit Logger(std::string scope) : _scope(std::move(scope)) {}

            friend Logger &operator<<(Logger &logger, const std::string &msg) {
                std::scoped_lock lock(logger._mutex);
                std::cout << msg;
                return logger;
            }

            friend Logger &operator<<(Logger &logger, const net::Message &msg) {
                std::scoped_lock lock(logger._mutex);
                std::cout << msg;
                return logger;
            }

            friend Logger &operator<<(Logger &logger, size_t val) {
                std::scoped_lock lock(logger._mutex);
                std::cout << val;
                return logger;
            }

            friend Logger &operator<<(Logger &logger, enum log log) {
                std::scoped_lock lock(logger._mutex);
                std::cout << std::endl;
                return logger;
            }

        private:
            std::string _scope;
            std::mutex _mutex;
        };*/
}

#endif //GRADUATEWORK_LOGGER_HPP