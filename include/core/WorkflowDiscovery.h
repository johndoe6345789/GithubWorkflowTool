#pragma once

#include <QString>
#include <QStringList>
#include <QObject>

namespace gwt {
namespace core {

/**
 * @brief Discovers GitHub workflow files in a repository
 */
class WorkflowDiscovery : public QObject {
    Q_OBJECT

public:
    explicit WorkflowDiscovery(QObject* parent = nullptr);
    ~WorkflowDiscovery() override;

    /**
     * @brief Discover workflow files in a repository
     * @param repoPath Local path to the repository
     * @return List of workflow file paths
     */
    QStringList discoverWorkflows(const QString& repoPath) const;

    /**
     * @brief Check if a path contains valid workflow files
     * @param repoPath Local path to check
     * @return true if workflows exist
     */
    bool hasWorkflows(const QString& repoPath) const;

private:
    /**
     * @brief Validate that a file is a proper YAML workflow
     * @param filePath Path to the YAML file
     * @return true if valid
     */
    bool isValidWorkflow(const QString& filePath) const;
};

} // namespace core
} // namespace gwt
