#include "core/CacheManager.h"
#include "core/StorageProvider.h"
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

namespace gwt {
namespace core {

CacheManager::CacheManager(QObject* parent)
    : QObject(parent)
{
}

CacheManager::~CacheManager() = default;

bool CacheManager::saveCache(const QString& key, const QStringList& paths) {
    QString cachePath = getCachePath(key);
    
    QDir().mkpath(QFileInfo(cachePath).path());
    
    // Simple implementation - would need tar/compression for real use
    for (const QString& path : paths) {
        QFileInfo info(path);
        if (info.isFile()) {
            QString destPath = cachePath + "/" + info.fileName();
            QDir().mkpath(cachePath);
            if (!QFile::copy(path, destPath)) {
                emit error("Failed to cache file: " + path);
                return false;
            }
        }
    }
    
    return true;
}

bool CacheManager::restoreCache(const QString& key, const QStringList& paths) {
    QString cachePath = getCachePath(key);
    
    if (!hasCache(key)) {
        emit cacheMiss(key);
        return false;
    }
    
    // Restore cached files
    QDir cacheDir(cachePath);
    QFileInfoList cachedFiles = cacheDir.entryInfoList(QDir::Files);
    
    for (int i = 0; i < cachedFiles.size() && i < paths.size(); ++i) {
        QString destPath = paths[i];
        QDir().mkpath(QFileInfo(destPath).path());
        
        if (!QFile::copy(cachedFiles[i].filePath(), destPath)) {
            emit error("Failed to restore cached file: " + destPath);
            return false;
        }
    }
    
    emit cacheHit(key);
    return true;
}

bool CacheManager::hasCache(const QString& key) const {
    QString cachePath = getCachePath(key);
    return QDir(cachePath).exists();
}

void CacheManager::clearAll() {
    QString cacheRoot = StorageProvider::instance().getCacheRoot() + "/cache";
    QDir dir(cacheRoot);
    dir.removeRecursively();
}

void CacheManager::clearCache(const QString& key) {
    QString cachePath = getCachePath(key);
    QDir dir(cachePath);
    dir.removeRecursively();
}

QString CacheManager::getCachePath(const QString& key) const {
    // Hash the key for filesystem safety
    QByteArray hashData = key.toUtf8();
    QString hash = QString(QCryptographicHash::hash(hashData, QCryptographicHash::Sha256).toHex());
    
    return StorageProvider::instance().getCacheRoot() + "/cache/" + hash;
}

} // namespace core
} // namespace gwt
