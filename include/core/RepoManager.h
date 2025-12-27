#pragma once

#include <QString>
#include <QStringList>
#include <QObject>
#include <memory>

namespace gwt {
namespace core {

class StorageProvider;

/**
 * @brief Manages Git repository cloning and operations
 */
class RepoManager : public QObject {
    Q_OBJECT

public:
    explicit RepoManager(QObject* parent = nullptr);
    ~RepoManager() override;

    /**
     * @brief Clone a repository to the local storage
     * @param repoUrl The repository URL (e.g., https://github.com/owner/repo)
     * @param branch Optional branch to clone (default: main/master)
     * @return true if successful
     */
    bool cloneRepository(const QString& repoUrl, const QString& branch = QString());

    /**
     * @brief Update an existing repository
     * @param repoUrl The repository URL
     * @return true if successful
     */
    bool updateRepository(const QString& repoUrl);

    /**
     * @brief Get the local path for a repository
     * @param repoUrl The repository URL
     * @return Local file system path
     */
    QString getLocalPath(const QString& repoUrl) const;

    /**
     * @brief Check if a repository is already cloned
     * @param repoUrl The repository URL
     * @return true if already cloned
     */
    bool isCloned(const QString& repoUrl) const;

    /**
     * @brief List all cloned repositories
     * @return List of repository URLs
     */
    QStringList listRepositories() const;

signals:
    void cloneProgress(int percentage, const QString& message);
    void cloneFinished(bool success);
    void error(const QString& errorMessage);

private:
    StorageProvider& m_storage;
};

} // namespace core
} // namespace gwt
