#include "core/JobExecutor.h"
#include "backends/ContainerBackend.h"
#include "backends/QemuBackend.h"
#include <QDebug>

namespace gwt {
namespace core {

JobExecutor::JobExecutor(QObject* parent)
    : QObject(parent)
    , m_running(false)
{
}

JobExecutor::~JobExecutor() = default;

bool JobExecutor::executeWorkflow(const Workflow& workflow, 
                                  const QString& triggerEvent,
                                  bool useQemu) {
    if (m_running) {
        emit error("Execution already in progress");
        return false;
    }
    
    m_running = true;
    
    // Create appropriate backend
    if (useQemu) {
        m_backend = std::make_unique<backends::QemuBackend>(this);
    } else {
        m_backend = std::make_unique<backends::ContainerBackend>(this);
    }
    
    // Connect backend signals
    connect(m_backend.get(), &backends::ExecutionBackend::output,
            [this](const QString& text) {
        emit stepOutput("", "", text);
    });
    
    // Resolve job execution order
    QStringList jobOrder = resolveJobOrder(workflow);
    
    bool success = true;
    for (const QString& jobId : jobOrder) {
        const WorkflowJob& job = workflow.jobs[jobId];
        
        emit jobStarted(jobId);
        
        if (!executeJob(job)) {
            success = false;
            emit jobFinished(jobId, false);
            break;
        }
        
        emit jobFinished(jobId, true);
    }
    
    m_running = false;
    emit executionFinished(success);
    
    return success;
}

void JobExecutor::stopExecution() {
    if (m_running && m_backend) {
        m_backend->cleanup();
        m_running = false;
    }
}

bool JobExecutor::isRunning() const {
    return m_running;
}

bool JobExecutor::executeJob(const WorkflowJob& job) {
    // Prepare environment
    if (!m_backend->prepareEnvironment(job.runsOn)) {
        emit error("Failed to prepare environment for: " + job.runsOn);
        return false;
    }
    
    // Execute steps
    for (const WorkflowStep& step : job.steps) {
        emit stepStarted(job.id, step.name);
        
        QVariantMap context;
        context["env"] = job.env;
        context["workingDirectory"] = step.workingDirectory;
        
        if (!m_backend->executeStep(step, context)) {
            emit stepFinished(job.id, step.name, false);
            return false;
        }
        
        emit stepFinished(job.id, step.name, true);
    }
    
    return true;
}

QStringList JobExecutor::resolveJobOrder(const Workflow& workflow) const {
    QStringList order;
    QSet<QString> processed;
    QMap<QString, QStringList> dependencies;
    
    // Build dependency map
    for (auto it = workflow.jobs.begin(); it != workflow.jobs.end(); ++it) {
        dependencies[it.key()] = it.value().needs;
    }
    
    // Simple topological sort
    bool changed = true;
    while (changed && processed.size() < workflow.jobs.size()) {
        changed = false;
        
        for (auto it = workflow.jobs.begin(); it != workflow.jobs.end(); ++it) {
            QString jobId = it.key();
            
            if (processed.contains(jobId)) {
                continue;
            }
            
            // Check if all dependencies are satisfied
            bool canRun = true;
            for (const QString& dep : dependencies[jobId]) {
                if (!processed.contains(dep)) {
                    canRun = false;
                    break;
                }
            }
            
            if (canRun) {
                order << jobId;
                processed.insert(jobId);
                changed = true;
            }
        }
    }
    
    return order;
}

} // namespace core
} // namespace gwt
