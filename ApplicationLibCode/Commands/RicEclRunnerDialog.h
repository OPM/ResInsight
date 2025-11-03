/////////////////////////////////////////////////////////////////////////////////
//
//  Simple dialog to select .data files and run eclrun/e300 on them
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QList>
#include <QProcess>
#include <QStringList>

class QListWidget;
class QPushButton;
class QComboBox;
class QProgressBar;
class QLabel;
class QTextEdit;

class RicEclRunnerDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RicEclRunnerDialog( QWidget* parent = nullptr );
    ~RicEclRunnerDialog() override;

private slots:
    void slotImportFiles();
    void slotRunClicked();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);
    void slotProcessError(QProcess::ProcessError error);

private:
    void setupUi();
    void updateProgress();
    void startNextProcess();

    QListWidget* m_listWidget;
    QPushButton* m_importButton;
    QPushButton* m_runButton;
    QComboBox*  m_triggerCombo;
    QProgressBar* m_progressBar;
    QLabel* m_triggerLabel;
    QTextEdit* m_logOutput;

    QStringList m_filesToRun;
    int m_currentIndex;
    QProcess* m_currentProcess;
    QString    m_currentProcessOutput;
};
