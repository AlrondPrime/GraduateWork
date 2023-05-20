#ifndef GRADUATEWORK_LOG_HPP
#define GRADUATEWORK_LOG_HPP

#include "pch.h"

//std::ofstream vcsLog{"../log/vcs.log", std::ios_base::ate};
//std::ofstream multiplyLog{"../log/multiply.log", std::ios_base::ate};

std::ostream &vcsLog{std::cout};
std::ostream &multiplyLog{std::cout};

//bfs::directory_entry versions_dir{"versions"};
//bfs::directory_entry root_dir{"ClientStorage"};

#endif //GRADUATEWORK_LOG_HPP