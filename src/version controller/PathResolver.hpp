#ifndef GRADUATEWORK_PATHRESOLVER_HPP
#define GRADUATEWORK_PATHRESOLVER_HPP

#include <utility>

#include "../pch.h"
#include "../Log.hpp"

namespace vcs {
    class PathResolver {
    public:
        explicit PathResolver(bfs::path storageDir, bfs::path versionsDir) :
                _storage_dir(std::move(storageDir)),
                _versions_dir(std::move(versionsDir)) {}

        virtual bfs::path root() {
            return _storage_dir;
        }

        virtual ~PathResolver() = default;

    protected:
        bfs::path _storage_dir;
        bfs::path _versions_dir;
    };

    class FileResolver {
    public:
        explicit FileResolver(bfs::path storageDir, bfs::path versionsDir) :
                _storage_dir(std::move(storageDir)),
                _versions_dir(std::move(versionsDir)) {
        }

        bfs::path storage(const bfs::path &pathToFile) const {
            return _storage_dir / pathToFile;
        }

        bfs::path copy(const bfs::path &pathToFile) const {
            return _versions_dir / pathToFile / pathToFile.filename();
        }

        bfs::path versionsFolder(const bfs::path &pathToFile) const {
            return _versions_dir / pathToFile;
        }

    private:
        bfs::path _storage_dir;
        bfs::path _versions_dir;
    };

    class VersionResolver {
    public:
        explicit VersionResolver(bfs::path storageDir, bfs::path versionsDir) :
                _storage_dir(std::move(storageDir)),
                _versions_dir(std::move(versionsDir)) {
        }

        bfs::path storage(const bfs::path &pathToVersion) const {
            return _storage_dir / pathToVersion.parent_path();
        }

        bfs::path copy(const bfs::path &pathToVersion) const {
            return _versions_dir / pathToVersion.parent_path() / pathToVersion.parent_path().filename();
        }

        bfs::path versionsFolder(const bfs::path &pathToVersion) const {
            return _versions_dir / pathToVersion.parent_path();
        }

    private:
        bfs::path _storage_dir;
        bfs::path _versions_dir;
    };
}

#endif //GRADUATEWORK_PATHRESOLVER_HPP