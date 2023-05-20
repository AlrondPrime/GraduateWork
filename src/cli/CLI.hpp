#ifndef GRADUATEWORK_CLI_HPP
#define GRADUATEWORK_CLI_HPP

#include "../pch.h"
#include "../Log.hpp"
#include "../version controller/Resolver.hpp"

namespace cli {
    class CLI {
    public:
        CLI(int argc, char **argv) {
            for (int i = 1; i < argc; ++i) {
                _args.emplace(argv[i]);
            }
        }

        void addBranch(const std::string &cmd, const std::function<void(std::string)> &handler) {
            _handlers_map.emplace(cmd, handler);
        }

        void resolveArgs() {
            // Verify command pattern
            if (_args.size() != 2) {
                std::cerr << "Illegal command pattern" << std::endl;
                return;
            }

            // Verify command
            auto arg_cmd = std::move(_args.front());
            _args.pop();
            if (_handlers_map.find(arg_cmd) == _handlers_map.end()) {
                std::cerr << "Illegal command \'" << arg_cmd << '\'' << std::endl;
                return;
            }

            // Verify command argument
            auto arg_path = std::move(_args.front());
            _args.pop();

//            if (!boost::filesystem::exists(storage_resolver.root() / arg_path) &&
//                !boost::filesystem::exists(copy_resolver.resolve(arg_path)) ||
//                !boost::filesystem::is_regular_file(storage_resolver.root() / arg_path) &&
//                !boost::filesystem::is_regular_file(copy_resolver.resolve(arg_path))) {
//                std::cerr << "Illegal path \'" << storage_resolver.root() / arg_path << '\'' << std::endl;
//                std::cerr << "Illegal path \'" << copy_resolver.resolve(arg_path) << '\'' << std::endl;
//                return;
//            }

            std::replace(arg_path.begin(), arg_path.end(), '\\', '/');

            // Call the appropriate handler
            _handlers_map.at(arg_cmd).operator()(arg_path);
        }

    private:
        std::unordered_map<std::string, std::function<void(std::string)>> _handlers_map;
        std::queue<std::string> _args;
    };
}

#endif //GRADUATEWORK_CLI_HPP