#pragma once

#include "ExecutionBackend.h"

namespace gwt {
namespace backends {

/**
 * @brief QEMU VM-based execution backend for higher fidelity
 */
class QemuBackend : public ExecutionBackend {
    Q_OBJECT

public:
    explicit QemuBackend(QObject* parent = nullptr);
    ~QemuBackend() override;

    bool executeStep(const core::WorkflowStep& step,
                    const QVariantMap& context) override;

    bool prepareEnvironment(const QString& runsOn) override;

    void cleanup() override;

private:
    QString m_vmId;
    QString m_qemuPath;
    
    /**
     * @brief Find QEMU executable
     */
    bool detectQemu();
    
    /**
     * @brief Map GitHub runner spec to VM image
     */
    QString mapRunsOnToVMImage(const QString& runsOn) const;
    
    /**
     * @brief Start the VM
     */
    bool startVM(const QString& imagePath);
    
    /**
     * @brief Stop the VM
     */
    void stopVM();
};

} // namespace backends
} // namespace gwt
