#ifndef GRADUATEWORK_PATHRESOLVER_HPP
#define GRADUATEWORK_PATHRESOLVER_HPP

#include "../pch.h"

namespace vcs {
    class PathResolver {
    public:
        explicit PathResolver(bfs::path storageDir) :
                _storage_dir(std::move(storageDir)),
                _versions_dir(_storage_dir.string() + "/.versions") {
            assert(bfs::exists(_storage_dir) && bfs::is_directory(_storage_dir));
            assert(bfs::exists(_versions_dir) && bfs::is_directory(_versions_dir));
        }

        virtual ~PathResolver() = default;

        [[nodiscard]] virtual bfs::path main(const bfs::path &) const = 0;

        [[nodiscard]] virtual bfs::path copy(const bfs::path &) const = 0;

        [[nodiscard]] virtual bfs::path fileVersionsFolder(const bfs::path &) const = 0;

        [[nodiscard]] virtual bfs::path storageDir() const {
            return _storage_dir;
        }

        [[nodiscard]] virtual bfs::path versionsDir() const {
            return _versions_dir;
        }

    protected:
        bfs::path _storage_dir;
        bfs::path _versions_dir;
    };

    class FileResolver : public PathResolver {
    public:
        explicit FileResolver(bfs::path storageDir) :
                PathResolver(std::move(storageDir)) {
        }

        [[nodiscard]] bfs::path main(const bfs::path &pathToFile) const override {
            return _storage_dir / pathToFile;
        }

        [[nodiscard]] bfs::path copy(const bfs::path &pathToFile) const override {
            return _versions_dir / pathToFile / pathToFile.filename();
        }

        [[nodiscard]] bfs::path fileVersionsFolder(const bfs::path &pathToFile) const override {
            return _versions_dir / pathToFile;
        }
    };

    class VersionResolver : public PathResolver {
    public:
        explicit VersionResolver(bfs::path storageDir) :
                PathResolver(std::move(storageDir)) {
        }

        [[nodiscard]] bfs::path main(const bfs::path &pathToVersion) const override {
            return _storage_dir / pathToVersion.parent_path();
        }

        [[nodiscard]] bfs::path copy(const bfs::path &pathToVersion) const override {
            return _versions_dir / pathToVersion.parent_path() / pathToVersion.parent_path().filename();
        }

        [[nodiscard]] bfs::path fileVersionsFolder(const bfs::path &pathToVersion) const override {
            return _versions_dir / pathToVersion.parent_path();
        }
    };
}

#endif //GRADUATEWORK_PATHRESOLVER_HPP