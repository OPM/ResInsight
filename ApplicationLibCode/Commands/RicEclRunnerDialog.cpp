/////////////////////////////////////////////////////////////////////////////////
//
//  Simple dialog to select .data files and run eclrun/e300 on them
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicEclRunnerDialog.h"
//#include <Windows.h>
#include <chrono>
#include <thread>

#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QProcess>
#include <QLabel>
#include <QDebug>
#include <QTextEdit>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QMutexLocker>
#include <QIcon>
#include <algorithm>
#include <QTimer>
#include <QPointer>
#include <QDateTime>
#include <QDialog>

RicEclRunnerDialog::RicEclRunnerDialog( QWidget* parent )
    : QWidget( parent )
    , m_addButton( nullptr )
    , m_triggerCombo( nullptr )
    , m_triggerLabel( nullptr )
    , m_taskTable( nullptr )
    , m_deleteButton( nullptr )
    , m_openButton( nullptr )
    , m_runButton( nullptr )
    , m_stopButton( nullptr )
    , m_runningLabel( nullptr )
    , m_logOutput( nullptr )
    , m_logFlushTimer(nullptr)
    , m_detailButton(nullptr)
{
    setObjectName("EclRunnerWidget");
    setupUi();
}

RicEclRunnerDialog::~RicEclRunnerDialog()
{
    // Ensure all running processes are stopped and deleted
    QMutexLocker locker(&m_processMutex);
    for (auto it = m_runningProcesses.begin(); it != m_runningProcesses.end(); ++it) {
        QProcess* p = it.value();
        if (p) {
            p->kill();
            p->deleteLater();
        }
    }
    m_runningProcesses.clear();
    if ( m_currentProcess ) {
        m_currentProcess->kill();
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }
}

void RicEclRunnerDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Top: 添加DATA文件
    // 设置整体样式
    setStyleSheet(
        "QWidget {"
        "    font-size:10pt;"
        "}"
        "QPushButton {"
        "    padding:5px15px;"
        "    background-color: #f0f0f0;"
        "    border:1px solid #c0c0c0;"
        "    border-radius:3px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #d0d0d0;"
        "}"
        "QLabel {"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    margin-top:10px;"
        "}"
        "QComboBox {"
        "    padding:5px;"
        "    border:1px solid #c0c0c0;"
        "    border-radius:3px;"
        "}"
    );

    QLabel* topTitle = new QLabel(tr("Add DATA Files"), this);
    topTitle->setStyleSheet("font-size:9pt; margin:5px0;");
    mainLayout->addWidget(topTitle);

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_triggerLabel = new QLabel( tr("Select Process"), this );
    topLayout->addWidget( m_triggerLabel );

    m_triggerCombo = new QComboBox(this);
    m_triggerCombo->addItem("Y-3");
    m_triggerCombo->addItem("Y-1");
    m_triggerCombo->setCurrentText("Y-3"); // 设置默认值为e300
    topLayout->addWidget(m_triggerCombo);

    m_addButton = new QPushButton(tr("Add Files"), this);
    topLayout->addWidget(m_addButton);

    mainLayout->addLayout(topLayout);

    // Middle: 任务列表
    QLabel* middleTitle = new QLabel(tr("Mission List"), this);
    mainLayout->addWidget(middleTitle);

    QHBoxLayout* taskHeader = new QHBoxLayout();
    taskHeader->setSpacing(8);
    taskHeader->addStretch();

    // Create a small grouped area for task control buttons and style them
    m_deleteButton = new QPushButton(tr("Delete"), this);
    m_openButton = new QPushButton(tr("Open"), this);
    m_stopButton = new QPushButton(tr("Stop"), this);
    m_runButton = new QPushButton(tr("Caulculate"), this);

    // Uniform sizes for nicer layout
    const int btnMinW =90;
    for ( QPushButton* b : { m_deleteButton, m_openButton, m_stopButton, m_runButton } ) {
        b->setMinimumWidth(btnMinW);
        b->setCursor(Qt::PointingHandCursor);
        b->setEnabled(false);
    }

    // Add buttons in logical order (Delete, Open, Stop, Run)
    taskHeader->addWidget(m_deleteButton);
    taskHeader->addWidget(m_openButton);
    taskHeader->addWidget(m_stopButton);
    taskHeader->addWidget(m_runButton);

    // Running count label
    m_runningLabel = new QLabel(tr("Running:0/%1").arg(RicEclRunnerDialog::MAX_CONCURRENT_TASKS), this);
    m_runningLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_runningLabel->setMinimumWidth(140);
    taskHeader->addWidget(m_runningLabel);
    mainLayout->addLayout(taskHeader);

    m_taskTable = new QTableWidget(this);
    m_taskTable->setColumnCount(4);
    QStringList headers;
    headers << tr("File Name") << tr("Model") << tr("Status") << tr("Output");
    m_taskTable->setHorizontalHeaderLabels(headers);
    m_taskTable->horizontalHeader()->setStretchLastSection(true);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTable->setMinimumHeight(160);
    m_taskTable->setAlternatingRowColors(true);
    m_taskTable->setStyleSheet(
        "QTableWidget {"
        "    gridline-color: #d0d0d0;"
        "    background-color: white;"
        "    alternate-background-color: #f8f8f8;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #0078d7;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f0f0f0;"
        "    padding:5px;"
        "    border: none;"
        "    border-right:1px solid #d0d0d0;"
        "    border-bottom:1px solid #d0d0d0;"
        "}"
    );
    mainLayout->addWidget(m_taskTable);

    // Bottom: 任务日志
    QLabel* bottomTitle = new QLabel(tr("Log"), this);
    mainLayout->addWidget(bottomTitle);

    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f8f8f8;"
        "    border:1px solid #d0d0d0;"
        "    font-family: Consolas, monospace;"
        "}"
    );
    m_logOutput->setMaximumHeight(220);

    // add detail button above log area (right aligned)
    QHBoxLayout* logHeader = new QHBoxLayout();
    logHeader->addStretch();
    m_detailButton = new QPushButton(tr("Detail"), this);
    m_detailButton->setMinimumWidth(80);
    m_detailButton->setEnabled(false);
    logHeader->addWidget(m_detailButton);
    mainLayout->addLayout(logHeader);

    mainLayout->addWidget(m_logOutput);

    // make layout margins and spacing responsive
    mainLayout->setContentsMargins(8,8,8,8);
    mainLayout->setSpacing(8);

    // Stretch factors so widgets resize proportionally with the window
    // Layout indices:0:topTitle,1:topLayout,2:middleTitle,3:taskHeader,4:m_taskTable,5:bottomTitle,6:m_logOutput
    mainLayout->setStretch(4,6); // task table gets most of extra space
    mainLayout->setStretch(6,3); // log area grows but less than table

    setLayout(mainLayout);

    // create log flush timer
    m_logFlushTimer = new QTimer(this);
    m_logFlushTimer->setInterval(200); // flush every200ms
    connect(m_logFlushTimer, &QTimer::timeout, this, &RicEclRunnerDialog::flushLogs);
    m_logFlushTimer->start();

    // Connections
    connect(m_addButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotAddFile);
    connect(m_deleteButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotDeleteSelected);
    connect(m_openButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotOpenSelected);
    connect(m_runButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotRunSelected);
    connect(m_stopButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotStopSelected);
    connect(m_taskTable, &QTableWidget::cellDoubleClicked, this, &RicEclRunnerDialog::slotTableCellDoubleClicked);
    connect(m_taskTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RicEclRunnerDialog::slotTableSelectionChanged);
    connect(m_detailButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotShowDetails);
}

void RicEclRunnerDialog::slotAddFile()
{
    // Directly open file dialog and add selected files to task list
    QStringList files = QFileDialog::getOpenFileNames( this,
                                                       tr("Select .data files"),
                                                       QString(),
                                                       tr("Data files (*.data);;All files (*.*)") );
    if ( files.isEmpty() ) return;

    for ( const QString& file : files )
    {
        // avoid duplicates in task list
        if ( m_taskFiles.contains(file) ) continue;

        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        QFileInfo fi(file);
        QTableWidgetItem* fileItem = new QTableWidgetItem(fi.fileName());
        fileItem->setToolTip(file);
        m_taskTable->setItem(row,0, fileItem);

        QString model = m_triggerCombo->currentText();
        QTableWidgetItem* modelItem = new QTableWidgetItem(model);
        m_taskTable->setItem(row,1, modelItem);

        QTableWidgetItem* statusItem = new QTableWidgetItem(tr("Pending"));
        m_taskTable->setItem(row,2, statusItem);

        QTableWidgetItem* resultItem = new QTableWidgetItem("");
        m_taskTable->setItem(row,3, resultItem);

        m_taskFiles.append(file);
        m_taskModels.append(model);
        m_taskStatus.append(QString("Pending"));
        m_taskOutputs.append(QStringList());
        m_taskLogs.append(QString());
        m_logShownLen.append(0);
    }

    // Clear any previously pending files - not used in this flow
    m_pendingFiles.clear();

    updateProgress();
}

void RicEclRunnerDialog::slotDeleteSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // collect rows and sort descending
    QVector<int> rows;
    for ( const QModelIndex& idx : sel ) rows.append(idx.row());
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for ( int r : rows ) {
        // if currently running, ignore deletion
        if ( m_runningProcesses.contains(r) ) continue;
        m_taskTable->removeRow(r);
        m_taskFiles.removeAt(r);
        m_taskModels.removeAt(r);
        m_taskStatus.removeAt(r);
        m_taskOutputs.removeAt(r);
        m_taskLogs.removeAt(r);
        m_logShownLen.removeAt(r);
    }

    m_taskTable->clearSelection();
    // Clearing queue to avoid stale row indices after removals
    m_runQueue.clear();
    updateProgress();
}

void RicEclRunnerDialog::slotOpenSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    for ( const QModelIndex& idx : sel ) {
        int r = idx.row();
        if ( r < 0 || r >= m_taskOutputs.size() ) continue;

        QComboBox* outputFiles = dynamic_cast<QComboBox*>( m_taskTable->cellWidget( r, 3 ) ); // clear previous output widget if any
        QString fileName = outputFiles->currentText();
        if ( !fileName.isEmpty() ) emit fileOpenRequested( m_outputFileNamePathMap[fileName] );
    }
}

void RicEclRunnerDialog::slotRunSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // append to queue only pending/failed tasks
    for ( const QModelIndex& idx : sel ) {
        int r = idx.row();
        if ( r < 0 || r >= m_taskStatus.size() ) continue;
        QString st = m_taskStatus[r];
        if ( st == "Running" || st == "Queued" ) continue;
        m_runQueue.append(r);
        m_taskStatus[r] = "Queued";
        m_taskTable->item(r,2)->setText("Queued");
    }

    // Start as many processes as possible up to MAX_CONCURRENT_TASKS
    while (!m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS) {
        startNextProcess();
    }
    updateRunningLabel();

    // Update buttons to reflect new queued/running state immediately
    slotTableSelectionChanged();
}

void RicEclRunnerDialog::startNextProcess()
{
    QMutexLocker locker(&m_processMutex);

    if (m_runQueue.isEmpty()) return;

    // take next queued row
    int row = m_runQueue.takeFirst();
    if ( row <0 || row >= m_taskFiles.size() ) {
        // try next
        return;
    }

    // mark running
    m_taskStatus[row] = "Running";
    if ( m_taskTable->item(row,2) ) m_taskTable->item(row,2)->setText("Running");

    QString fileToRun = m_taskFiles[row];
    QFileInfo fileInfo(fileToRun);
    QString workingDir = fileInfo.absolutePath();
    QString dataArg = fileInfo.completeBaseName();

    QString model = m_taskModels[row];
    QString exePath = (model == "Y-3") ? "D:/e300/e300.exe" : "D:/eclipse/eclrun.exe";

    QProcess* proc = new QProcess(this);
    proc->setWorkingDirectory(workingDir);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // capture output per-process
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc, row]() {
        QString output = proc->readAllStandardOutput();
        if ( row >=0 && row < m_taskLogs.size() ) {
            // buffer output; avoid frequent direct UI updates
            m_taskLogs[row] += output;
        }
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[proc-out] row" << row << "len" << output.size();
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RicEclRunnerDialog::slotProcessFinished);
    connect(proc, &QProcess::errorOccurred, this, &RicEclRunnerDialog::slotProcessError);

    QStringList args;
    args << dataArg;

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Starting in" << workingDir << ":" << exePath << args;
    m_taskLogs[row] += QString("Starting in %1 : %2 %3\n").arg(workingDir).arg(exePath).arg(args.join(' '));

    if (!QFile::exists(exePath)) {
        m_taskLogs[row] += QString("Executable not found: %1\n").arg(exePath);
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Executable not found:" << exePath;
    }

    // store running process
    m_runningProcesses.insert(row, proc);
    updateRunningLabel();

    m_runButton->setEnabled(false);
    m_runButton->setText(tr("Running..."));

    // start asynchronously; rely on signals for started/finished/error
    connect(proc, &QProcess::started, this, [this, row, exePath, workingDir, args]() {
        m_taskLogs[row] += QString("Process started successfully in %1 : %2 %3\n").arg(workingDir).arg(exePath).arg(args.join(' '));
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Process started row" << row;
    });

    proc->start(exePath, args);

    // Immediately refresh selection-based button states
    slotTableSelectionChanged();

}

void RicEclRunnerDialog::slotProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;

    // find the row for this process
    int row = -1;
    {
        QMutexLocker locker(&m_processMutex);
        for (auto it = m_runningProcesses.constBegin(); it != m_runningProcesses.constEnd(); ++it) {
            if ( it.value() == proc ) { row = it.key(); break; }
        }
    }

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Process finished signal for row" << row << "exit" << exitCode << "status" << status;

    if ( row <0 || row >= m_taskFiles.size() ) {
        // cleanup and continue
        proc->deleteLater();
        QMutexLocker locker(&m_processMutex);
        m_runningProcesses.remove(row);
        startNextProcess();
        return;
    }

    // Append remaining output
    QString remaining = proc->readAllStandardOutput();
    if (!remaining.isEmpty()) {
        m_taskLogs[row] += remaining;
    }

    QString currentFile = m_taskFiles[row];
    QFileInfo fileInfo(currentFile);
    QString baseName = fileInfo.completeBaseName();
    QString workingDir = fileInfo.absolutePath();

    if (status == QProcess::NormalExit && exitCode ==0) {
        m_taskStatus[row] = "Done";
        if ( m_taskTable->item(row,2) ) m_taskTable->item(row,2)->setText("Done");
        m_taskLogs[row] += QString("Process completed successfully for %1\n").arg(currentFile);

        // find outputs (.egrid / .grid) case-insensitive
        QDir dir(workingDir);
        QStringList foundFiles;
        QStringList allFiles = dir.entryList(QDir::Files);
        QString basePattern = QRegularExpression::escape(baseName);
        QRegularExpression egridPattern(basePattern + "\\.(egrid)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression gridPattern(basePattern + "\\.(grid)$", QRegularExpression::CaseInsensitiveOption);
        for ( const QString& f : allFiles ) {
            if ( egridPattern.match(f).hasMatch() || gridPattern.match(f).hasMatch() ) {
                foundFiles << dir.absoluteFilePath(f);
            }
        }

        m_taskOutputs[row] = foundFiles;

        if ( !foundFiles.isEmpty() ) {
            QComboBox* cb = new QComboBox(m_taskTable);
            for ( const QString& p : foundFiles )
            {
                qsizetype idx = p.lastIndexOf( '/' );
                QString fileName = p.mid( idx + 1 );
                m_outputFileNamePathMap[fileName] = p;
                cb->addItem( fileName );
            }
            m_taskTable->setCellWidget(row,3, cb);
        }
    } else {
        m_taskStatus[row] = "Failed";
        if ( m_taskTable->item(row,2) ) m_taskTable->item(row,2)->setText("Failed");
        QString errorOutput = proc->readAllStandardError();
        if (!errorOutput.isEmpty()) {
            m_taskLogs[row] += errorOutput;
        }
    }

    // cleanup
    QMutexLocker locker(&m_processMutex);
    m_runningProcesses.remove(row);
    proc->deleteLater();

    updateRunningLabel();
    updateProgress();

    // start more if queued
    while (!m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS) {
        startNextProcess();
    }

    // If nothing is running and nothing queued, restore run button state
    if (m_runningProcesses.isEmpty() && m_runQueue.isEmpty()) {
        m_runButton->setEnabled(true);
        m_runButton->setText(tr("Caulculate"));
    }

    // Ensure buttons reflect the new state for any currently selected rows
    slotTableSelectionChanged();
}

void RicEclRunnerDialog::slotProcessError(QProcess::ProcessError error)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;

    int row = -1;
    {
        QMutexLocker locker(&m_processMutex);
        for (auto it = m_runningProcesses.constBegin(); it != m_runningProcesses.constEnd(); ++it) {
            if ( it.value() == proc ) { row = it.key(); break; }
        }
    }

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Process error for row" << row << "err" << error;

    if ( row >=0 && row < m_taskFiles.size() ) {
        m_taskStatus[row] = "Error";
        if ( m_taskTable->item(row,2) ) m_taskTable->item(row,2)->setText("Error");
        QString errorOutput = proc->readAllStandardError();
        if (!errorOutput.isEmpty()) m_taskLogs[row] += errorOutput;
    }

    QMutexLocker locker(&m_processMutex);
    m_runningProcesses.remove(row);
    proc->deleteLater();

    updateRunningLabel();
    updateProgress();

    // start next queued tasks if any
    while (!m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS) {
        startNextProcess();
    }

    // If nothing is running and nothing queued, restore run button state
    if (m_runningProcesses.isEmpty() && m_runQueue.isEmpty()) {
        m_runButton->setEnabled(true);
        m_runButton->setText(tr("Caulculate"));
    }
}

void RicEclRunnerDialog::slotTableSelectionChanged()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) {
        m_deleteButton->setEnabled(false);
        m_openButton->setEnabled(false);
        m_runButton->setEnabled(false);
        if (m_stopButton) m_stopButton->setEnabled(false);
        m_logOutput->clear();
        if (m_detailButton) m_detailButton->setEnabled(false);
        return;
    }

    m_deleteButton->setEnabled(false);
    m_openButton->setEnabled(false);
    m_runButton->setEnabled(false);
    if (m_stopButton) m_stopButton->setEnabled(false);
    if (m_detailButton) m_detailButton->setEnabled(false);

    // Determine button enabled state based on selected rows' statuses.
    // Rules:
    // - If any selected row is Running or Queued: Stop enabled; Run/Delete/Open disabled.
    // - Otherwise: Stop disabled. Run enabled if any selected row is Pending/Failed/Cancelled.
    // Delete enabled only if none selected are Running/Queued. Open enabled if at least one selected
    // has outputs and none are Running/Queued.

    bool anyRunningOrQueued = false;
    bool anyPendingOrFailedOrCancelled = false;
    bool anyDoneWithOutputs = false;
    bool anyHasOutputs = false;
    bool anyHasLogs = false;

    for ( const QModelIndex& idx : sel ) {
        int r = idx.row();
        if ( r <0 || r >= m_taskStatus.size() ) continue;
        QString st = m_taskStatus[r];
        if ( st == "Running" || st == "Queued" ) {
            anyRunningOrQueued = true;
        }
        if ( st == "Pending" || st == "Failed" || st == "Cancelled" ) {
            anyPendingOrFailedOrCancelled = true;
        }
        if ( st == "Done" ) {
            // done may or may not have outputs
            if ( r < m_taskOutputs.size() && !m_taskOutputs[r].isEmpty() ) anyDoneWithOutputs = true;
        }
        if ( r < m_taskOutputs.size() && !m_taskOutputs[r].isEmpty() ) anyHasOutputs = true;
        if ( r < m_taskLogs.size() && !m_taskLogs[r].isEmpty() ) anyHasLogs = true;
        // also consider runningProcesses map for safety
        if ( m_runningProcesses.contains(r) ) anyRunningOrQueued = true;
    }

    // Stop button
    if ( m_stopButton ) m_stopButton->setEnabled(anyRunningOrQueued);

    // If any running/queued selected, disable run/delete/open as requested
    if ( anyRunningOrQueued ) {
        m_runButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        m_openButton->setEnabled(false);
    } else {
        // Run enabled if any pending/failed/cancelled selected
        m_runButton->setEnabled(anyPendingOrFailedOrCancelled);

        // Delete enabled when none are running/queued (already ensured)
        m_deleteButton->setEnabled(true);

        // Open enabled if any selected has outputs
        m_openButton->setEnabled(anyHasOutputs);
    }

    // Detail button enabled if any selected row has logs
    if ( m_detailButton ) m_detailButton->setEnabled(anyHasLogs);

    int firstRow = sel.first().row();
    if ( firstRow >=0 && firstRow < m_taskLogs.size() ) {
        // show full buffered log immediately
        m_logOutput->setPlainText(m_taskLogs[firstRow]);
        // remember we've shown all so flush won't re-show
        m_logShownLen[firstRow] = m_taskLogs[firstRow].size();
    } else {
        m_logOutput->clear();
    }
}

void RicEclRunnerDialog::slotStopSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // Collect running processes and queued rows while holding the mutex to avoid races.
    QVector<QPointer<QProcess>> procsToStop;
    QVector<int> runningRows;
    QVector<int> queuedRows;

    {
        QMutexLocker locker(&m_processMutex);
        for ( const QModelIndex& idx : sel ) {
            int r = idx.row();
            if ( r <0 || r >= m_taskStatus.size() ) continue;

            if ( m_runningProcesses.contains(r) ) {
                QProcess* p = m_runningProcesses.value(r, nullptr);
                if ( p ) {
                    procsToStop.append(QPointer<QProcess>(p));
                    runningRows.append(r);
                }
                // mark as cancelled here logically; UI update later
            } else {
                // If queued, remove from queue
                if ( m_runQueue.contains(r) ) {
                    m_runQueue.removeAll(r);
                    queuedRows.append(r);
                }
            }
        }
    }

    // Update statuses and UI AFTER releasing the mutex to avoid holding the lock during UI operations.
    for ( int r : queuedRows ) {
        if ( r >=0 && r < m_taskStatus.size() ) {
            m_taskStatus[r] = "Cancelled";
            if ( m_taskTable->item(r,2) ) m_taskTable->item(r,2)->setText("Cancelled");
        }
    }
    for ( int r : runningRows ) {
        if ( r >=0 && r < m_taskStatus.size() ) {
            m_taskStatus[r] = "Cancelled";
            if ( m_taskTable->item(r,2) ) m_taskTable->item(r,2)->setText("Cancelled");
        }
    }

    // Terminate running processes without holding the mutex. Use QPointer to avoid touching deleted objects.
    for ( const QPointer<QProcess>& pp : procsToStop ) {
        if ( pp ) 
        {
            // try graceful terminate first (non-blocking)
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "Terminating process" << pp.data();

            qint64 ppId = pp->processId();
            QProcess::execute( "taskkill", { "/F", "/PID", QString::number( pp->processId() ) } );
        }
    }

    // Update running label and state
    updateRunningLabel();
    updateProgress();

    // If nothing running, restore run button
    {
        QMutexLocker locker(&m_processMutex);
        if (m_runningProcesses.isEmpty() && m_runQueue.isEmpty()) {
            m_runButton->setEnabled(true);
            m_runButton->setText(tr("Caulculate"));
        }
    }

    // Refresh buttons for current selection after cancellation
    slotTableSelectionChanged();
}

void RicEclRunnerDialog::slotTableCellDoubleClicked(int row, int /*column*/)
{
    if ( row <0 || row >= m_taskFiles.size() ) return;

    // If outputs are available for this task, open them; otherwise open the input data file
    if ( row < m_taskOutputs.size() && !m_taskOutputs[row].isEmpty() ) 
    {
        QComboBox* outputFiles = dynamic_cast<QComboBox*>( m_taskTable->cellWidget( row, 3 ) ); // clear previous output widget if any
        QString fileName = outputFiles->currentText();
        if ( !fileName.isEmpty() ) emit fileOpenRequested( m_outputFileNamePathMap[fileName] );
    } else {
        emit fileOpenRequested(m_taskFiles[row]);
    }
}

void RicEclRunnerDialog::updateRunningLabel()
{
    int running = m_runningProcesses.size();
    if (m_runningLabel) {
        m_runningLabel->setText(tr("Running: %1/%2").arg(running).arg(MAX_CONCURRENT_TASKS));
    }
}

void RicEclRunnerDialog::updateProgress()
{
    // Progress bar was removed; keep this function as a lightweight status updater.
    // Currently it does not update a visual progress bar.
    Q_UNUSED(m_taskFiles);
    Q_UNUSED(m_taskStatus);
}

// flush buffered logs to the QTextEdit for the currently selected row
void RicEclRunnerDialog::flushLogs()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;
    int row = sel.first().row();
    if ( row <0 || row >= m_taskLogs.size() ) return;

    const QString& buf = m_taskLogs[row];
    int shown =0;
    if ( row < m_logShownLen.size() ) shown = m_logShownLen[row];
    if ( shown < buf.size() ) {
        QString newText = buf.mid(shown);
        // append without adding extra newlines
        m_logOutput->moveCursor(QTextCursor::End);
        m_logOutput->insertPlainText(newText);
        m_logShownLen[row] = buf.size();
    }
}

// Show a modal dialog with parsed log details from selected task
void RicEclRunnerDialog::slotShowDetails()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;
    int row = sel.first().row();
    if ( row <0 || row >= m_taskLogs.size() ) return;

    QString log = m_taskLogs[row];
    if ( log.isEmpty() ) return;

    // Parse log lines into category (warning/message/error) and message
    QStringList lines = log.split('\n', Qt::SkipEmptyParts);

    // Create dialog
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Log Details"));
    dlg.setModal(true);
    dlg.resize(800,400);
    dlg.setWindowFlags( dlg.windowFlags() | Qt::WindowMinMaxButtonsHint );
    dlg.show();

    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    QTableWidget* table = new QTableWidget(&dlg);
    table->setColumnCount(2);
    QStringList headers;
    headers << tr("Level") << tr("Message");
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->setRowCount(0);

    QRegularExpression warnRe("\\bwarn(?:ing)?\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression errRe("\\berror\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression msgRe("\\bmessage\\b", QRegularExpression::CaseInsensitiveOption);

    for ( qsizetype i = 0;i < lines.size();)
    {
        const QString& l = lines[i];
        // clean up common prefixes/suffixes (e.g. lines starting with "@--" and ending with ".")
        QString cleaned = l.trimmed();
        if ( !cleaned.startsWith("@") ) {
            lines.removeAt( i );
            continue;
        }
        i++;
    }

    QString info = "";
    for (auto l : lines)
    {
        QString cleaned = l.trimmed().removeAt(0);
        if (cleaned.startsWith("--"))
        {
            cleaned = cleaned.remove( 0, 2 );
            int r   = table->rowCount();

            if (info.size())
            {
                QTableWidgetItem* msgItem = new QTableWidgetItem( info );
                table->setItem( r - 1, 1, msgItem );
                info = "";
            }
            table->insertRow( r );
            QString level = tr( "Message" );
            
            QTableWidgetItem* levelItem = new QTableWidgetItem( "" );
            table->setItem( r, 0, levelItem );
            if (errRe.match(cleaned).hasMatch())
            {
                level = tr( "Error" );
                cleaned.remove( 0, 5 ); // remove leading "Error"
                QTableWidgetItem* item = table->item( r, 0 );
                item->setBackground( Qt::red );
            }
            else if (warnRe.match(cleaned).hasMatch())
            {
                level = tr( "Warning" );
                cleaned.remove( 0, 7 );
                QTableWidgetItem* item = table->item( r, 0 );
                item->setBackground( Qt::yellow );
            }
            else if (msgRe.match(cleaned).hasMatch())
            {
                cleaned.remove( 0, 8 );
                level = tr( "Message" );
            }

            levelItem->setText(level);
        }
        info += cleaned;


        //if ( cleaned.startsWith( "--" ) && !info.size() )
        //{
        //    cleaned = cleaned.remove( 0, 2 ); // remove leading "@--"
        //    QString level = tr( "Message" );
        //    if ( errRe.match( cleaned ).hasMatch() )
        //        level = tr( "Error" );
        //    else if ( warnRe.match( cleaned ).hasMatch() )
        //        level = tr( "Warning" );
        //    else if ( msgRe.match( cleaned ).hasMatch() )
        //        level = tr( "Message" );
        //    int r = table->rowCount();
        //    table->insertRow( r );
        //    QTableWidgetItem* levelItem = new QTableWidgetItem( level );
        //    table->setItem( r, 0, levelItem );
        //}
        //else if ( cleaned.startsWith( "--" ) && info.size() )
        //{
        //    int r = table->rowCount() - 1;
        //    QTableWidgetItem* msgItem   = new QTableWidgetItem( info );
        //    table->setItem( r, 1, msgItem );
        //    info = "";

        //    cleaned = cleaned.remove( 0, 2 ); // remove leading "@--"
        //    QString level = tr( "Message" );
        //    if ( errRe.match( cleaned ).hasMatch() )
        //        level = tr( "Error" );
        //    else if ( warnRe.match( cleaned ).hasMatch() )
        //        level = tr( "Warning" );
        //    else if ( msgRe.match( cleaned ).hasMatch() )
        //        level = tr( "Message" );
        //    r++;
        //    table->insertRow( r );
        //    QTableWidgetItem* levelItem = new QTableWidgetItem( level );
        //    table->setItem( r, 0, levelItem );
        //}
        //info += cleaned;
    }

    layout->addWidget(table);

    dlg.exec();
}
