#include "cli/CommandHandler.h"
#include "core/RepoManager.h"
#include "core/JobExecutor.h"
#include "core/WorkflowDiscovery.h"
#include "core/WorkflowParser.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

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

} // namespace cli
} // namespace gwt
