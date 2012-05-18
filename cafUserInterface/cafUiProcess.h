//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include <QtCore/QProcess>
#include <QDateTime>

namespace caf
{

const int PROCESS_STATE_NORMAL  = 0;    // Normal messages
const int PROCESS_STATE_RUNNING = 1;    // Messages sent as long as the process is running 
const int PROCESS_STATE_ERROR   = 2;    // Message sent for error messages


//=================================================================================================================================
//
// 
//=================================================================================================================================
class UiProcess : public QProcess
{
    Q_OBJECT

private:
    QTime       m_timer;

public:
    UiProcess(QObject* pParent = 0);

private:
    void    doEmitStatusMsg(const QString& msg, int statusMsgType);

private slots:
    void    slotProcStarted();
    void    slotProcError(QProcess::ProcessError error);
    void    slotProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void    slotProcStateChanged(QProcess::ProcessState newState);
    void    slotUpdateStatusMessage();

signals:
    void    signalStatusMsg(const QString& msg, int statusMsgType);
    void    signalFormattedStatusMsg(const QString& msg);
};


} // end namespace caf
