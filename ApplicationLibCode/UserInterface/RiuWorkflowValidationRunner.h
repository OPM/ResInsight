/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cafPdmPointer.h"

#include <QObject>
#include <QPointer>

class QProcess;
class QTimer;
class RimWorkflow;

//==================================================================================================
/// Checks the edited workflow with `taskmaestro workflow describe` in the background: a short
/// pause after the last edit, then `taskmaestro_helper save --describe` into the workflow's edit
/// folder. Results for an older revision are dropped.
//==================================================================================================
class RiuWorkflowValidationRunner : public QObject
{
    Q_OBJECT

public:
    static constexpr int debounceMs = 400;
    static constexpr int watchdogMs = 30000;

    explicit RiuWorkflowValidationRunner( QObject* parent = nullptr );
    ~RiuWorkflowValidationRunner() override;

    void schedule( RimWorkflow* workflow );
    void cancel();
    bool isRunning() const;

signals:
    void validationStarted();
    void validationFinished( RimWorkflow* workflow );

private:
    void start();
    void onFinished();
    void onTimeout();
    void stopProcess();

private:
    QTimer*                      m_debounceTimer;
    QTimer*                      m_watchdogTimer;
    QPointer<QProcess>           m_process;
    caf::PdmPointer<RimWorkflow> m_workflow;
    caf::PdmPointer<RimWorkflow> m_runningWorkflow;
    int                          m_runningRevision = -1;
};
