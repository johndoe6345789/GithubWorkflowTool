#pragma once

#include "WorkflowParser.h"
#include <QObject>
#include <memory>

namespace gwt {
namespace backends {
class ExecutionBackend;
}

namespace core {

/**
 * @brief Executes workflow jobs and manages their lifecycle
 */
class JobExecutor : public QObject {
    Q_OBJECT

public:
    explicit JobExecutor(QObject* parent = nullptr);
    ~JobExecutor() override;

    /**
     * @brief Execute a complete workflow
     * @param workflow The workflow to execute
     * @param triggerEvent The event that triggered the workflow
     * @param useQemu Use QEMU backend instead of container backend
     * @return true if execution started successfully
     */
    bool executeWorkflow(const Workflow& workflow, 
                        const QString& triggerEvent,
                        bool useQemu = false);

    /**
     * @brief Stop execution of current workflow
     */
    void stopExecution();

    /**
     * @brief Check if execution is currently running
     * @return true if running
     */
    bool isRunning() const;

signals:
    void jobStarted(const QString& jobId);
    void jobFinished(const QString& jobId, bool success);
    void stepStarted(const QString& jobId, const QString& stepName);
    void stepFinished(const QString& jobId, const QString& stepName, bool success);
    void stepOutput(const QString& jobId, const QString& stepName, const QString& output);
    void executionFinished(bool success);
    void error(const QString& errorMessage);

private:
    bool m_running;
    std::unique_ptr<backends::ExecutionBackend> m_backend;
    
    /**
     * @brief Execute a single job
     */
    bool executeJob(const WorkflowJob& job);
    
    /**
     * @brief Resolve job dependencies (needs)
     */
    QStringList resolveJobOrder(const Workflow& workflow) const;
};

} // namespace core
} // namespace gwt
