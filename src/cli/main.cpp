#include "cli/CommandHandler.h"
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("GithubWorkflowTool");
    app.setApplicationVersion("0.1.0");
    
    gwt::cli::CommandHandler handler;
    
    QStringList args = app.arguments();
    args.removeFirst(); // Remove program name
    
    return handler.execute(args);
}
