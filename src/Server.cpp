#include "net/Server.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: Server.exe <path to folder that will be used as server root>\n";
        return 1;
    }
    uint16_t port{60000};
    std::string root_dir{argv[1]};

    net::Server server{port};
    server.root(root_dir);
    server.Start();
    server.mainLoop();

    return 0;
}