#include "core/ArtifactManager.h"
#include "core/StorageProvider.h"
#include <QDir>
#include <QFile>
#include <QDebug>

namespace gwt {
namespace core {

ArtifactManager::ArtifactManager(QObject* parent)
    : QObject(parent)
{
}

ArtifactManager::~ArtifactManager() = default;

bool ArtifactManager::uploadArtifact(const QString& name, 
                                     const QString& path,
                                     const QString& workflowId) {
    QString artifactPath = getArtifactPath(name, workflowId);
    
    QDir().mkpath(QFileInfo(artifactPath).path());
    
    // Simple file copy for now - would need compression/archiving for directories
    QFileInfo sourceInfo(path);
    
    if (sourceInfo.isFile()) {
        if (!QFile::copy(path, artifactPath)) {
            emit error("Failed to upload artifact: " + name);
            return false;
        }
    } else if (sourceInfo.isDir()) {
        // Would need recursive directory copying
        emit error("Directory artifacts not yet implemented");
        return false;
    }
    
    return true;
}

bool ArtifactManager::downloadArtifact(const QString& name,
                                       const QString& workflowId,
                                       const QString& destinationPath) {
    QString artifactPath = getArtifactPath(name, workflowId);
    
    if (!QFile::exists(artifactPath)) {
        emit error("Artifact not found: " + name);
        return false;
    }
    
    QDir().mkpath(QFileInfo(destinationPath).path());
    
    if (!QFile::copy(artifactPath, destinationPath)) {
        emit error("Failed to download artifact: " + name);
        return false;
    }
    
    return true;
}

QStringList ArtifactManager::listArtifacts(const QString& workflowId) const {
    QStringList artifacts;
    
    QString artifactDir = StorageProvider::instance().getCacheRoot() + "/artifacts/" + workflowId;
    QDir dir(artifactDir);
    
    if (!dir.exists()) {
        return artifacts;
    }
    
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
        artifacts << file.fileName();
    }
    
    return artifacts;
}

QString ArtifactManager::getArtifactPath(const QString& name, const QString& workflowId) const {
    return StorageProvider::instance().getCacheRoot() + "/artifacts/" + workflowId + "/" + name;
}

} // namespace core
} // namespace gwt
