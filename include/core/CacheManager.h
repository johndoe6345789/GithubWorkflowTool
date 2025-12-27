#pragma once

#include <QString>
#include <QStringList>
#include <QObject>

namespace gwt {
namespace core {

/**
 * @brief Manages workflow caching (actions/cache equivalent)
 */
class CacheManager : public QObject {
    Q_OBJECT

public:
    explicit CacheManager(QObject* parent = nullptr);
    ~CacheManager() override;

    /**
     * @brief Save paths to cache with a key
     * @param key Cache key
     * @param paths Paths to cache
     * @return true if successful
     */
    bool saveCache(const QString& key, const QStringList& paths);

    /**
     * @brief Restore cached paths
     * @param key Cache key
     * @param paths Paths to restore to
     * @return true if cache hit and restored
     */
    bool restoreCache(const QString& key, const QStringList& paths);

    /**
     * @brief Check if a cache key exists
     * @param key Cache key
     * @return true if exists
     */
    bool hasCache(const QString& key) const;

    /**
     * @brief Clear all caches
     */
    void clearAll();

    /**
     * @brief Clear specific cache entry
     * @param key Cache key
     */
    void clearCache(const QString& key);

signals:
    void cacheHit(const QString& key);
    void cacheMiss(const QString& key);
    void error(const QString& errorMessage);

private:
    QString getCachePath(const QString& key) const;
};

} // namespace core
} // namespace gwt
