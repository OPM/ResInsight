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
#include <QMap>
#include <QVector>
#include <QMutex>

class QListWidget;
class QPushButton;
class QComboBox;
class QProgressBar;
class QLabel;
class QTextEdit;
class QTableWidget;
class QComboBox;

class RicEclRunnerDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RicEclRunnerDialog( QWidget* parent = nullptr );
    ~RicEclRunnerDialog() override;

private slots:
    void slotImportFiles();
    void slotAddFile();
    void slotDeleteSelected();
    void slotOpenSelected();
    void slotRunSelected();
    void slotTableSelectionChanged();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);
    void slotProcessError(QProcess::ProcessError error);
    void slotTableCellDoubleClicked(int row, int column);
    void slotStopSelected();

signals:
    void fileOpenRequested(const QString& filePath);

private:
    void setupUi();
    void updateProgress();
    void startNextProcess();

    // Top area
    QPushButton* m_importButton;
    QPushButton* m_addButton; // add selected imported file to task list
    QComboBox*   m_triggerCombo; // model selector (e300/e100)
    QLabel*      m_triggerLabel;
    
    // Store pending files
    QStringList  m_pendingFiles; // files selected but not yet added to task list

    // Task list area
    QTableWidget* m_taskTable; // columns: FileName, Model, Status, Result
    QPushButton*  m_deleteButton;
    QPushButton*  m_openButton;
    QPushButton*  m_runButton; // runs selected tasks
    QPushButton*  m_stopButton; // stop selected task(s)
    QLabel*       m_runningLabel; // shows Running: X/Max

    // Bottom log area
    QTextEdit*    m_logOutput;

    // per-task storage (indexed by table row)
    QVector<QString> m_taskFiles;
    QVector<QString> m_taskModels;
    QVector<QString> m_taskStatus;
    QVector<QStringList> m_taskOutputs;
    QVector<QString> m_taskLogs;

    // Multi-process management
    static const int MAX_CONCURRENT_TASKS = 8;
    QMap<int, QProcess*> m_runningProcesses; // row -> process
    QList<int> m_runQueue; // row indices to run
    QMutex m_processMutex;

    // Keep single-process members for compatibility with existing logic
    // (some call sites still reference these). They are optional when
    // multi-process map is used, but restoring them keeps changes minimal.
    int m_currentTaskRow = -1; // currently running task row, or -1
    QProcess* m_currentProcess = nullptr;
    
    // helper to show running count
    void updateRunningLabel();
};
