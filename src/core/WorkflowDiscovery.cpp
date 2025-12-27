#include "core/WorkflowDiscovery.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace gwt {
namespace core {

WorkflowDiscovery::WorkflowDiscovery(QObject* parent)
    : QObject(parent)
{
}

WorkflowDiscovery::~WorkflowDiscovery() = default;

QStringList WorkflowDiscovery::discoverWorkflows(const QString& repoPath) const {
    QStringList workflows;
    
    QString workflowDir = repoPath + "/.github/workflows";
    QDir dir(workflowDir);
    
    if (!dir.exists()) {
        return workflows;
    }
    
    QStringList filters;
    filters << "*.yml" << "*.yaml";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& file : files) {
        if (isValidWorkflow(file.filePath())) {
            workflows << file.filePath();
        }
    }
    
    return workflows;
}

bool WorkflowDiscovery::hasWorkflows(const QString& repoPath) const {
    return !discoverWorkflows(repoPath).isEmpty();
}

bool WorkflowDiscovery::isValidWorkflow(const QString& filePath) const {
    // Basic validation - check if file exists and is readable
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile() || !info.isReadable()) {
        return false;
    }
    
    // More sophisticated validation would parse the YAML
    // For now, just check file extension
    QString suffix = info.suffix().toLower();
    return suffix == "yml" || suffix == "yaml";
}

} // namespace core
} // namespace gwt
