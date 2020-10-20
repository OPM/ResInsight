//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafProgressInfo.h"
#include "cafAssert.h"
#include "cafMemoryInspector.h"
#include "cafProgressState.h"

#include <QApplication>
#include <QCoreApplication>
#include <QPointer>
#include <QProgressDialog>
#include <QThread>

#include <algorithm>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ProgressTask::ProgressTask( ProgressInfo& parentTask )
    : m_parentTask( parentTask )
{
}
ProgressTask::~ProgressTask()
{
    m_parentTask.incrementProgress();
}

//==================================================================================================
///
/// \class caf::ProgressInfo
/// This class provides a simple frontend to the Qt progressbar, allowing distributed
/// progress calculation.
///
/// Create an instance of this object in the method/function that needs progress information
/// Then call incrementProgress() or setProgress() at proper times in your method.
/// When the method returns, the ProgressInfo destructor will clean up and finish.
/// The real beauty is that this class will magically interact with possible ProgressInfo instances
/// in functions that your method calls, providing a complete, consistent and detailed progress bar
///
/// caf::ProgressInfo progInfo(3, "Open File");
/// progInfo.setProgressDescription("Reading");
/// ...readFile()
/// progInfo.incrementProgress();
/// progInfo.setProgressDescription("Validating");
/// ... validateData();
/// progInfo.incrementProgress();
/// progInfo.setProgressDescription("Building geometry");
///  ... buildGeometry();
/// progInfo.incrementProgress(); // not needed really, because the destructor will send the progress to top.
///
/// There are one particular limitation: The progress will not work correctly if the higher level
/// ProgressInfo object does not increment progress between the creation and operation of two (or more)
/// independent lower level ProgressInfo objects. If not, the progress will restart (within its limits)
/// for each progress object that is operating.
///
/// caf::ProgressInfo progInfoHighLevel(3, "Open File");
///
/// {
///     caf::ProgressInfo progInfoLowLevel(10, "");
/// }
/// // NEEDS progInfoHighLevel.incrementProgress() here !!
/// {
///     caf::ProgressInfo progInfoLowLevel(10, "");
/// }
///
/// It is not allowed to have several ProgressInfo objects in the same scope level
///
/// caf::ProgressInfo progInfo1(10, "");
/// caf::ProgressInfo progInfo2(10, ""); //<-- Will not work well
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// The title will be written centered in the dialog, under other progress titles in action.
/// If you do not need a title for a particular level, simply pass "" and it will be ignored.
/// \sa setProgressDescription
//--------------------------------------------------------------------------------------------------
ProgressInfo::ProgressInfo( size_t maxProgressValue, const QString& title, bool delayShowingProgress )
{
    ProgressInfoStatic::start( maxProgressValue, title, delayShowingProgress );

    if ( dynamic_cast<QApplication*>( QCoreApplication::instance() ) )
    {
        QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ProgressInfo::~ProgressInfo()
{
    ProgressInfoStatic::finished();

    if ( dynamic_cast<QApplication*>( QCoreApplication::instance() ) )
    {
        QApplication::restoreOverrideCursor();
    }
}

//--------------------------------------------------------------------------------------------------
/// Sets a description of the step currently being executed.
/// Used to create a nice text in the progressDialog, by appending " : <your text>"
/// to the current levels title. If no title is requested for this level, the ":" is omitted.
/// So: One line per level that has title and/or description.
///     in the format:
///     <Title>: <Description>
/// The ":" is only there when needed
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgressDescription( const QString& description )
{
    ProgressInfoStatic::setProgressDescription( description );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgress( size_t progressValue )
{
    ProgressInfoStatic::setProgress( progressValue );
}

//--------------------------------------------------------------------------------------------------
/// Increments progress by 1, or by the amount set by setNextProgressStepSize
//--------------------------------------------------------------------------------------------------
void ProgressInfo::incrementProgress()
{
    ProgressInfoStatic::incrementProgress();
}

//--------------------------------------------------------------------------------------------------
/// To make a certain operation span more of the progress bar than one tick,
/// set the number of progress ticks that you want it to use before calling it.
/// Eg.
/// caf::ProgressInfo progInfo(5, "Doing things");
/// // ... Do one small thing
/// progInfo.incrementProgress();
/// progInfo.setNextProgressStepSize(3);
/// // ... Some heavy function call with its own progress handling
/// progInfo.incrementProgress();
///
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setNextProgressIncrement( size_t nextStepSize )
{
    ProgressInfoStatic::setNextProgressIncrement( nextStepSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ProgressTask ProgressInfo::task( const QString& description, int stepSize )
{
    setProgressDescription( description );
    setNextProgressIncrement( stepSize );
    return caf::ProgressTask( *this );
}

//==================================================================================================
///
/// Internal file-scope private functions to implement the progress dialog
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString createMemoryLabelText()
{
    uint64_t currentUsage        = caf::MemoryInspector::getApplicationPhysicalMemoryUsageMiB();
    uint64_t totalPhysicalMemory = caf::MemoryInspector::getTotalPhysicalMemoryMiB();

    float currentUsageFraction = 0.0f;
    if ( currentUsage > 0u && totalPhysicalMemory > 0u )
    {
        currentUsageFraction = std::min( 1.0f, static_cast<float>( currentUsage ) / totalPhysicalMemory );
    }

    QString labelText( "\n" );
    if ( currentUsageFraction > 0.5 )
    {
        labelText =
            QString( "Memory Used: %1 MiB, Total Physical Memory: %2 MiB\n" ).arg( currentUsage ).arg( totalPhysicalMemory );
    }
    return labelText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static QProgressDialog* progressDialog()
{
    static QPointer<QProgressDialog> progDialog;
    if ( progDialog.isNull() && dynamic_cast<QApplication*>( QCoreApplication::instance() ) )
    {
        progDialog = new QProgressDialog( nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint );

        progDialog->hide();
        progDialog->setAutoClose( false );
        progDialog->setAutoReset( false );
        progDialog->setMinimumWidth( 400 );
    }
    return progDialog;
}

//--------------------------------------------------------------------------------------------------
/// A static vector containing the maximum values for the progress on each sublevel (call stack level)
//--------------------------------------------------------------------------------------------------
static std::vector<size_t>& maxProgressStack()
{
    static std::vector<size_t> m_maxProgressStack;

    return m_maxProgressStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& titleStack()
{
    static std::vector<QString> m_titleStack;

    return m_titleStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& descriptionStack()
{
    static std::vector<QString> m_descriptionStack;

    return m_descriptionStack;
}

//--------------------------------------------------------------------------------------------------
/// The actual progress value on each level (call stack level) 0 corresponds to the outermost function
//--------------------------------------------------------------------------------------------------
static std::vector<size_t>& progressStack()
{
    static std::vector<size_t> m_progressStack;

    return m_progressStack;
}

//--------------------------------------------------------------------------------------------------
/// The number of progress ticks (span) on each callstack level that the level deeper (in callstack) shall fill
/// used to balance the progress, making some (heavy) operations span more of the progress bar
//--------------------------------------------------------------------------------------------------
static std::vector<size_t>& progressSpanStack()
{
    static std::vector<size_t> m_progressSpanStack;

    return m_progressSpanStack;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the total maximum value for the progress bar composed
/// of the complete stack
//--------------------------------------------------------------------------------------------------
static size_t currentTotalMaxProgressValue()
{
    std::vector<size_t>& maxProgressStack_v = maxProgressStack();

    size_t levCount = 1;
    for ( size_t levelDepth = 0; levelDepth < maxProgressStack_v.size(); ++levelDepth )
    {
        levCount *= maxProgressStack_v[levelDepth];
    }
    return levCount;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the total progress value based on the current level subdivision and progress
//--------------------------------------------------------------------------------------------------
static size_t currentTotalProgress()
{
    double progress = 0;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();
    std::vector<size_t>& maxProgressStack_v  = maxProgressStack();

    for ( int i = static_cast<int>( progressStack_v.size() ) - 1; i >= 0; --i )
    {
        size_t span = ( i < 1 ) ? 1 : progressSpanStack_v[i - 1];
        progress    = span * ( progress + progressStack_v[i] ) / (double)maxProgressStack_v[i];
    }

    size_t totalIntProgress = static_cast<size_t>( currentTotalMaxProgressValue() * progress );

    return totalIntProgress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static QString currentComposedLabel()
{
    QString labelText;

    for ( size_t i = 0; i < titleStack().size(); ++i )
    {
        if ( !titleStack()[i].isEmpty() ) labelText += titleStack()[i];
        if ( !titleStack()[i].isEmpty() && !descriptionStack()[i].isEmpty() ) labelText += ": ";
        if ( !descriptionStack()[i].isEmpty() ) labelText += descriptionStack()[i];
        if ( !( titleStack()[i].isEmpty() && descriptionStack()[i].isEmpty() ) ) labelText += "\n";
    }
    labelText += createMemoryLabelText();
    return labelText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ProgressState::isActive()
{
    return !maxProgressStack().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4668 )
// Define this one to tell windows.h to not define min() and max() as macros
#if defined WIN32 && !defined NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#pragma warning( pop )
#endif

void openDebugWindow()
{
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
    AllocConsole();

    FILE*   consoleFilePointer;
    errno_t err;

    err = freopen_s( &consoleFilePointer, "conin$", "r", stdin );
    err = freopen_s( &consoleFilePointer, "conout$", "w", stdout );
    err = freopen_s( &consoleFilePointer, "conout$", "w", stderr );

#pragma warning( pop )
#endif
}

void reportError( const std::string& errorMsg )
{
    openDebugWindow();
    std::cout << "Error in caf::ProgressInfo :" << std::endl;
    std::cout << errorMsg << std::endl;
    std::cout << "Current progress state:" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << currentComposedLabel().toStdString();
    std::cout << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "Prog\tMax\tSpan" << std::endl;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& maxProgressStack_v  = maxProgressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();

    size_t level             = 0;
    bool   hasMoreProgLevels = level < progressStack_v.size();
    bool   hasMoreMaxLevels  = level < maxProgressStack_v.size();
    bool   hasMoreSpanLevels = level < progressSpanStack_v.size();

    while ( hasMoreProgLevels || hasMoreMaxLevels || hasMoreSpanLevels )
    {
        if ( hasMoreProgLevels )
            std::cout << progressStack_v[level] << "\t";
        else
            std::cout << "--"
                      << "\t";
        if ( hasMoreMaxLevels )
            std::cout << maxProgressStack_v[level] << "\t";
        else
            std::cout << "--"
                      << "\t";
        if ( hasMoreSpanLevels )
            std::cout << progressSpanStack_v[level] << "\t";
        else
            std::cout << "--"
                      << "\t";

        std::cout << std::endl;
        ++level;
        hasMoreProgLevels = level < progressStack_v.size();
        hasMoreMaxLevels  = level < maxProgressStack_v.size();
        hasMoreSpanLevels = level < progressSpanStack_v.size();
    }
}

//==================================================================================================
///
/// \class caf::ProgressInfoBlocker
///
/// Used to disable progress info on a temporary basis
///
//==================================================================================================

ProgressInfoBlocker::ProgressInfoBlocker()
{
    ProgressInfoStatic::s_disabled = true;
}

ProgressInfoBlocker::~ProgressInfoBlocker()
{
    ProgressInfoStatic::s_disabled = false;
}

//==================================================================================================
///
/// \class caf::ProgressInfoStatic
///
///
///
//==================================================================================================

bool ProgressInfoStatic::s_disabled = false;
bool ProgressInfoStatic::s_running  = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::start( size_t maxProgressValue, const QString& title, bool delayShowingProgress )
{
    if ( !isUpdatePossible() ) return;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();
    std::vector<size_t>& maxProgressStack_v  = maxProgressStack();

    QProgressDialog* dialog = progressDialog();

    if ( !maxProgressStack_v.size() )
    {
        // progressDialog()->setWindowModality(Qt::ApplicationModal);
        if ( dialog )
        {
            dialog->setMinimum( 0 );
            dialog->setWindowTitle( title );
            dialog->setCancelButton( nullptr );
            if ( delayShowingProgress )
            {
                dialog->setMinimumDuration( 1000 );
            }
            else
            {
                dialog->show();
            }
        }
    }
    s_running = true;
    maxProgressStack_v.push_back( maxProgressValue );
    progressStack_v.push_back( 0 );
    progressSpanStack_v.push_back( 1 );
    titleStack().push_back( title );
    descriptionStack().push_back( "" );

    if ( dialog )
    {
        dialog->setMaximum( static_cast<int>( currentTotalMaxProgressValue() ) );
        dialog->setValue( static_cast<int>( currentTotalProgress() ) );
        dialog->setLabelText( currentComposedLabel() );
    }
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    // if (progressDialog()) progressDialog()->repaint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgressDescription( const QString& description )
{
    if ( !isUpdatePossible() ) return;

    descriptionStack().back() = description;

    QProgressDialog* dialog = progressDialog();
    if ( dialog )
    {
        dialog->setLabelText( currentComposedLabel() );
    }
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    // if (progressDialog()) progressDialog()->repaint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgress( size_t progressValue )
{
    if ( !isUpdatePossible() ) return;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();
    std::vector<size_t>& maxProgressStack_v  = maxProgressStack();

    if ( progressValue == progressStack_v.back() ) return; // Do nothing if no progress.

    // Guard against the max value set for this level
    if ( progressValue > maxProgressStack_v.back() )
    {
        reportError( "setProgress() is called with a progressValue > max, progressValue == " +
                     std::to_string( progressValue ) );
        progressValue = maxProgressStack_v.back();
    }

    progressStack_v.back()     = progressValue;
    progressSpanStack_v.back() = 1;

    int totalProgress    = static_cast<int>( currentTotalProgress() );
    int totalMaxProgress = static_cast<int>( currentTotalMaxProgressValue() );

    if ( static_cast<int>( totalProgress ) > totalMaxProgress )
    {
        reportError( "totalProgress > totalMaxProgress"
                     ", totalProgress == " +
                     std::to_string( totalProgress ) + ", totalMaxProgress == " + std::to_string( totalMaxProgress ) );
        totalProgress = totalMaxProgress;
    }

    QProgressDialog* dialog = progressDialog();
    if ( dialog )
    {
        dialog->setMaximum( totalMaxProgress );
        dialog->setValue( totalProgress );
    }

    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    // if (progressDialog()) progressDialog()->repaint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::incrementProgress()
{
    if ( !isUpdatePossible() ) return;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();

    CAF_ASSERT( progressStack_v.size() );
    ProgressInfoStatic::setProgress( progressStack_v.back() + progressSpanStack_v.back() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setNextProgressIncrement( size_t nextStepSize )
{
    if ( !isUpdatePossible() ) return;

    CAF_ASSERT( progressSpanStack().size() );
    std::vector<size_t>& maxProgressStack_v = maxProgressStack();
    std::vector<size_t>& progressStack_v    = progressStack();

    // Guard against the max value set for this level
    if ( ( progressStack_v.back() + nextStepSize ) > maxProgressStack_v.back() )
    {
        reportError( "setNextProgressIncrement() is using a too high increment(" + std::to_string( nextStepSize ) + ").\n" +
                     "It will result in a total progress of " + std::to_string( progressStack_v.back() + nextStepSize ) +
                     "\nwhich is past the max limit: " + std::to_string( maxProgressStack_v.back() ) );

        nextStepSize = maxProgressStack_v.back() - progressStack_v.back();
    }

    progressSpanStack().back() = nextStepSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ProgressInfoStatic::isRunning()
{
    return s_running;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::finished()
{
    if ( !isUpdatePossible() ) return;

    std::vector<size_t>& progressStack_v     = progressStack();
    std::vector<size_t>& progressSpanStack_v = progressSpanStack();
    std::vector<size_t>& maxProgressStack_v  = maxProgressStack();

    CAF_ASSERT( maxProgressStack_v.size() && progressStack_v.size() && progressSpanStack_v.size() &&
                titleStack().size() && descriptionStack().size() );

    // Set progress to max value, and leave it there until somebody touches the progress again

    ProgressInfoStatic::setProgress( maxProgressStack_v.back() );

    // Pop all the stacks
    maxProgressStack_v.pop_back();
    progressStack_v.pop_back();
    progressSpanStack_v.pop_back();
    titleStack().pop_back();
    descriptionStack().pop_back();

    // Update the text to reflect the "previous level"
    QProgressDialog* dialog = progressDialog();
    if ( dialog )
    {
        dialog->setLabelText( currentComposedLabel() );
    }

    // If we are finishing the last level, clean up
    if ( maxProgressStack_v.empty() )
    {
        if ( dialog )
        {
            dialog->reset();
            dialog->close();
            s_running = false;
        }
    }
    else
    {
        // Make sure the Gui is repainted
        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        // if (progressDialog()) progressDialog()->repaint();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setEnabled( bool enable )
{
    s_disabled = !enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ProgressInfoStatic::isUpdatePossible()
{
    if ( s_disabled ) return false;

    if ( dynamic_cast<QApplication*>( QCoreApplication::instance() ) )
    {
        QProgressDialog* dialog = progressDialog();
        if ( dialog )
        {
            return dialog->thread() == QThread::currentThread();
        }
    }
    return false;
}

} // namespace caf
