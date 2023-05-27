#include "cli/CLI.hpp"
#include "version controller/VersionController.hpp"

static_assert(sizeof(uint8_t) == 1);

/*
 * char - unsigned by default
 * 0 - signed
 * 1 - unsigned
 * */

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);

    std::string storagePath;
    bpo::options_description config("Config");
    config.add_options()
            ("storagePath", bpo::value<std::string>(&storagePath));
    bpo::variables_map variables_map;
    bpo::store(bpo::parse_config_file("../Client.cfg", config), variables_map);
    variables_map.notify();

    if (!bfs::exists(storagePath) || !bfs::is_directory(storagePath)) {
        std::cerr << "Path to storage \'" << storagePath << "\' does not exist!" << std::endl;
        throw std::runtime_error("Path to storage does not exist!");
    }

    bfs::create_directory(storagePath + "/.versions");

    vcs::VersionController version_controller{storagePath};
    cli::CLI cli;
    cli.addBranch("add", [&](const bfs::path &path) { version_controller.add(path); });
    cli.addBranch("update", [&](const bfs::path &path) { version_controller.update(path); });
    cli.addBranch("restore", [&](const bfs::path &path) { version_controller.restore(path); });
    cli.addBranch("observe", [&](const bfs::path &path) { version_controller.addObservable(path); });
    cli.addBranch("checkIntegrity", [&](const bfs::path &path) { version_controller.check_integrity(path); });
    cli.resolveArgs(argc, argv);

    // Going into endless cli loop
    std::string buffer;
    while (true) {
//        std::cout << "(cli) ";
//        std::cout.flush();
        std::getline(std::cin, buffer);
        if (cli.resolveArgs(cli::split(buffer)) == -1)
            break;
    }

    return 0;
}