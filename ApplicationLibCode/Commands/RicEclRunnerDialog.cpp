/////////////////////////////////////////////////////////////////////////////////
//
//  Simple dialog to select .data files and run eclrun/e300 on them
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicEclRunnerDialog.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListWidget>
#include <QMutexLocker>
#include <QPointer>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QSizePolicy>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QDialog>
#include <QFontDatabase>
#include <algorithm>

RicEclRunnerDialog::RicEclRunnerDialog( QWidget* parent )
    : QWidget( parent )
    , m_importButton( nullptr )
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
{
    setObjectName( "EclRunnerWidget" );
    setupUi();
}

RicEclRunnerDialog::~RicEclRunnerDialog()
{
    // Ensure all running processes are stopped and deleted
    QMutexLocker locker( &m_processMutex );
    for ( auto it = m_runningProcesses.begin(); it != m_runningProcesses.end(); ++it )
    {
        QProcess* p = it.value();
        if ( p )
        {
            p->kill();
            p->deleteLater();
        }
    }
    m_runningProcesses.clear();
    if ( m_currentProcess )
    {
        m_currentProcess->kill();
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }
}

void RicEclRunnerDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );

    // Top: 添加DATA文件
    // Remove inline dialog-wide stylesheet. Use styleRole properties per-widget so QSS controls appearance.
    // Determine default dialog font to match Cell Selection Tool (use QDialog default font)
    QFont defaultFont = QDialog().font();

    // Compute standard button height based on dialog default font
    QPushButton tmpBtn;
    tmpBtn.setFont( defaultFont );
    int standardButtonHeight = tmpBtn.sizeHint().height();

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_importButton         = new QPushButton( tr( "Select File" ), this );
    m_importButton->setIcon( QIcon::fromTheme( "document-open" ) );
    m_importButton->setFont( defaultFont );
    if ( standardButtonHeight >0 ) m_importButton->setFixedHeight( standardButtonHeight );
    // Use styleRole so QSS can style this button according to current theme
    m_importButton->setProperty( "styleRole", "EclRunnerButton" );
    // Ensure no per-widget inline stylesheet/palette overrides remain
    m_importButton->setStyleSheet( QString() );
    m_importButton->setPalette( QApplication::palette() );
    topLayout->addWidget( m_importButton );

    // Increase distance between Select File and the process controls
    topLayout->addSpacing(12 );

    // Group Select Process label and combo in a compact inner layout
    QHBoxLayout* processLayout = new QHBoxLayout();
    processLayout->setSpacing(4 );
    processLayout->setContentsMargins(0,0,0,0 );

    m_triggerLabel = new QLabel( tr( "Select Process" ), this );
    m_triggerLabel->setFont( defaultFont );
    processLayout->addWidget( m_triggerLabel );

    m_triggerCombo = new QComboBox( this );
    m_triggerCombo->addItem( "e300" );
    m_triggerCombo->addItem( "e100" );
    m_triggerCombo->setCurrentText( "e300" ); // 设置默认值为e300
    m_triggerCombo->setFont( defaultFont );
    processLayout->addWidget( m_triggerCombo );

    topLayout->addLayout( processLayout );

    m_addButton = new QPushButton( tr( "Add Files" ), this );
    m_addButton->setFont( defaultFont );
    if ( standardButtonHeight >0 ) m_addButton->setFixedHeight( standardButtonHeight );
    m_addButton->setProperty( "styleRole", "EclRunnerButton" );
    m_addButton->setStyleSheet( QString() );
    m_addButton->setPalette( QApplication::palette() );
    topLayout->addWidget( m_addButton );

    // Make Select File and Add Files buttons match the combo box height (keeping widths/layout unchanged)
    // Ensure combo uses default font too
    // already set above

    // Use Select File button's font for several labels to ensure consistent font type
    QFont btnFont = defaultFont;
    // Create the title label and set its font to match the Select File button
    QLabel* topTitle = new QLabel( tr( "Add DATA Files" ), this );
    topTitle->setStyleSheet( "margin:5px0;" );
    // make title use same font as Select File button
    topTitle->setFont( btnFont );
    mainLayout->addWidget( topTitle );

    mainLayout->addLayout( topLayout );

    // Middle: 任务列表
    QLabel* middleTitle = new QLabel( tr( "Mission List" ), this );
    middleTitle->setFont( btnFont );
    mainLayout->addWidget( middleTitle );

    QHBoxLayout* taskHeader = new QHBoxLayout();
    // Header buttons
    m_deleteButton = new QPushButton( tr( "Delete" ), this );
    m_deleteButton->setEnabled( false );
    m_deleteButton->setFont( defaultFont );
    m_deleteButton->setProperty( "styleRole", "EclRunnerButton" );
    m_deleteButton->setStyleSheet( QString() );
    m_deleteButton->setPalette( QApplication::palette() );

    m_openButton = new QPushButton( tr( "Open" ), this );
    m_openButton->setEnabled( false );
    m_openButton->setFont( defaultFont );
    m_openButton->setProperty( "styleRole", "EclRunnerButton" );
    m_openButton->setStyleSheet( QString() );
    m_openButton->setPalette( QApplication::palette() );

    m_stopButton = new QPushButton( tr( "Stop" ), this );
    m_stopButton->setEnabled( false );
    m_stopButton->setFont( defaultFont );
    m_stopButton->setProperty( "styleRole", "EclRunnerButton" );
    m_stopButton->setStyleSheet( QString() );
    m_stopButton->setPalette( QApplication::palette() );

    m_runButton = new QPushButton( tr( "Caulculate" ), this );
    m_runButton->setEnabled( false );
    m_runButton->setFont( defaultFont );
    m_runButton->setProperty( "styleRole", "EclRunnerButton" );
    m_runButton->setStyleSheet( QString() );
    m_runButton->setPalette( QApplication::palette() );

    // Running count label
    m_runningLabel = new QLabel( tr( "Running:0/%1" ).arg( RicEclRunnerDialog::MAX_CONCURRENT_TASKS ), this );
    m_runningLabel->setAlignment( Qt::AlignCenter );
    m_runningLabel->setFont( defaultFont );

    // Make all controls expand horizontally and have equal stretch so they fill the header width equally
    m_deleteButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_openButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_stopButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_runButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_runningLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

    // Match header button heights to Cell Selection Tool button height
    if ( standardButtonHeight >0 )
    {
      m_deleteButton->setFixedHeight( standardButtonHeight );
      m_openButton->setFixedHeight( standardButtonHeight );
      m_stopButton->setFixedHeight( standardButtonHeight );
      m_runButton->setFixedHeight( standardButtonHeight );
      // keep running label default height; it will align with buttons vertically
    }

    // Add with equal stretch factors (1 each) so widths are equal
    taskHeader->addWidget( m_deleteButton,1 );
    taskHeader->addWidget( m_openButton,1 );
    taskHeader->addWidget( m_stopButton,1 );
    taskHeader->addWidget( m_runButton,1 );
    taskHeader->addWidget( m_runningLabel,1 );

    mainLayout->addLayout( taskHeader );

    m_taskTable = new QTableWidget( this );
    m_taskTable->setColumnCount( 4 );
    QStringList headers;
    headers << tr( "File Name" ) << tr( "Model" ) << tr( "Status" ) << tr( "Output" );
    m_taskTable->setHorizontalHeaderLabels( headers );
    m_taskTable->horizontalHeader()->setStretchLastSection( true );
    if ( m_taskTable->horizontalHeader() )
    {
        m_taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    m_taskTable->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_taskTable->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_taskTable->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_taskTable->setMinimumHeight( 160 );
    m_taskTable->setAlternatingRowColors( true );
    // Use styleRole and clear local inline stylesheet/palette
    m_taskTable->setProperty( "styleRole", "EclRunnerTable" );
    m_taskTable->setStyleSheet( QString() );
    m_taskTable->setPalette( QApplication::palette() );

    if ( m_taskTable->horizontalHeader() )
    {
        m_taskTable->horizontalHeader()->setFont( btnFont );
    }

    mainLayout->addWidget( m_taskTable );

    // Bottom: 任务日志
    QLabel* bottomTitle = new QLabel( tr( "Log" ), this );
    bottomTitle->setFont( btnFont );
    mainLayout->addWidget( bottomTitle );

    m_logOutput = new QTextEdit( this );
    m_logOutput->setReadOnly( true );
    // Use styleRole and clear inline styles so theme controls appearance
    m_logOutput->setProperty( "styleRole", "EclRunnerLog" );
    m_logOutput->setStyleSheet( QString() );
    m_logOutput->setPalette( QApplication::palette() );
    m_logOutput->setMaximumHeight( 220 );
    m_logOutput->setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );
    mainLayout->addWidget( m_logOutput );

    setLayout( mainLayout );

    // Connections
    connect( m_importButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotImportFiles );
    connect( m_addButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotAddFile );
    connect( m_deleteButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotDeleteSelected );
    connect( m_openButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotOpenSelected );
    connect( m_runButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotRunSelected );
    connect( m_stopButton, &QPushButton::clicked, this, &RicEclRunnerDialog::slotStopSelected );
    connect( m_taskTable, &QTableWidget::cellDoubleClicked, this, &RicEclRunnerDialog::slotTableCellDoubleClicked );
    connect( m_taskTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RicEclRunnerDialog::slotTableSelectionChanged );
}

void RicEclRunnerDialog::slotImportFiles()
{
    QStringList files = QFileDialog::getOpenFileNames( this, "Select .data files", QString(), "Data files (*.data);;All files (*.*)" );
    if ( files.isEmpty() ) return;

    // Store the selected files and overwrite any previous selection
    m_pendingFiles = files;
}

void RicEclRunnerDialog::slotAddFile()
{
    if ( m_pendingFiles.isEmpty() ) return;

    // Add all pending files to the task list
    for ( const QString& file : m_pendingFiles )
    {
        // avoid duplicates in task list
        if ( m_taskFiles.contains( file ) ) continue;

        int row = m_taskTable->rowCount();
        m_taskTable->insertRow( row );

        QFileInfo         fi( file );
        QTableWidgetItem* fileItem = new QTableWidgetItem( fi.fileName() );
        fileItem->setToolTip( file );
        m_taskTable->setItem( row, 0, fileItem );

        QString           model     = m_triggerCombo->currentText();
        QTableWidgetItem* modelItem = new QTableWidgetItem( model );
        m_taskTable->setItem( row, 1, modelItem );

        QTableWidgetItem* statusItem = new QTableWidgetItem( tr( "Pending" ) );
        m_taskTable->setItem( row, 2, statusItem );

        QTableWidgetItem* resultItem = new QTableWidgetItem( "" );
        m_taskTable->setItem( row, 3, resultItem );

        m_taskFiles.append( file );
        m_taskModels.append( model );
        m_taskStatus.append( QString( "Pending" ) );
        m_taskOutputs.append( QStringList() );
        m_taskLogs.append( QString() );
    }

    // Clear pending files after adding them to the task list
    m_pendingFiles.clear();

    updateProgress();
}

void RicEclRunnerDialog::slotDeleteSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // collect rows and sort descending
    QVector<int> rows;
    for ( const QModelIndex& idx : sel )
        rows.append( idx.row() );
    std::sort( rows.begin(), rows.end(), std::greater<int>() );

    for ( int r : rows )
    {
        // if currently running, ignore deletion
        if ( m_runningProcesses.contains( r ) ) continue;
        m_taskTable->removeRow( r );
        m_taskFiles.removeAt( r );
        m_taskModels.removeAt( r );
        m_taskStatus.removeAt( r );
        m_taskOutputs.removeAt( r );
        m_taskLogs.removeAt( r );
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

    for ( const QModelIndex& idx : sel )
    {
        int r = idx.row();
        if ( r < 0 || r >= m_taskOutputs.size() ) continue;
        for ( const QString& out : m_taskOutputs[r] )
        {
            if ( !out.isEmpty() ) emit fileOpenRequested( out );
        }
    }
}

void RicEclRunnerDialog::slotRunSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // append to queue only pending/failed tasks
    for ( const QModelIndex& idx : sel )
    {
        int r = idx.row();
        if ( r < 0 || r >= m_taskStatus.size() ) continue;
        QString st = m_taskStatus[r];
        if ( st == "Running" || st == "Queued" ) continue;
        m_runQueue.append( r );
        m_taskStatus[r] = "Queued";
        m_taskTable->item( r, 2 )->setText( "Queued" );
    }

    // Start as many processes as possible up to MAX_CONCURRENT_TASKS
    while ( !m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS )
    {
        startNextProcess();
    }
    updateRunningLabel();
}

void RicEclRunnerDialog::startNextProcess()
{
    QMutexLocker locker( &m_processMutex );

    if ( m_runQueue.isEmpty() ) return;

    // take next queued row
    int row = m_runQueue.takeFirst();
    if ( row < 0 || row >= m_taskFiles.size() )
    {
        // try next
        return;
    }

    // mark running
    m_taskStatus[row] = "Running";
    if ( m_taskTable->item( row, 2 ) ) m_taskTable->item( row, 2 )->setText( "Running" );

    QString   fileToRun = m_taskFiles[row];
    QFileInfo fileInfo( fileToRun );
    QString   workingDir = fileInfo.absolutePath();
    QString   dataArg    = fileInfo.completeBaseName();

    QString model   = m_taskModels[row];
    QString exePath = ( model == "e300" ) ? "D:/e300/e300.exe" : "D:/eclipse/eclrun.exe";

    QProcess* proc = new QProcess( this );
    proc->setWorkingDirectory( workingDir );
    proc->setProcessChannelMode( QProcess::MergedChannels );

    // capture output per-process
    connect( proc,
             &QProcess::readyReadStandardOutput,
             this,
             [this, proc, row]()
             {
                 QString output = proc->readAllStandardOutput();
                 if ( row >= 0 && row < m_taskLogs.size() )
                 {
                     m_taskLogs[row] += output;
                     QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
                     if ( !sel.isEmpty() && sel.first().row() == row )
                     {
                         m_logOutput->append( output );
                     }
                 }
             } );

    connect( proc, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &RicEclRunnerDialog::slotProcessFinished );
    connect( proc, &QProcess::errorOccurred, this, &RicEclRunnerDialog::slotProcessError );

    QStringList args;
    args << dataArg;

    qDebug() << "Starting in" << workingDir << ":" << exePath << args;
    m_taskLogs[row] += QString( "Starting in %1 : %2 %3\n" ).arg( workingDir ).arg( exePath ).arg( args.join( ' ' ) );

    if ( !QFile::exists( exePath ) )
    {
        m_taskLogs[row] += QString( "Executable not found: %1\n" ).arg( exePath );
        qDebug() << "Executable not found:" << exePath;
    }

    // store running process
    m_runningProcesses.insert( row, proc );
    updateRunningLabel();

    m_runButton->setEnabled( false );
    m_runButton->setText( tr( "Running..." ) );

    // start asynchronously; rely on signals for started/finished/error
    connect( proc,
             &QProcess::started,
             this,
             [this, row, exePath, workingDir, args]()
             {
                 m_taskLogs[row] +=
                     QString( "Process started successfully in %1 : %2 %3\n" ).arg( workingDir ).arg( exePath ).arg( args.join( ' ' ) );
             } );

    proc->start( exePath, args );
}

void RicEclRunnerDialog::slotProcessFinished( int exitCode, QProcess::ExitStatus status )
{
    QProcess* proc = qobject_cast<QProcess*>( sender() );
    if ( !proc ) return;

    // find the row for this process
    int row = -1;
    {
        QMutexLocker locker( &m_processMutex );
        for ( auto it = m_runningProcesses.constBegin(); it != m_runningProcesses.constEnd(); ++it )
        {
            if ( it.value() == proc )
            {
                row = it.key();
                break;
            }
        }
    }

    if ( row < 0 || row >= m_taskFiles.size() )
    {
        // cleanup and continue
        proc->deleteLater();
        QMutexLocker locker( &m_processMutex );
        m_runningProcesses.remove( row );
        startNextProcess();
        return;
    }

    // Append remaining output
    QString remaining = proc->readAllStandardOutput();
    if ( !remaining.isEmpty() )
    {
        m_taskLogs[row] += remaining;
        QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
        if ( !sel.isEmpty() && sel.first().row() == row )
        {
            m_logOutput->append( remaining );
        }
    }

    QString   currentFile = m_taskFiles[row];
    QFileInfo fileInfo( currentFile );
    QString   baseName   = fileInfo.completeBaseName();
    QString   workingDir = fileInfo.absolutePath();

    if ( status == QProcess::NormalExit && exitCode == 0 )
    {
        m_taskStatus[row] = "Done";
        if ( m_taskTable->item( row, 2 ) ) m_taskTable->item( row, 2 )->setText( "Done" );
        m_taskLogs[row] += QString( "Process completed successfully for %1\n" ).arg( currentFile );

        // find outputs (.egrid / .grid) case-insensitive
        QDir               dir( workingDir );
        QStringList        foundFiles;
        QStringList        allFiles    = dir.entryList( QDir::Files );
        QString            basePattern = QRegularExpression::escape( baseName );
        QRegularExpression egridPattern( basePattern + "\\.(egrid)$", QRegularExpression::CaseInsensitiveOption );
        QRegularExpression gridPattern( basePattern + "\\.(grid)$", QRegularExpression::CaseInsensitiveOption );
        for ( const QString& f : allFiles )
        {
            if ( egridPattern.match( f ).hasMatch() || gridPattern.match( f ).hasMatch() )
            {
                foundFiles << dir.absoluteFilePath( f );
            }
        }

        m_taskOutputs[row] = foundFiles;

        if ( !foundFiles.isEmpty() )
        {
            QComboBox* cb = new QComboBox( m_taskTable );
            for ( const QString& p : foundFiles )
                cb->addItem( p );
            m_taskTable->setCellWidget( row, 3, cb );
        }
    }
    else
    {
        m_taskStatus[row] = "Failed";
        if ( m_taskTable->item( row, 2 ) ) m_taskTable->item( row, 2 )->setText( "Failed" );
        QString errorOutput = proc->readAllStandardError();
        if ( !errorOutput.isEmpty() )
        {
            m_taskLogs[row] += errorOutput;
        }
    }

    // cleanup
    QMutexLocker locker( &m_processMutex );
    m_runningProcesses.remove( row );
    proc->deleteLater();

    updateRunningLabel();
    updateProgress();

    // start more if queued
    while ( !m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS )
    {
        startNextProcess();
    }

    // If nothing is running and nothing queued, restore run button state
    if ( m_runningProcesses.isEmpty() && m_runQueue.isEmpty() )
    {
        m_runButton->setEnabled( true );
        m_runButton->setText( tr( "Caulculate" ) );
    }
}

void RicEclRunnerDialog::slotProcessError( QProcess::ProcessError error )
{
    QProcess* proc = qobject_cast<QProcess*>( sender() );
    if ( !proc ) return;

    int row = -1;
    {
        QMutexLocker locker( &m_processMutex );
        for ( auto it = m_runningProcesses.constBegin(); it != m_runningProcesses.constEnd(); ++it )
        {
            if ( it.value() == proc )
            {
                row = it.key();
                break;
            }
        }
    }

    if ( row >= 0 && row < m_taskFiles.size() )
    {
        m_taskStatus[row] = "Error";
        if ( m_taskTable->item( row, 2 ) ) m_taskTable->item( row, 2 )->setText( "Error" );
        QString errorOutput = proc->readAllStandardError();
        if ( !errorOutput.isEmpty() ) m_taskLogs[row] += errorOutput;
    }

    QMutexLocker locker( &m_processMutex );
    m_runningProcesses.remove( row );
    proc->deleteLater();

    updateRunningLabel();
    updateProgress();

    // start next queued tasks if any
    while ( !m_runQueue.isEmpty() && m_runningProcesses.size() < MAX_CONCURRENT_TASKS )
    {
        startNextProcess();
    }

    // If nothing is running and nothing queued, restore run button state
    if ( m_runningProcesses.isEmpty() && m_runQueue.isEmpty() )
    {
        m_runButton->setEnabled( true );
        m_runButton->setText( tr( "Caulculate" ) );
    }
}

void RicEclRunnerDialog::slotTableSelectionChanged()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() )
    {
        m_deleteButton->setEnabled( false );
        m_openButton->setEnabled( false );
        m_runButton->setEnabled( false );
        if ( m_stopButton ) m_stopButton->setEnabled( false );
        m_logOutput->clear();
        return;
    }

    m_deleteButton->setEnabled( true );
    m_openButton->setEnabled( true );
    m_runButton->setEnabled( true );
    // Stop enabled if any selected row is queued or running
    bool canStop = false;
    for ( const QModelIndex& idx : sel )
    {
        int r = idx.row();
        if ( r >= 0 && r < m_taskStatus.size() )
        {
            QString st = m_taskStatus[r];
            if ( st == "Running" || st == "Queued" )
            {
                canStop = true;
                break;
            }
        }
        if ( m_runningProcesses.contains( r ) )
        {
            canStop = true;
            break;
        }
    }
    if ( m_stopButton ) m_stopButton->setEnabled( canStop );

    int firstRow = sel.first().row();
    if ( firstRow >= 0 && firstRow < m_taskLogs.size() )
    {
        m_logOutput->setPlainText( m_taskLogs[firstRow] );
    }
    else
    {
        m_logOutput->clear();
    }
}

void RicEclRunnerDialog::slotStopSelected()
{
    QModelIndexList sel = m_taskTable->selectionModel()->selectedRows();
    if ( sel.isEmpty() ) return;

    // For each selected row: if running, terminate process; if queued, remove from queue
    for ( const QModelIndex& idx : sel )
    {
        int r = idx.row();
        if ( r < 0 || r >= m_taskStatus.size() ) continue;

        // If running, terminate process
        if ( m_runningProcesses.contains( r ) )
        {
            QProcess* p = m_runningProcesses.value( r, nullptr );
            if ( p )
            {
                // try graceful terminate first (non-blocking)
                p->terminate();
                // schedule a forced kill if it hasn't exited after 2s
                QPointer<QProcess> pp( p );
                QTimer::singleShot( 2000,
                                    this,
                                    [pp]()
                                    {
                                        if ( pp && pp->state() != QProcess::NotRunning )
                                        {
                                            pp->kill();
                                        }
                                    } );
            }
            // mark as cancelled; leave mapping in place so finished() handler can clean up
            m_taskStatus[r] = "Cancelled";
            if ( m_taskTable->item( r, 2 ) ) m_taskTable->item( r, 2 )->setText( "Cancelled" );
        }
        else
        {
            // If queued, remove from queue
            if ( m_runQueue.contains( r ) )
            {
                m_runQueue.removeAll( r );
                m_taskStatus[r] = "Cancelled";
                if ( m_taskTable->item( r, 2 ) ) m_taskTable->item( r, 2 )->setText( "Cancelled" );
            }
        }
    }

    // Update running label and state
    updateRunningLabel();
    updateProgress();

    // If nothing running, restore run button
    if ( m_runningProcesses.isEmpty() && m_runQueue.isEmpty() )
    {
        m_runButton->setEnabled( true );
        m_runButton->setText( tr( "Caulculate" ) );
    }
}

void RicEclRunnerDialog::slotTableCellDoubleClicked( int row, int /*column*/ )
{
    if ( row < 0 || row >= m_taskFiles.size() ) return;

    // If outputs are available for this task, open them; otherwise open the input data file
    if ( row < m_taskOutputs.size() && !m_taskOutputs[row].isEmpty() )
    {
        for ( const QString& out : m_taskOutputs[row] )
        {
            if ( !out.isEmpty() ) emit fileOpenRequested( out );
        }
    }
    else
    {
        emit fileOpenRequested( m_taskFiles[row] );
    }
}

void RicEclRunnerDialog::updateRunningLabel()
{
    int running = m_runningProcesses.size();
    if ( m_runningLabel )
    {
        m_runningLabel->setText( tr( "Running: %1/%2" ).arg( running ).arg( MAX_CONCURRENT_TASKS ) );
    }
}

void RicEclRunnerDialog::updateProgress()
{
    // Progress bar was removed; keep this function as a lightweight status updater.
    // Currently it does not update a visual progress bar.
    Q_UNUSED( m_taskFiles );
    Q_UNUSED( m_taskStatus );
}
