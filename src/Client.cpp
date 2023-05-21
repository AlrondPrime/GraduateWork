
#include "cli/CLI.hpp"
#include "version controller/VersionController.hpp"

static_assert(sizeof(uint8_t) == 1, "");

/*
 * char - unsigned by default
 * 0 - signed
 * 1 - unsigned
 * */

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);

    // Set working directory to the project's root directory
    boost::filesystem::current_path(boost::filesystem::current_path().parent_path());

    vcs::VersionController version_controller;
    cli::CLI cli;
    cli.addBranch("add", [&](const bfs::path &path) { version_controller.add(path); });
    cli.addBranch("update", [&](const bfs::path &path) { version_controller.update(path); });
    cli.addBranch("restore", [&](const bfs::path &path) { version_controller.restore(path); });
    cli.resolveArgs(argc, argv);

    // Going into endless cli loop
    std::string buffer;
    while (true) {
        std::cout << "(cli) ";
        std::cout.flush();
        std::getline(std::cin, buffer);
        if (cli.resolveArgs(cli::split(buffer, ' ')) == -1)
            break;
    }

    return 0;
}