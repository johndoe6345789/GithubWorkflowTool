#include "gui/MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GithubWorkflowTool");
    app.setApplicationVersion("0.1.0");
    
    gwt::gui::MainWindow window;
    window.show();
    
    return app.exec();
}
