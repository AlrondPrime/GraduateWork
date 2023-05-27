#include "net/Server.hpp"

int main() {
    std::ios_base::sync_with_stdio(false);

    std::string storagePath;
    bpo::options_description config("Config");
    config.add_options()
            ("storagePath", bpo::value<std::string>(&storagePath));
    bpo::variables_map variables_map;
    bpo::store(bpo::parse_config_file("../Server.cfg", config), variables_map);
    variables_map.notify();

    if (!bfs::exists(storagePath) || !bfs::is_directory(storagePath)) {
        std::cerr << "Path to storage \'" << storagePath << "\' does not exist!" << std::endl;
        throw std::runtime_error("Path to storage does not exist!");
    }

    bfs::create_directory(storagePath + "/.versions");

    uint16_t port{60000};
    net::Server server{port};
    server.root(storagePath);
    server.Start();
    server.mainLoop();

    return 0;
}