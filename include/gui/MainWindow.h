#pragma once

#include <QMainWindow>
#include <memory>

class QTreeWidget;
class QTextEdit;
class QPushButton;
class QComboBox;

namespace gwt {
namespace core {
class RepoManager;
class JobExecutor;
}

namespace gui {

/**
 * @brief Main window for the GUI application
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCloneRepository();
    void onRefreshRepositories();
    void onRepositorySelected();
    void onRunWorkflow();
    void onJobOutput(const QString& jobId, const QString& stepName, const QString& output);

private:
    void setupUI();
    void loadRepositories();

    QTreeWidget* m_repoTree;
    QTreeWidget* m_workflowTree;
    QTextEdit* m_outputView;
    QPushButton* m_cloneButton;
    QPushButton* m_runButton;
    QComboBox* m_backendCombo;

    std::unique_ptr<core::RepoManager> m_repoManager;
    std::unique_ptr<core::JobExecutor> m_executor;
    std::unique_ptr<core::WorkflowParser> m_parser;
};

} // namespace gui
} // namespace gwt
