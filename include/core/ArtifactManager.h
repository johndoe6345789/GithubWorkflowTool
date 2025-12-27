#pragma once

#include <QString>
#include <QObject>

namespace gwt {
namespace core {

/**
 * @brief Manages workflow artifacts (upload/download)
 */
class ArtifactManager : public QObject {
    Q_OBJECT

public:
    explicit ArtifactManager(QObject* parent = nullptr);
    ~ArtifactManager() override;

    /**
     * @brief Upload an artifact
     * @param name Artifact name
     * @param path Path to artifact file or directory
     * @param workflowId Associated workflow ID
     * @return true if successful
     */
    bool uploadArtifact(const QString& name, 
                       const QString& path,
                       const QString& workflowId);

    /**
     * @brief Download an artifact
     * @param name Artifact name
     * @param workflowId Associated workflow ID
     * @param destinationPath Where to download the artifact
     * @return true if successful
     */
    bool downloadArtifact(const QString& name,
                         const QString& workflowId,
                         const QString& destinationPath);

    /**
     * @brief List all artifacts for a workflow
     * @param workflowId The workflow ID
     * @return List of artifact names
     */
    QStringList listArtifacts(const QString& workflowId) const;

signals:
    void uploadProgress(int percentage);
    void downloadProgress(int percentage);
    void error(const QString& errorMessage);

private:
    QString getArtifactPath(const QString& name, const QString& workflowId) const;
};

} // namespace core
} // namespace gwt
