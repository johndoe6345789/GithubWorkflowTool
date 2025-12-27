#include "backends/ContainerBackend.h"
#include <QProcess>
#include <QDebug>

namespace gwt {
namespace backends {

ContainerBackend::ContainerBackend(QObject* parent)
    : ExecutionBackend(parent)
{
    detectRuntime();
}

ContainerBackend::~ContainerBackend() {
    cleanup();
}

bool ContainerBackend::executeStep(const core::WorkflowStep& step,
                                   const QVariantMap& context) {
    if (m_containerId.isEmpty()) {
        emit error("Container not prepared");
        return false;
    }
    
    // Execute command in container
    if (!step.run.isEmpty()) {
        QProcess process;
        QStringList args;
        
        // Use specified shell or default to sh
        QString shell = step.shell.isEmpty() ? "sh" : step.shell;
        args << "exec" << m_containerId << shell << "-c" << step.run;
        
        process.start(m_containerRuntime, args);
        
        if (!process.waitForFinished(STEP_TIMEOUT_MS)) {
            emit error("Step execution timeout");
            return false;
        }
        
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        emit this->output(output);
        
        if (process.exitCode() != 0) {
            QString errorMsg = QString::fromUtf8(process.readAllStandardError());
            emit error("Step failed: " + errorMsg);
            return false;
        }
    } else if (!step.uses.isEmpty()) {
        // Handle actions like actions/checkout@v3
        emit this->output("Action execution: " + step.uses + " (stub)");
        // Would need more complex action resolution
    }
    
    return true;
}

bool ContainerBackend::prepareEnvironment(const QString& runsOn) {
    QString image = mapRunsOnToImage(runsOn);
    
    QProcess process;
    QStringList args;
    args << "run" << "-d" << "-it" << image << "sh";
    
    process.start(m_containerRuntime, args);
    
    if (!process.waitForFinished(PREPARE_TIMEOUT_MS)) {
        emit error("Container creation timeout");
        return false;
    }
    
    if (process.exitCode() != 0) {
        QString errorMsg = QString::fromUtf8(process.readAllStandardError());
        emit error("Failed to create container: " + errorMsg);
        return false;
    }
    
    m_containerId = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    return !m_containerId.isEmpty();
}

void ContainerBackend::cleanup() {
    if (!m_containerId.isEmpty()) {
        QProcess process;
        QStringList args;
        args << "rm" << "-f" << m_containerId;
        
        process.start(m_containerRuntime, args);
        process.waitForFinished(CLEANUP_TIMEOUT_MS);
        
        m_containerId.clear();
    }
}

bool ContainerBackend::detectRuntime() {
    // Try docker first
    QProcess dockerCheck;
    dockerCheck.start("docker", QStringList() << "--version");
    
    if (dockerCheck.waitForFinished(5000) && dockerCheck.exitCode() == 0) {
        m_containerRuntime = "docker";
        return true;
    }
    
    // Try podman
    QProcess podmanCheck;
    podmanCheck.start("podman", QStringList() << "--version");
    
    if (podmanCheck.waitForFinished(5000) && podmanCheck.exitCode() == 0) {
        m_containerRuntime = "podman";
        return true;
    }
    
    return false;
}

QString ContainerBackend::mapRunsOnToImage(const QString& runsOn) const {
    // Map GitHub runner specs to container images
    if (runsOn.contains("ubuntu-latest") || runsOn.contains("ubuntu-22.04")) {
        return "ubuntu:22.04";
    } else if (runsOn.contains("ubuntu-20.04")) {
        return "ubuntu:20.04";
    } else if (runsOn.contains("ubuntu")) {
        return "ubuntu:latest";
    } else if (runsOn.contains("debian")) {
        return "debian:latest";
    } else if (runsOn.contains("alpine")) {
        return "alpine:latest";
    }
    
    // Default
    return "ubuntu:22.04";
}

} // namespace backends
} // namespace gwt
