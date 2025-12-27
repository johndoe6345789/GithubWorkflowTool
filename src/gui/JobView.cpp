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
    Q_UNUSED(jobId);
    Q_UNUSED(status);
    // Implementation for job details
}

} // namespace gui
} // namespace gwt
