#include "core/StorageProvider.h"
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace gwt {
namespace core {

StorageProvider& StorageProvider::instance() {
    static StorageProvider instance;
    return instance;
}

StorageProvider::StorageProvider() {
#ifdef Q_OS_WIN
    // Windows: %APPDATA%\GithubWorkflowTool\repos\
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_repoRoot = appData + "/repos";
    m_cacheRoot = appData + "/cache";
#else
    // Linux: XDG directories
    QString dataHome = qEnvironmentVariable("XDG_DATA_HOME");
    if (dataHome.isEmpty()) {
        dataHome = QDir::homePath() + "/.local/share";
    }
    m_repoRoot = dataHome + "/githubworkflowtool/repos";

    QString cacheHome = qEnvironmentVariable("XDG_CACHE_HOME");
    if (cacheHome.isEmpty()) {
        cacheHome = QDir::homePath() + "/.cache";
    }
    m_cacheRoot = cacheHome + "/githubworkflowtool";
#endif

    ensureDirectoriesExist();
}

QString StorageProvider::getRepoStorageRoot() const {
    return m_repoRoot;
}

QString StorageProvider::getCacheRoot() const {
    return m_cacheRoot;
}

QString StorageProvider::getRepoDirectory(const QString& repoUrl) const {
    QString key = generateRepoKey(repoUrl);
    return m_repoRoot + "/" + key;
}

bool StorageProvider::ensureDirectoriesExist() {
    QDir dir;
    bool success = true;
    
    if (!dir.exists(m_repoRoot)) {
        success &= dir.mkpath(m_repoRoot);
    }
    
    if (!dir.exists(m_cacheRoot)) {
        success &= dir.mkpath(m_cacheRoot);
    }
    
    return success;
}

QString StorageProvider::generateRepoKey(const QString& repoUrl) const {
    QUrl url(repoUrl);
    QString host = url.host();
    QString path = url.path();
    
    // Remove leading/trailing slashes and .git suffix
    path = path.trimmed();
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    if (path.endsWith(".git")) {
        path = path.left(path.length() - 4);
    }
    
    // Generate hash of the full URL for uniqueness
    QByteArray hashData = repoUrl.toUtf8();
    QString hash = QString(QCryptographicHash::hash(hashData, QCryptographicHash::Sha256).toHex()).left(8);
    
    // Create key: host/owner/name_hash
    QString key = host + "/" + path + "_" + hash;
    
    // Sanitize the key to be filesystem-safe
    key.replace(':', '_');
    key.replace('?', '_');
    key.replace('*', '_');
    key.replace('"', '_');
    key.replace('<', '_');
    key.replace('>', '_');
    key.replace('|', '_');
    
    return key;
}

} // namespace core
} // namespace gwt
