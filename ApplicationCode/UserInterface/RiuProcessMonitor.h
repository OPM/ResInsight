/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

class QDockWidget;
class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace caf
{
    class UiProcess;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuProcessMonitor : public QWidget
{
    Q_OBJECT

private:
    QLabel*         m_labelStatus;          // Shows current status string
    QPlainTextEdit* m_textEdit;             // Showing the textual output from the process
    QPushButton*    m_terminatePushButton;

    caf::UiProcess* m_monitoredProcess;     // Pointer to the process we're monitoring. Needed to fetch text

public:
    explicit RiuProcessMonitor(QDockWidget* pParent);
    ~RiuProcessMonitor() override;

    void                    startMonitorWorkProcess(caf::UiProcess* process);
    void                    stopMonitorWorkProcess();

    void                    addStringToLog(const QString& text);

public slots:
    void                    slotClearTextEdit();

private:
    void                    setStatusMsg(const QString& status, int messageType);

private slots:
    void                    slotShowProcStatusMsg(const QString& message, int messageType);
    void                    slotProcReadyReadStdOut();
    void                    slotProcReadyReadStdErr();
    void                    slotTerminateProcess();
};

