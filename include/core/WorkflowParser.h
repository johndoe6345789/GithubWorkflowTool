#pragma once

#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <memory>

namespace gwt {
namespace core {

/**
 * @brief Represents a workflow step
 */
struct WorkflowStep {
    QString name;
    QString id;
    QString run;                    // Shell command
    QString uses;                   // Action to use (e.g., actions/checkout@v3)
    QVariantMap with;               // Parameters for the action
    QVariantMap env;                // Environment variables
    QString workingDirectory;
    QString shell;
    QString ifCondition;            // Conditional execution
};

/**
 * @brief Represents a workflow job
 */
struct WorkflowJob {
    QString id;
    QString name;
    QString runsOn;                 // e.g., ubuntu-latest, windows-latest
    QStringList needs;              // Dependencies on other jobs
    QVariantMap env;                // Environment variables
    QVariantMap outputs;            // Job outputs
    QList<WorkflowStep> steps;
    QVariantMap strategy;           // Matrix strategy
    QString ifCondition;            // Conditional execution
};

/**
 * @brief Represents a complete workflow
 */
struct Workflow {
    QString name;
    QString filePath;
    QVariantMap on;                 // Trigger events
    QVariantMap env;                // Global environment variables
    QMap<QString, WorkflowJob> jobs;
};

/**
 * @brief Parses GitHub workflow YAML files
 */
class WorkflowParser {
public:
    WorkflowParser();
    ~WorkflowParser();

    /**
     * @brief Parse a workflow file
     * @param filePath Path to the workflow YAML file
     * @return Parsed workflow structure
     */
    Workflow parse(const QString& filePath);

    /**
     * @brief Check if the last parse had errors
     * @return true if errors occurred
     */
    bool hasErrors() const;

    /**
     * @brief Get error messages from last parse
     * @return List of error messages
     */
    QStringList getErrors() const;

private:
    QStringList m_errors;
};

} // namespace core
} // namespace gwt
