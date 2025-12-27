#pragma once

#include <QVariantMap>
#include <QList>

namespace gwt {
namespace core {

struct WorkflowJob;

/**
 * @brief Handles matrix strategy expansion for workflow jobs
 */
class MatrixStrategy {
public:
    MatrixStrategy();
    ~MatrixStrategy();

    /**
     * @brief Expand a job with matrix strategy into multiple jobs
     * @param job The job with matrix strategy
     * @return List of expanded jobs
     */
    QList<WorkflowJob> expandMatrix(const WorkflowJob& job) const;

    /**
     * @brief Check if a job has a matrix strategy
     * @param job The job to check
     * @return true if matrix strategy is present
     */
    bool hasMatrix(const WorkflowJob& job) const;

private:
    /**
     * @brief Generate all combinations from matrix variables
     */
    QList<QVariantMap> generateCombinations(const QVariantMap& matrix) const;
};

} // namespace core
} // namespace gwt
