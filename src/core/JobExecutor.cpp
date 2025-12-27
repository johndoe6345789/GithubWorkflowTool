#include "core/JobExecutor.h"
#include "backends/ContainerBackend.h"
#include "backends/QemuBackend.h"
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QStringList>

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
    
    // Build dependency graph for gated execution
    QMap<QString, QStringList> dependencies;
    QMap<QString, QStringList> dependents;
    QStringList readyQueue;
    QSet<QString> queued;

    for (auto it = workflow.jobs.begin(); it != workflow.jobs.end(); ++it) {
        const QString& jobId = it.key();
        const QStringList& needs = it.value().needs;
        dependencies[jobId] = needs;

        if (needs.isEmpty()) {
            readyQueue << jobId;
            queued.insert(jobId);
        }

        for (const QString& dep : needs) {
            dependents[dep] << jobId;
        }
    }

    if (readyQueue.isEmpty()) {
        emit error("No runnable jobs found. Check for circular or missing dependencies.");
        m_running = false;
        return false;
    }

    QSet<QString> processed;
    QSet<QString> failed;
    bool success = true;

    while (!readyQueue.isEmpty()) {
        QString jobId = readyQueue.takeFirst();
        queued.remove(jobId);

        const WorkflowJob& job = workflow.jobs[jobId];

        emit jobStarted(jobId);

        bool jobSuccess = executeJob(job);
        emit jobFinished(jobId, jobSuccess);

        processed.insert(jobId);
        if (!jobSuccess) {
            failed.insert(jobId);
            success = false;
        }

        // Evaluate dependents and gate execution based on dependency outcomes
        for (const QString& dependentId : dependents[jobId]) {
            if (processed.contains(dependentId) || queued.contains(dependentId)) {
                continue;
            }

            const QStringList& needs = dependencies[dependentId];

            bool depsProcessed = true;
            bool depsFailed = false;
            for (const QString& dep : needs) {
                if (!processed.contains(dep)) {
                    depsProcessed = false;
                    break;
                }
                if (failed.contains(dep)) {
                    depsFailed = true;
                }
            }

            if (!depsProcessed) {
                continue;
            }

            if (depsFailed) {
                emit error(QStringLiteral("Skipping %1 because a dependency failed").arg(dependentId));
                emit jobFinished(dependentId, false);
                processed.insert(dependentId);
                failed.insert(dependentId);
                success = false;
            } else {
                readyQueue << dependentId;
                queued.insert(dependentId);
            }
        }
    }

    if (processed.size() != workflow.jobs.size()) {
        emit error("Workflow contains unresolved dependencies or cycles");
        success = false;
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

} // namespace core
} // namespace gwt
