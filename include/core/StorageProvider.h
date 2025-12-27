#pragma once

#include <QString>
#include <QDir>

namespace gwt {
namespace core {

/**
 * @brief Provides platform-specific storage paths for repository management
 * 
 * Windows: %APPDATA%\GithubWorkflowTool\repos\
 * Linux: $XDG_DATA_HOME/githubworkflowtool/repos/ or ~/.local/share/githubworkflowtool/repos/
 * Cache: $XDG_CACHE_HOME/githubworkflowtool/ or ~/.cache/githubworkflowtool/
 */
class StorageProvider {
public:
    /**
     * @brief Get the singleton instance
     */
    static StorageProvider& instance();

    /**
     * @brief Get the root directory for repository storage
     * @return Path to repository storage root
     */
    QString getRepoStorageRoot() const;

    /**
     * @brief Get the cache directory
     * @return Path to cache directory
     */
    QString getCacheRoot() const;

    /**
     * @brief Get the directory for a specific repository
     * @param repoUrl The repository URL
     * @return Path to the repository's local storage
     */
    QString getRepoDirectory(const QString& repoUrl) const;

    /**
     * @brief Ensure storage directories exist
     * @return true if directories were created or already exist
     */
    bool ensureDirectoriesExist();

private:
    StorageProvider();
    ~StorageProvider() = default;
    StorageProvider(const StorageProvider&) = delete;
    StorageProvider& operator=(const StorageProvider&) = delete;

    /**
     * @brief Generate a normalized repository key from URL
     * @param repoUrl The repository URL
     * @return Normalized key (host/owner/name + hash)
     */
    QString generateRepoKey(const QString& repoUrl) const;

    QString m_repoRoot;
    QString m_cacheRoot;
};

} // namespace core
} // namespace gwt
