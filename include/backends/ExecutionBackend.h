#pragma once

#include "core/WorkflowParser.h"
#include <QObject>
#include <QString>

namespace gwt {
namespace backends {

/**
 * @brief Base class for execution backends
 */
class ExecutionBackend : public QObject {
    Q_OBJECT

public:
    explicit ExecutionBackend(QObject* parent = nullptr);
    ~ExecutionBackend() override;

    /**
     * @brief Execute a job step
     * @param step The step to execute
     * @param context Execution context (env vars, working dir, etc.)
     * @return true if successful
     */
    virtual bool executeStep(const core::WorkflowStep& step,
                            const QVariantMap& context) = 0;

    /**
     * @brief Prepare the execution environment
     * @param runsOn The runner specification (ubuntu-latest, etc.)
     * @return true if successful
     */
    virtual bool prepareEnvironment(const QString& runsOn) = 0;

    /**
     * @brief Cleanup the execution environment
     */
    virtual void cleanup() = 0;

signals:
    void output(const QString& text);
    void error(const QString& errorMessage);
};

} // namespace backends
} // namespace gwt
