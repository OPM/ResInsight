/////////////////////////////////////////////////////////////////////////////////
//
//  Simple dialog to select .data files and run eclrun/e300 on them
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicEclRunnerDialog.h"

#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
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

RicEclRunnerDialog::RicEclRunnerDialog( QWidget* parent )
    : QWidget( parent )
    , m_listWidget( nullptr )
    , m_importButton( nullptr )
    , m_runButton( nullptr )
    , m_triggerCombo( nullptr )
    , m_progressBar( nullptr )
    , m_triggerLabel( nullptr )
    , m_currentIndex( -1 )
    , m_currentProcess( nullptr )
{
    setObjectName("EclRunnerWidget");
    setupUi();
}

RicEclRunnerDialog::~RicEclRunnerDialog()
{
    if ( m_currentProcess )
    {
        m_currentProcess->kill();
        delete m_currentProcess;
    }
}

void RicEclRunnerDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_importButton = new QPushButton( tr("Import Data"), this );
    topLayout->addWidget( m_importButton );

    m_triggerLabel = new QLabel( tr("选择处理程序"), this );
    topLayout->addWidget( m_triggerLabel );

    m_triggerCombo = new QComboBox(this);
    m_triggerCombo->addItem("e300");
    m_triggerCombo->addItem("e100");
    m_triggerCombo->setCurrentIndex(0);
    topLayout->addWidget(m_triggerCombo);

    m_runButton = new QPushButton( tr("演算"), this );
    topLayout->addWidget( m_runButton );

    mainLayout->addLayout( topLayout );

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
    mainLayout->addWidget( m_listWidget );

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange( 0, 100 );
    m_progressBar->setValue( 0 );
    mainLayout->addWidget( m_progressBar );

    // Log output area
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(180);
    mainLayout->addWidget(m_logOutput);

    setLayout( mainLayout );

    connect( m_importButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotImportFiles );
    connect( m_runButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotRunClicked );
}

void RicEclRunnerDialog::slotImportFiles()
{
    QStringList files = QFileDialog::getOpenFileNames( this,
                                                       "Select .data files",
                                                       QString(),
                                                       "Data files (*.data);;All files (*.*)" );
    if ( files.isEmpty() ) return;

    for ( const QString& f : files )
    {
        if ( !m_filesToRun.contains( f ) )
        {
            m_filesToRun.append( f );
            m_listWidget->addItem( f );
        }
    }
}

void RicEclRunnerDialog::slotRunClicked()
{
    m_filesToRun.clear();
    for (QListWidgetItem* item : m_listWidget->selectedItems()) {
        m_filesToRun.append(item->text());
    }

    if (m_filesToRun.isEmpty()) {
        return;
    }

    // Reset progress and start processing
    m_currentIndex = 0;
    m_progressBar->setValue(0);
    startNextProcess();
}

void RicEclRunnerDialog::startNextProcess()
{
    if (m_currentIndex >= m_filesToRun.size()) {
        // All done
        m_progressBar->setValue(100);
        return;
    }

    if (m_currentProcess) {
        m_currentProcess->disconnect(this);
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }

    QString fileToRun = m_filesToRun.at(m_currentIndex);
    QFileInfo fileInfo(fileToRun);
    QString workingDir = fileInfo.absolutePath();
    // e300/eclrun will append the suffix ".DATA" itself. Pass the base name
    // (without any extensions) so we don't end up with "name.data.DATA".
    QString dataArg = fileInfo.completeBaseName();
    
    QString trigger = m_triggerCombo->currentText();
    QString exePath = (trigger == "e300") ? "D:/e300/e300.exe" : "D:/eclipse/eclrun.exe";

    m_currentProcess = new QProcess(this);
    m_currentProcess->setWorkingDirectory(workingDir);
    m_currentProcess->setProcessChannelMode(QProcess::MergedChannels);
    
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &RicEclRunnerDialog::slotProcessFinished);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &RicEclRunnerDialog::slotProcessError);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
        this, [this]() {
        QString output = m_currentProcess->readAllStandardOutput();
        m_currentProcessOutput += output;
        m_logOutput->append(output);
        qDebug() << "Process output:" << output;
        });

    QStringList args;
    args << dataArg;  // pass base name so external runner will open '<base>.DATA'

    qDebug() << "Starting in" << workingDir << ":" << exePath << args;
    m_logOutput->append(QString("Starting in %1 : %2 %3").arg(workingDir).arg(exePath).arg(args.join(' ')));

    // Check executable exists
    if (!QFile::exists(exePath)) {
        m_logOutput->append(QString("Executable not found: %1").arg(exePath));
        qDebug() << "Executable not found:" << exePath;
        // continue and let QProcess report FailedToStart
    }
    
    // 禁用运行按钮，避免重复执行
    m_runButton->setEnabled(false);
    m_runButton->setText(tr("正在运行..."));
    
    m_currentProcess->start(exePath, args);
    
    if (!m_currentProcess->waitForStarted(3000)) {
        qDebug() << "Failed to start process" << exePath;
        slotProcessError(QProcess::FailedToStart);
        m_runButton->setEnabled(true);
        m_runButton->setText(tr("演算"));
        return;
    }
    
    // 不在这里等待完成，让进程异步运行
    qDebug() << "Process started successfully, running asynchronously...";
    m_logOutput->append("Process started successfully, running asynchronously...");
}

void RicEclRunnerDialog::slotProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    QString currentFile = m_filesToRun.at(m_currentIndex);
    QFileInfo fileInfo(currentFile);
    QString baseName = fileInfo.completeBaseName();
    QString workingDir = fileInfo.absolutePath();
    
    // Append any remaining output
    QString remaining = m_currentProcess->readAllStandardOutput();
    if (!remaining.isEmpty()) {
        m_currentProcessOutput += remaining;
        m_logOutput->append(remaining);
    }

    if (status == QProcess::NormalExit && exitCode == 0) {
        qDebug() << "Process completed successfully for" << currentFile;
        m_logOutput->append(QString("Process completed successfully for %1").arg(currentFile));

        // 检查是否生成了关键文件（不区分大小写），并列出目录中与基名匹配的文件
        QDir dir(workingDir);
        QStringList candidates = dir.entryList(QStringList() << baseName + "*", QDir::Files);
        if (candidates.isEmpty()) {
            m_logOutput->append(QString("No output files found with prefix %1 in %2").arg(baseName).arg(workingDir));
            qDebug() << "No output files found with prefix" << baseName << "in" << workingDir;
        } else {
            m_logOutput->append(QString("Found %1 candidate output files:").arg(candidates.size()));
            for (const QString& c : candidates) {
                m_logOutput->append("  " + c);
                qDebug() << "Candidate output file:" << c;
            }
        }
    } else {
        qDebug() << "Process failed for" << currentFile << "with exit code" << exitCode;
        m_logOutput->append(QString("Process failed for %1 with exit code %2").arg(currentFile).arg(exitCode));
        QString errorOutput = m_currentProcess->readAllStandardError();
        if (!errorOutput.isEmpty()) {
            qDebug() << "Error output:" << errorOutput;
            m_logOutput->append(errorOutput);
        }
    }

    ++m_currentIndex;
    updateProgress();
    
    if (m_currentIndex >= m_filesToRun.size()) {
        // 所有文件处理完毕，恢复按钮状态
        m_runButton->setEnabled(true);
        m_runButton->setText(tr("演算"));
    } else {
        startNextProcess();
    }
}

void RicEclRunnerDialog::slotProcessError(QProcess::ProcessError error)
{
    QString currentFile = m_filesToRun.at(m_currentIndex);
    qDebug() << "Process error for" << currentFile << ":" << error;
    
    switch (error) {
        case QProcess::FailedToStart:
            qDebug() << "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions.";
            break;
        case QProcess::Crashed:
            qDebug() << "The process crashed some time after starting successfully.";
            break;
        default:
            qDebug() << "An error occurred while running the process.";
            break;
    }
    
    QString errorOutput = m_currentProcess->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qDebug() << "Error output:" << errorOutput;
    }

    ++m_currentIndex;
    updateProgress();
    startNextProcess();
}

void RicEclRunnerDialog::updateProgress()
{
    if ( m_filesToRun.isEmpty() )
    {
        m_progressBar->setValue( 0 );
        return;
    }

    int done = 0;
    if ( m_currentIndex <= 0 ) done = 0;
    else done = m_currentIndex;

    int total = m_filesToRun.size();
    int percent = ( total == 0 ) ? 0 : ( done * 100 ) / total;
    m_progressBar->setValue( percent );
}
