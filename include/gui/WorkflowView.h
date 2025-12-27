#pragma once

#include <QWidget>

namespace gwt {
namespace gui {

/**
 * @brief Widget for displaying workflow information
 */
class WorkflowView : public QWidget {
    Q_OBJECT

public:
    explicit WorkflowView(QWidget* parent = nullptr);
    ~WorkflowView() override;

    void loadWorkflow(const QString& workflowPath);

private:
    void setupUI();
};

} // namespace gui
} // namespace gwt
