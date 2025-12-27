#pragma once

#include <QWidget>

namespace gwt {
namespace gui {

/**
 * @brief Widget for displaying job execution details
 */
class JobView : public QWidget {
    Q_OBJECT

public:
    explicit JobView(QWidget* parent = nullptr);
    ~JobView() override;

    void setJobInfo(const QString& jobId, const QString& status);

private:
    void setupUI();
};

} // namespace gui
} // namespace gwt
