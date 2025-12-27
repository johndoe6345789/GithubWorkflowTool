#include "gui/WorkflowView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace gwt {
namespace gui {

WorkflowView::WorkflowView(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

WorkflowView::~WorkflowView() = default;

void WorkflowView::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Workflow View", this);
    layout->addWidget(label);
}

void WorkflowView::loadWorkflow(const QString& workflowPath) {
    Q_UNUSED(workflowPath);
    // Implementation for detailed workflow view
}

} // namespace gui
} // namespace gwt
