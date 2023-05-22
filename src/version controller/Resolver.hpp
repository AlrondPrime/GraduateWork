#ifndef GRADUATEWORK_RESOLVER_HPP
#define GRADUATEWORK_RESOLVER_HPP

#include "PathResolver.hpp"

namespace {
    std::string versions_folder{R"(C:\Users\alrondprime\CLionProjects\GraduateWork\ClientStorage\versions)"};
    std::string storage_folder{R"(C:\Users\alrondprime\CLionProjects\GraduateWork\ClientStorage)"};
}

const vcs::FileResolver fileResolver{storage_folder, versions_folder};
const vcs::VersionResolver versionResolver{storage_folder, versions_folder};

#endif //GRADUATEWORK_RESOLVER_HPP