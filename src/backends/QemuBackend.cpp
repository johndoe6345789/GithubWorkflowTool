#include "backends/QemuBackend.h"
#include <QProcess>
#include <QDebug>

namespace gwt {
namespace backends {

QemuBackend::QemuBackend(QObject* parent)
    : ExecutionBackend(parent)
{
    detectQemu();
}

QemuBackend::~QemuBackend() {
    cleanup();
}

bool QemuBackend::executeStep(const core::WorkflowStep& step,
                              const QVariantMap& context) {
    if (m_vmId.isEmpty()) {
        emit error("VM not prepared");
        return false;
    }
    
    // Execute command in VM via SSH or QEMU guest agent
    if (!step.run.isEmpty()) {
        // Stub implementation - would need VM communication setup
        emit output("Executing in VM: " + step.run);
        // Would use QEMU monitor or SSH to execute
    } else if (!step.uses.isEmpty()) {
        emit output("Action execution in VM: " + step.uses + " (stub)");
    }
    
    return true;
}

bool QemuBackend::prepareEnvironment(const QString& runsOn) {
    QString vmImage = mapRunsOnToVMImage(runsOn);
    
    if (!startVM(vmImage)) {
        emit error("Failed to start VM");
        return false;
    }
    
    return true;
}

void QemuBackend::cleanup() {
    stopVM();
}

bool QemuBackend::detectQemu() {
    QProcess qemuCheck;
    qemuCheck.start("qemu-system-x86_64", QStringList() << "--version");
    
    if (qemuCheck.waitForFinished(5000) && qemuCheck.exitCode() == 0) {
        m_qemuPath = "qemu-system-x86_64";
        return true;
    }
    
    return false;
}

QString QemuBackend::mapRunsOnToVMImage(const QString& runsOn) const {
    // Map GitHub runner specs to VM images
    // These would be pre-built VM images stored locally
    if (runsOn.contains("ubuntu-latest") || runsOn.contains("ubuntu-22.04")) {
        return "ubuntu-22.04.qcow2";
    } else if (runsOn.contains("ubuntu-20.04")) {
        return "ubuntu-20.04.qcow2";
    } else if (runsOn.contains("windows-latest")) {
        return "windows-2022.qcow2";
    }
    
    return "ubuntu-22.04.qcow2";
}

bool QemuBackend::startVM(const QString& imagePath) {
    // Stub implementation - would need proper VM startup with networking
    m_vmId = "vm-" + QString::number(QDateTime::currentSecsSinceEpoch());
    
    // Would execute something like:
    // qemu-system-x86_64 -m 2048 -smp 2 -hda imagePath -net user -net nic
    
    emit output("Starting QEMU VM with image: " + imagePath);
    
    // For now, just simulate
    return true;
}

void QemuBackend::stopVM() {
    if (!m_vmId.isEmpty()) {
        emit output("Stopping VM: " + m_vmId);
        // Would send shutdown command to VM
        m_vmId.clear();
    }
}

} // namespace backends
} // namespace gwt
