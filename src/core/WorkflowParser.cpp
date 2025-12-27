#include "core/WorkflowParser.h"
#include <yaml-cpp/yaml.h>
#include <QFile>
#include <QDebug>

namespace gwt {
namespace core {

WorkflowParser::WorkflowParser() = default;

WorkflowParser::~WorkflowParser() = default;

Workflow WorkflowParser::parse(const QString& filePath) {
    m_errors.clear();
    Workflow workflow;
    workflow.filePath = filePath;
    
    try {
        YAML::Node root = YAML::LoadFile(filePath.toStdString());
        
        // Parse workflow name
        if (root["name"]) {
            workflow.name = QString::fromStdString(root["name"].as<std::string>());
        }
        
        // Parse triggers (on)
        if (root["on"]) {
            // Simplified parsing - would need more complex handling
            YAML::Node onNode = root["on"];
            if (onNode.IsScalar()) {
                workflow.on["type"] = QString::fromStdString(onNode.as<std::string>());
            } else if (onNode.IsMap()) {
                // Store as QVariantMap for now
                workflow.on["_raw"] = "complex";
            }
        }
        
        // Parse global env
        if (root["env"]) {
            YAML::Node envNode = root["env"];
            for (auto it = envNode.begin(); it != envNode.end(); ++it) {
                QString key = QString::fromStdString(it->first.as<std::string>());
                QString value = QString::fromStdString(it->second.as<std::string>());
                workflow.env[key] = value;
            }
        }
        
        // Parse jobs
        if (root["jobs"]) {
            YAML::Node jobsNode = root["jobs"];
            for (auto it = jobsNode.begin(); it != jobsNode.end(); ++it) {
                QString jobId = QString::fromStdString(it->first.as<std::string>());
                YAML::Node jobNode = it->second;
                
                WorkflowJob job;
                job.id = jobId;
                
                if (jobNode["name"]) {
                    job.name = QString::fromStdString(jobNode["name"].as<std::string>());
                }
                
                if (jobNode["runs-on"]) {
                    job.runsOn = QString::fromStdString(jobNode["runs-on"].as<std::string>());
                }
                
                // Parse needs
                if (jobNode["needs"]) {
                    YAML::Node needsNode = jobNode["needs"];
                    if (needsNode.IsScalar()) {
                        job.needs << QString::fromStdString(needsNode.as<std::string>());
                    } else if (needsNode.IsSequence()) {
                        for (size_t i = 0; i < needsNode.size(); ++i) {
                            job.needs << QString::fromStdString(needsNode[i].as<std::string>());
                        }
                    }
                }
                
                // Parse steps
                if (jobNode["steps"]) {
                    YAML::Node stepsNode = jobNode["steps"];
                    for (size_t i = 0; i < stepsNode.size(); ++i) {
                        YAML::Node stepNode = stepsNode[i];
                        WorkflowStep step;
                        
                        if (stepNode["name"]) {
                            step.name = QString::fromStdString(stepNode["name"].as<std::string>());
                        }
                        
                        if (stepNode["id"]) {
                            step.id = QString::fromStdString(stepNode["id"].as<std::string>());
                        }
                        
                        if (stepNode["run"]) {
                            step.run = QString::fromStdString(stepNode["run"].as<std::string>());
                        }
                        
                        if (stepNode["uses"]) {
                            step.uses = QString::fromStdString(stepNode["uses"].as<std::string>());
                        }
                        
                        if (stepNode["working-directory"]) {
                            step.workingDirectory = QString::fromStdString(stepNode["working-directory"].as<std::string>());
                        }
                        
                        if (stepNode["shell"]) {
                            step.shell = QString::fromStdString(stepNode["shell"].as<std::string>());
                        }
                        
                        if (stepNode["if"]) {
                            step.ifCondition = QString::fromStdString(stepNode["if"].as<std::string>());
                        }
                        
                        job.steps.append(step);
                    }
                }
                
                // Parse strategy (matrix)
                if (jobNode["strategy"]) {
                    YAML::Node strategyNode = jobNode["strategy"];
                    if (strategyNode["matrix"]) {
                        YAML::Node matrixNode = strategyNode["matrix"];
                        QVariantMap matrixMap;
                        
                        for (auto it = matrixNode.begin(); it != matrixNode.end(); ++it) {
                            QString key = QString::fromStdString(it->first.as<std::string>());
                            YAML::Node valueNode = it->second;
                            
                            if (valueNode.IsSequence()) {
                                QVariantList values;
                                for (size_t i = 0; i < valueNode.size(); ++i) {
                                    values << QString::fromStdString(valueNode[i].as<std::string>());
                                }
                                matrixMap[key] = values;
                            } else {
                                matrixMap[key] = QString::fromStdString(valueNode.as<std::string>());
                            }
                        }
                        
                        job.strategy["matrix"] = matrixMap;
                    }
                }
                
                workflow.jobs[jobId] = job;
            }
        }
        
    } catch (const YAML::Exception& e) {
        m_errors << QString("YAML parsing error in %1: %2").arg(filePath, e.what());
    }
    
    return workflow;
}

bool WorkflowParser::hasErrors() const {
    return !m_errors.isEmpty();
}

QStringList WorkflowParser::getErrors() const {
    return m_errors;
}

} // namespace core
} // namespace gwt
