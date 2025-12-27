#include "gui/MainWindow.h"
#include "core/RepoManager.h"
#include "core/JobExecutor.h"
#include "core/WorkflowDiscovery.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QFileInfo>

namespace gwt {
namespace gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_repoManager(std::make_unique<core::RepoManager>())
    , m_executor(std::make_unique<core::JobExecutor>())
{
    setupUI();
    loadRepositories();
    
    // Connect signals
    connect(m_executor.get(), &core::JobExecutor::stepOutput,
            this, &MainWindow::onJobOutput);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("GitHub Workflow Tool");
    resize(1000, 700);
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    
    // Top buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_cloneButton = new QPushButton("Clone Repository", this);
    QPushButton* refreshButton = new QPushButton("Refresh", this);
    
    buttonLayout->addWidget(m_cloneButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // Repository tree
    QLabel* repoLabel = new QLabel("Repositories:", this);
    mainLayout->addWidget(repoLabel);
    
    m_repoTree = new QTreeWidget(this);
    m_repoTree->setHeaderLabels(QStringList() << "Path");
    m_repoTree->setMaximumHeight(200);
    mainLayout->addWidget(m_repoTree);
    
    // Workflow tree
    QLabel* workflowLabel = new QLabel("Workflows:", this);
    mainLayout->addWidget(workflowLabel);
    
    m_workflowTree = new QTreeWidget(this);
    m_workflowTree->setHeaderLabels(QStringList() << "Workflow");
    m_workflowTree->setMaximumHeight(200);
    mainLayout->addWidget(m_workflowTree);
    
    // Execution controls
    QHBoxLayout* execLayout = new QHBoxLayout();
    m_runButton = new QPushButton("Run Workflow", this);
    m_backendCombo = new QComboBox(this);
    m_backendCombo->addItem("Container Backend");
    m_backendCombo->addItem("QEMU Backend");
    
    execLayout->addWidget(new QLabel("Backend:", this));
    execLayout->addWidget(m_backendCombo);
    execLayout->addWidget(m_runButton);
    execLayout->addStretch();
    
    mainLayout->addLayout(execLayout);
    
    // Output view
    QLabel* outputLabel = new QLabel("Output:", this);
    mainLayout->addWidget(outputLabel);
    
    m_outputView = new QTextEdit(this);
    m_outputView->setReadOnly(true);
    mainLayout->addWidget(m_outputView);
    
    // Connect signals
    connect(m_cloneButton, &QPushButton::clicked, this, &MainWindow::onCloneRepository);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshRepositories);
    connect(m_repoTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onRepositorySelected);
    connect(m_runButton, &QPushButton::clicked, this, &MainWindow::onRunWorkflow);
}

void MainWindow::loadRepositories() {
    m_repoTree->clear();
    QStringList repos = m_repoManager->listRepositories();
    
    for (const QString& repo : repos) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_repoTree);
        item->setText(0, repo);
        item->setData(0, Qt::UserRole, repo);
    }
}

void MainWindow::onCloneRepository() {
    bool ok;
    QString repoUrl = QInputDialog::getText(this, "Clone Repository",
                                           "Repository URL:",
                                           QLineEdit::Normal,
                                           "https://github.com/user/repo",
                                           &ok);
    
    if (ok && !repoUrl.isEmpty()) {
        m_outputView->append("Cloning: " + repoUrl);
        
        if (m_repoManager->cloneRepository(repoUrl)) {
            m_outputView->append("Successfully cloned");
            loadRepositories();
        } else {
            QMessageBox::warning(this, "Clone Failed", "Failed to clone repository");
        }
    }
}

void MainWindow::onRefreshRepositories() {
    loadRepositories();
}

void MainWindow::onRepositorySelected() {
    QList<QTreeWidgetItem*> selected = m_repoTree->selectedItems();
    if (selected.isEmpty()) {
        return;
    }
    
    QString repoPath = selected[0]->data(0, Qt::UserRole).toString();
    
    // Discover workflows
    core::WorkflowDiscovery discovery;
    QStringList workflows = discovery.discoverWorkflows(repoPath);
    
    m_workflowTree->clear();
    for (const QString& workflow : workflows) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_workflowTree);
        item->setText(0, QFileInfo(workflow).fileName());
        item->setData(0, Qt::UserRole, workflow);
    }
}

void MainWindow::onRunWorkflow() {
    QList<QTreeWidgetItem*> selected = m_workflowTree->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "No Workflow Selected", "Please select a workflow to run");
        return;
    }
    
    QString workflowPath = selected[0]->data(0, Qt::UserRole).toString();
    m_outputView->append("\n=== Running workflow: " + workflowPath + " ===\n");
    
    // Parse and execute
    core::WorkflowParser parser;
    core::Workflow workflow = parser.parse(workflowPath);
    
    if (parser.hasErrors()) {
        m_outputView->append("Parsing errors:");
        for (const QString& error : parser.getErrors()) {
            m_outputView->append("  " + error);
        }
        return;
    }
    
    bool useQemu = m_backendCombo->currentIndex() == 1;
    m_executor->executeWorkflow(workflow, "push", useQemu);
}

void MainWindow::onJobOutput(const QString& jobId, const QString& stepName, const QString& output) {
    Q_UNUSED(jobId);
    Q_UNUSED(stepName);
    m_outputView->append(output);
}

} // namespace gui
} // namespace gwt
