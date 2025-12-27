#include "core/MatrixStrategy.h"
#include "core/WorkflowParser.h"
#include <QDebug>

namespace gwt {
namespace core {

MatrixStrategy::MatrixStrategy() = default;

MatrixStrategy::~MatrixStrategy() = default;

QList<WorkflowJob> MatrixStrategy::expandMatrix(const WorkflowJob& job) const {
    QList<WorkflowJob> expandedJobs;
    
    if (!hasMatrix(job)) {
        expandedJobs << job;
        return expandedJobs;
    }
    
    // Extract matrix variables
    QVariantMap matrix = job.strategy["matrix"].toMap();
    QList<QVariantMap> combinations = generateCombinations(matrix);
    
    // Create a job for each combination
    for (const QVariantMap& combo : combinations) {
        WorkflowJob expandedJob = job;
        
        // Update job ID to include matrix values
        QString matrixSuffix = "(";
        for (auto it = combo.begin(); it != combo.end(); ++it) {
            if (it != combo.begin()) {
                matrixSuffix += ", ";
            }
            matrixSuffix += it.key() + "=" + it.value().toString();
        }
        matrixSuffix += ")";
        
        expandedJob.id = job.id + matrixSuffix;
        expandedJob.name = job.name + " " + matrixSuffix;
        
        // Add matrix variables to environment
        for (auto it = combo.begin(); it != combo.end(); ++it) {
            expandedJob.env["matrix." + it.key()] = it.value();
        }
        
        expandedJobs << expandedJob;
    }
    
    return expandedJobs;
}

bool MatrixStrategy::hasMatrix(const WorkflowJob& job) const {
    return job.strategy.contains("matrix") && !job.strategy["matrix"].toMap().isEmpty();
}

QList<QVariantMap> MatrixStrategy::generateCombinations(const QVariantMap& matrix) const {
    QList<QVariantMap> results;
    
    if (matrix.isEmpty()) {
        return results;
    }
    
    // Get all keys and their values
    QStringList keys = matrix.keys();
    QList<QVariantList> valueLists;
    
    for (const QString& key : keys) {
        QVariant value = matrix[key];
        if (value.type() == QVariant::List) {
            valueLists << value.toList();
        } else {
            valueLists << QVariantList{value};
        }
    }
    
    // Generate all combinations
    QList<int> indices(keys.size(), 0);
    bool done = false;
    
    while (!done) {
        QVariantMap combo;
        for (int i = 0; i < keys.size(); ++i) {
            combo[keys[i]] = valueLists[i][indices[i]];
        }
        results << combo;
        
        // Increment indices
        int pos = keys.size() - 1;
        while (pos >= 0) {
            indices[pos]++;
            if (indices[pos] < valueLists[pos].size()) {
                break;
            }
            indices[pos] = 0;
            pos--;
        }
        
        if (pos < 0) {
            done = true;
        }
    }
    
    return results;
}

} // namespace core
} // namespace gwt
