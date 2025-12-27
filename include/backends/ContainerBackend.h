#pragma once

#include "ExecutionBackend.h"

namespace gwt {
namespace backends {

/**
 * @brief Container-based execution backend (using Docker or Podman)
 */
class ContainerBackend : public ExecutionBackend {
    Q_OBJECT

public:
    explicit ContainerBackend(QObject* parent = nullptr);
    ~ContainerBackend() override;

    bool executeStep(const core::WorkflowStep& step,
                    const QVariantMap& context) override;

    bool prepareEnvironment(const QString& runsOn) override;

    void cleanup() override;

private:
    static constexpr int STEP_TIMEOUT_MS = 300000;  // 5 minutes
    static constexpr int PREPARE_TIMEOUT_MS = 60000; // 1 minute
    static constexpr int CLEANUP_TIMEOUT_MS = 30000; // 30 seconds
    
    QString m_containerId;
    QString m_containerRuntime;  // "docker" or "podman"
    
    /**
     * @brief Detect available container runtime
     */
    bool detectRuntime();
    
    /**
     * @brief Map GitHub runner spec to container image
     */
    QString mapRunsOnToImage(const QString& runsOn) const;
};

} // namespace backends
} // namespace gwt
