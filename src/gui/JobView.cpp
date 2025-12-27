#include "gui/JobView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace gwt {
namespace gui {

JobView::JobView(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

JobView::~JobView() = default;

void JobView::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Job View", this);
    layout->addWidget(label);
}

void JobView::setJobInfo(const QString& jobId, const QString& status) {
    // TODO: Implement job details display
    // - Show job ID and name
    // - Display job status with color coding
    // - Show step progress
    // - Display logs for individual steps
    Q_UNUSED(jobId);
    Q_UNUSED(status);
}

} // namespace gui
} // namespace gwt
