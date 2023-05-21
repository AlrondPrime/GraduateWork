#ifndef GRADUATEWORK_CLI_HPP
#define GRADUATEWORK_CLI_HPP

#include "../pch.h"
#include "../Log.hpp"
#include "../version controller/Resolver.hpp"

namespace cli {
    std::queue<std::string> split(const std::string &s, char delimiter) {
    std::queue<std::string> result;
    std::string token;

    size_t start{0};
    size_t end{s.find(delimiter)};
    while (end != std::string::npos) {
        result.emplace(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delimiter, start);
    }
    result.emplace(s.substr(start, end - start));

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