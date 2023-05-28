#ifndef GRADUATEWORK_CLI_HPP
#define GRADUATEWORK_CLI_HPP

#include "../pch.h"

namespace cli {
    std::queue<std::string> split(const std::string &s) {
        std::queue<std::string> result;
        bool quoted{false};

        auto start{0};
        auto end{0};
        while (end < s.size()) {
            if (s.at(end) == ' ' && !quoted) {
                result.emplace(s.substr(start, end - start));
                start = end + 1;
                end = start;
                continue;
            }
            if (s.at(end) == '\"') {
                if (quoted) {
                    ++start;
                    result.emplace(s.substr(start, end - start));
                    quoted = false;
                    start = end + 2;
                    end = start;
                    continue;
                } else {
                    quoted = true;
                }
            }
            ++end;
        }
        if (start < s.size())
            result.emplace(s.substr(start, s.size()));

        if (quoted) {
            std::cerr << "Illegal command" << std::endl;
            throw std::runtime_error("Illegal command");
        }

        return result;
    }

    class CLI {
    public:
        void addBranch(const std::string &cmd, const std::function<void(std::string)> &handler) {
            _handlers_map.emplace(cmd, handler);
        }

        int resolveArgs(int argc, char **argv) {
            if (argc == 1)
                return 0;

            std::queue<std::string> args;
            for (int i = 1; i < argc; ++i)
                args.emplace(argv[i]);

            return resolveArgs(args);
        }

        int resolveArgs(std::queue<std::string> args) {
            if (args.size() == 1 && args.front() == "exit")
                return -1;

            // Verify command pattern
            if (args.size() != 2) {
                std::cerr << "Illegal command pattern size. "
                          << "Args count=" << args.size() << std::endl;
                return 1;
            }

            // Extract command
            auto arg_cmd = std::move(args.front());
            args.pop();
            if (_handlers_map.find(arg_cmd) == _handlers_map.end()) {
                std::cerr << "Illegal command \'" << arg_cmd << '\'' << std::endl;
                return 1;
            }

            // Extract command argument
            auto arg_path = std::move(args.front());
            args.pop();
            std::replace(arg_path.begin(), arg_path.end(), '\\', '/');

            // Call the appropriate handler
            _handlers_map.at(arg_cmd).operator()(arg_path);
            return 0;
        }

    private:
        std::unordered_map<std::string, std::function<void(std::string)>> _handlers_map;
    };
}

#endif //GRADUATEWORK_CLI_HPP