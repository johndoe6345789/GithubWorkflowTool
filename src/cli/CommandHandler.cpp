#include "cli/CommandHandler.h"
#include "core/RepoManager.h"
#include "core/JobExecutor.h"
#include "core/WorkflowDiscovery.h"
#include "core/WorkflowParser.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QFileInfo>

namespace gwt {
namespace cli {

CommandHandler::CommandHandler(QObject* parent)
    : QObject(parent)
    , m_repoManager(std::make_unique<core::RepoManager>())
    , m_executor(std::make_unique<core::JobExecutor>())
{
}

CommandHandler::~CommandHandler() = default;

int CommandHandler::execute(const QStringList& args) {
    if (args.isEmpty() || args[0] == "help" || args[0] == "--help" || args[0] == "-h") {
        printHelp();
        return 0;
    }
    
    QString command = args[0];
    
    if (command == "clone") {
        return handleClone(args.mid(1));
    } else if (command == "list") {
        return handleList(args.mid(1));
    } else if (command == "run") {
        return handleRun(args.mid(1));
    } else if (command == "workflows") {
        return handleWorkflows(args.mid(1));
    } else if (command == "doctor") {
        return handleDoctor(args.mid(1));
    } else {
        QTextStream err(stderr);
        err << "Unknown command: " << command << Qt::endl;
        printHelp();
        return 1;
    }
}

void CommandHandler::printHelp() const {
    QTextStream out(stdout);
    out << "GithubWorkflowTool - Local GitHub Workflow Simulator" << Qt::endl;
    out << Qt::endl;
    out << "Usage: gwt <command> [options]" << Qt::endl;
    out << Qt::endl;
    out << "Commands:" << Qt::endl;
    out << "  clone <url>        Clone a repository" << Qt::endl;
    out << "  list               List cloned repositories" << Qt::endl;
    out << "  workflows <repo>   List workflows in a repository" << Qt::endl;
    out << "  run <repo> <wf>    Run a workflow" << Qt::endl;
    out << "  doctor [workflow]  Check system and workflow compatibility" << Qt::endl;
    out << "  help               Show this help message" << Qt::endl;
    out << Qt::endl;
}

int CommandHandler::handleClone(const QStringList& args) {
    if (args.isEmpty()) {
        QTextStream err(stderr);
        err << "Error: Repository URL required" << Qt::endl;
        return 1;
    }
    
    QString repoUrl = args[0];
    QTextStream out(stdout);
    out << "Cloning repository: " << repoUrl << Qt::endl;
    
    if (m_repoManager->cloneRepository(repoUrl)) {
        out << "Successfully cloned to: " << m_repoManager->getLocalPath(repoUrl) << Qt::endl;
        return 0;
    } else {
        QTextStream err(stderr);
        err << "Failed to clone repository" << Qt::endl;
        return 1;
    }
}

int CommandHandler::handleList(const QStringList& args) {
    Q_UNUSED(args);
    
    QStringList repos = m_repoManager->listRepositories();
    QTextStream out(stdout);
    
    out << "Cloned repositories:" << Qt::endl;
    for (const QString& repo : repos) {
        out << "  " << repo << Qt::endl;
    }
    
    return 0;
}

int CommandHandler::handleRun(const QStringList& args) {
    if (args.size() < 2) {
        QTextStream err(stderr);
        err << "Error: Repository path and workflow file required" << Qt::endl;
        return 1;
    }
    
    QString repoPath = args[0];
    QString workflowFile = args[1];
    
    QTextStream out(stdout);
    out << "Running workflow: " << workflowFile << Qt::endl;
    
    // Parse workflow
    core::WorkflowParser parser;
    core::Workflow workflow = parser.parse(workflowFile);
    
    if (parser.hasErrors()) {
        QTextStream err(stderr);
        err << "Workflow parsing errors:" << Qt::endl;
        for (const QString& error : parser.getErrors()) {
            err << "  " << error << Qt::endl;
        }
        return 1;
    }
    
    // Execute workflow
    bool useQemu = args.contains("--qemu");
    if (m_executor->executeWorkflow(workflow, "push", useQemu)) {
        out << "Workflow execution completed" << Qt::endl;
        return 0;
    } else {
        QTextStream err(stderr);
        err << "Workflow execution failed" << Qt::endl;
        return 1;
    }
}

int CommandHandler::handleWorkflows(const QStringList& args) {
    if (args.isEmpty()) {
        QTextStream err(stderr);
        err << "Error: Repository path required" << Qt::endl;
        return 1;
    }
    
    QString repoPath = args[0];
    core::WorkflowDiscovery discovery;
    QStringList workflows = discovery.discoverWorkflows(repoPath);
    
    QTextStream out(stdout);
    out << "Workflows in " << repoPath << ":" << Qt::endl;
    for (const QString& workflow : workflows) {
        out << "  " << workflow << Qt::endl;
    }
    
    return 0;
}

int CommandHandler::handleDoctor(const QStringList& args) {
    QTextStream out(stdout);
    out << "GithubWorkflowTool Diagnostics" << Qt::endl;
    out << "==============================" << Qt::endl;
    out << Qt::endl;
    
    int issues = 0;
    int warnings = 0;
    int errors = 0;
    
    // Check backend availability
    out << "Backend Availability:" << Qt::endl;
    
    // Check Docker
    QProcess dockerCheck;
    dockerCheck.start("docker", QStringList() << "--version");
    dockerCheck.waitForFinished(3000);
    
    if (dockerCheck.exitCode() == 0) {
        QString dockerVersion = QString::fromUtf8(dockerCheck.readAllStandardOutput()).trimmed();
        out << "✓ Container backend: Docker detected (" << dockerVersion.split(" ").value(2, "").remove(',') << ")" << Qt::endl;
    } else {
        // Try Podman
        QProcess podmanCheck;
        podmanCheck.start("podman", QStringList() << "--version");
        podmanCheck.waitForFinished(3000);
        
        if (podmanCheck.exitCode() == 0) {
            QString podmanVersion = QString::fromUtf8(podmanCheck.readAllStandardOutput()).trimmed();
            out << "✓ Container backend: Podman detected (" << podmanVersion.split(" ").value(2, "") << ")" << Qt::endl;
        } else {
            out << "✗ Container backend: Neither Docker nor Podman found" << Qt::endl;
            out << "  → Install Docker or Podman for container backend support" << Qt::endl;
            errors++;
            issues++;
        }
    }
    
    // Check QEMU
    QProcess qemuCheck;
    qemuCheck.start("qemu-system-x86_64", QStringList() << "--version");
    qemuCheck.waitForFinished(3000);
    
    if (qemuCheck.exitCode() == 0) {
        QString qemuVersion = QString::fromUtf8(qemuCheck.readAllStandardOutput()).split('\n').first().trimmed();
        out << "✓ QEMU backend: Available (" << qemuVersion.split(" ").value(3, "") << ")" << Qt::endl;
    } else {
        out << "⚠ QEMU backend: Not available" << Qt::endl;
        out << "  → Install QEMU for VM-based execution (optional)" << Qt::endl;
        warnings++;
        issues++;
    }
    
    out << Qt::endl;
    
    // Check workflow if provided
    if (!args.isEmpty()) {
        QString workflowFile = args[0];
        
        if (!QFile::exists(workflowFile)) {
            out << "Error: Workflow file not found: " << workflowFile << Qt::endl;
            return 1;
        }
        
        out << "Workflow: " << QFileInfo(workflowFile).fileName() << Qt::endl;
        
        // Parse workflow
        core::WorkflowParser parser;
        core::Workflow workflow = parser.parse(workflowFile);
        
        if (parser.hasErrors()) {
            out << "✗ Error: Workflow parsing failed" << Qt::endl;
            for (const QString& error : parser.getErrors()) {
                out << "  " << error << Qt::endl;
            }
            errors++;
            issues++;
        } else {
            out << "✓ Basic workflow structure valid" << Qt::endl;
            
            // Check for job dependencies
            bool hasValidDeps = true;
            for (const auto& [jobId, job] : workflow.jobs.asKeyValueRange()) {
                if (!job.needs.isEmpty()) {
                    for (const QString& dep : job.needs) {
                        if (!workflow.jobs.contains(dep)) {
                            hasValidDeps = false;
                            out << "✗ Error: Job '" << jobId << "' depends on non-existent job '" << dep << "'" << Qt::endl;
                            errors++;
                            issues++;
                        }
                    }
                }
            }
            
            if (hasValidDeps && workflow.jobs.size() > 1) {
                bool hasDeps = false;
                for (const auto& [jobId, job] : workflow.jobs.asKeyValueRange()) {
                    if (!job.needs.isEmpty()) {
                        hasDeps = true;
                        break;
                    }
                }
                if (hasDeps) {
                    out << "✓ Job dependencies resolvable" << Qt::endl;
                }
            }
            
            // Check for known limitations
            QString workflowContent;
            QFile file(workflowFile);
            if (file.open(QIODevice::ReadOnly)) {
                workflowContent = QString::fromUtf8(file.readAll());
                file.close();
            }
            
            // Check for reusable workflows
            if (workflowContent.contains("workflow_call")) {
                out << "⚠ Warning: Uses reusable workflow (not supported in v1)" << Qt::endl;
                out << "  → Workaround: Flatten workflow or wait for v2" << Qt::endl;
                out << "  → See LIMITATIONS.md §3.1" << Qt::endl;
                warnings++;
                issues++;
            }
            
            // Check for service containers
            if (workflowContent.contains("services:")) {
                out << "⚠ Warning: Service containers detected (not supported in v1)" << Qt::endl;
                out << "  → Workaround: Run services manually before workflow execution" << Qt::endl;
                out << "  → See LIMITATIONS.md §3.3" << Qt::endl;
                warnings++;
                issues++;
            }
            
            // Check for advanced expressions
            QStringList advancedExpressions = {"fromJSON", "hashFiles", "toJSON"};
            for (const QString& expr : advancedExpressions) {
                if (workflowContent.contains(expr)) {
                    out << "⚠ Warning: Uses '" << expr << "' expression (limited support in v1)" << Qt::endl;
                    out << "  → Workaround: Simplify expression or use explicit run steps" << Qt::endl;
                    out << "  → See LIMITATIONS.md §3.1" << Qt::endl;
                    warnings++;
                    issues++;
                    break; // Only warn once for expressions
                }
            }
            
            // Check for macOS runners
            if (workflowContent.contains("macos-latest") || workflowContent.contains("macos-")) {
                out << "✗ Error: macOS runners not supported in v1" << Qt::endl;
                out << "  → Workaround: Use Linux runners or run macOS workflows on GitHub" << Qt::endl;
                out << "  → See LIMITATIONS.md §3.4" << Qt::endl;
                errors++;
                issues++;
            }
            
            // Check for concurrency groups
            if (workflowContent.contains("concurrency:")) {
                out << "⚠ Warning: Concurrency groups (not supported in v1)" << Qt::endl;
                out << "  → Note: All jobs run as configured, no concurrency limits applied" << Qt::endl;
                out << "  → See LIMITATIONS.md §3.1" << Qt::endl;
                warnings++;
                issues++;
            }
        }
        
        out << Qt::endl;
    }
    
    // Summary
    if (issues == 0) {
        out << "Summary: No issues detected" << Qt::endl;
        out << "✓ System is ready for workflow execution" << Qt::endl;
    } else {
        out << "Summary: " << issues << " issue(s) detected";
        if (warnings > 0 && errors > 0) {
            out << " (" << warnings << " warning(s), " << errors << " error(s))";
        } else if (warnings > 0) {
            out << " (" << warnings << " warning(s))";
        } else if (errors > 0) {
            out << " (" << errors << " error(s))";
        }
        out << Qt::endl;
        
        if (errors > 0) {
            out << "⚠ Workflow may fail or require modifications to run successfully" << Qt::endl;
        } else {
            out << "⚠ Workflow should run with noted limitations" << Qt::endl;
        }
    }
    
    out << Qt::endl;
    out << "For detailed information on limitations and workarounds, see LIMITATIONS.md" << Qt::endl;
    
    return errors > 0 ? 1 : 0;
}

} // namespace cli
} // namespace gwt
