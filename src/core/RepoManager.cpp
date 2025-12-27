#include "core/RepoManager.h"
#include "core/StorageProvider.h"
#include <QProcess>
#include <QDir>
#include <QDebug>

namespace gwt {
namespace core {

RepoManager::RepoManager(QObject* parent)
    : QObject(parent)
    , m_storage(StorageProvider::instance())
{
}

RepoManager::~RepoManager() = default;

bool RepoManager::cloneRepository(const QString& repoUrl, const QString& branch) {
    QString localPath = m_storage.getRepoDirectory(repoUrl);
    
    if (isCloned(repoUrl)) {
        emit error("Repository already cloned at: " + localPath);
        return false;
    }
    
    QDir().mkpath(QFileInfo(localPath).path());
    
    QProcess git;
    QStringList args;
    args << "clone";
    
    if (!branch.isEmpty()) {
        args << "--branch" << branch;
    }
    
    args << repoUrl << localPath;
    
    git.start("git", args);
    
    if (!git.waitForStarted()) {
        emit error("Failed to start git process");
        return false;
    }
    
    emit cloneProgress(10, "Cloning repository...");
    
    // Wait for completion with timeout
    if (!git.waitForFinished(300000)) { // 5 minutes timeout
        emit error("Git clone timeout");
        git.kill();
        return false;
    }
    
    if (git.exitCode() != 0) {
        QString errorMsg = QString::fromUtf8(git.readAllStandardError());
        emit error("Git clone failed: " + errorMsg);
        return false;
    }
    
    emit cloneProgress(100, "Clone completed");
    emit cloneFinished(true);
    return true;
}

bool RepoManager::updateRepository(const QString& repoUrl) {
    QString localPath = getLocalPath(repoUrl);
    
    if (!isCloned(repoUrl)) {
        emit error("Repository not cloned: " + repoUrl);
        return false;
    }
    
    QProcess git;
    git.setWorkingDirectory(localPath);
    git.start("git", QStringList() << "pull");
    
    if (!git.waitForFinished(60000)) { // 1 minute timeout
        emit error("Git pull timeout");
        return false;
    }
    
    if (git.exitCode() != 0) {
        QString errorMsg = QString::fromUtf8(git.readAllStandardError());
        emit error("Git pull failed: " + errorMsg);
        return false;
    }
    
    return true;
}

QString RepoManager::getLocalPath(const QString& repoUrl) const {
    return m_storage.getRepoDirectory(repoUrl);
}

bool RepoManager::isCloned(const QString& repoUrl) const {
    QString localPath = getLocalPath(repoUrl);
    QDir dir(localPath);
    return dir.exists() && dir.exists(".git");
}

QStringList RepoManager::listRepositories() const {
    QStringList repos;
    QString root = m_storage.getRepoStorageRoot();
    
    QDir rootDir(root);
    if (!rootDir.exists()) {
        return repos;
    }
    
    // Recursively find all directories with .git
    QFileInfoList entries = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& entry : entries) {
        QDir hostDir(entry.filePath());
        QFileInfoList repoEntries = hostDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& repoEntry : repoEntries) {
            if (QDir(repoEntry.filePath()).exists(".git")) {
                repos << repoEntry.filePath();
            }
        }
    }
    
    return repos;
}

} // namespace core
} // namespace gwt
