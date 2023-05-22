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

        virtual ~PathResolver() = default;

        [[nodiscard]] virtual bfs::path main(const bfs::path &) const = 0;
        [[nodiscard]] virtual bfs::path copy(const bfs::path &) const = 0;
        [[nodiscard]] virtual bfs::path fileVersions(const bfs::path &) const = 0;

        virtual bfs::path storage() const{
            return _storage_dir;
        }

        virtual bfs::path versions() const{
            return _versions_dir;
        }

    protected:
        bfs::path _storage_dir;
        bfs::path _versions_dir;
    };

    class FileResolver : public PathResolver{
    public:
        FileResolver(bfs::path storageDir, bfs::path versionsDir) :
                PathResolver(std::move(storageDir), std::move(versionsDir))
//                _storage_dir(std::move(storageDir)),
//                _versions_dir(std::move(versionsDir))
        {
        }

        [[nodiscard]] bfs::path main(const bfs::path &pathToFile) const override {
            return _storage_dir / pathToFile;
        }

        [[nodiscard]] bfs::path copy(const bfs::path &pathToFile) const override {
            return _versions_dir / pathToFile / pathToFile.filename();
        }

        [[nodiscard]] bfs::path fileVersions(const bfs::path &pathToFile) const override {
            return _versions_dir / pathToFile;
        }

    private:
//        bfs::path _storage_dir;
//        bfs::path _versions_dir;
    };

    class VersionResolver : public PathResolver{
    public:
        explicit VersionResolver(bfs::path storageDir, bfs::path versionsDir) :
                 PathResolver(std::move(storageDir), std::move(versionsDir))
//                _storage_dir(std::move(storageDir)),
//                _versions_dir(std::move(versionsDir))
                {
        }

        [[nodiscard]] bfs::path main(const bfs::path &pathToVersion) const override {
            return _storage_dir / pathToVersion.parent_path();
        }

        [[nodiscard]] bfs::path copy(const bfs::path &pathToVersion) const override {
            return _versions_dir / pathToVersion.parent_path() / pathToVersion.parent_path().filename();
        }

        [[nodiscard]] bfs::path fileVersions(const bfs::path &pathToVersion) const override {
            return _versions_dir / pathToVersion.parent_path();
        }

    private:
//        bfs::path _storage_dir;
//        bfs::path _versions_dir;
    };
}

#endif //GRADUATEWORK_PATHRESOLVER_HPP