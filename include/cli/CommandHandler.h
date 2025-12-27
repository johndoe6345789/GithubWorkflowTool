#pragma once

#include <QObject>
#include <QStringList>
#include <memory>

namespace gwt {
namespace core {
class RepoManager;
class JobExecutor;
}

namespace cli {

/**
 * @brief Handles command-line interface commands
 */
class CommandHandler : public QObject {
    Q_OBJECT

public:
    explicit CommandHandler(QObject* parent = nullptr);
    ~CommandHandler() override;

    /**
     * @brief Execute a command with arguments
     * @param args Command line arguments
     * @return Exit code
     */
    int execute(const QStringList& args);

private:
    std::unique_ptr<core::RepoManager> m_repoManager;
    std::unique_ptr<core::JobExecutor> m_executor;

    void printHelp() const;
    int handleClone(const QStringList& args);
    int handleList(const QStringList& args);
    int handleRun(const QStringList& args);
    int handleWorkflows(const QStringList& args);
    int handleDoctor(const QStringList& args);
};

} // namespace cli
} // namespace gwt
