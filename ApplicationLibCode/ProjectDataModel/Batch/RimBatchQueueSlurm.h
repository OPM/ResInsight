/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "Hpc/RiaHpcTools.h"

#include "RimBatchQueue.h"

class RimProcessMonitor;

//==================================================================================================
///
///
//==================================================================================================
class RimBatchQueueSlurm : public RimBatchQueue
{
    CAF_PDM_HEADER_INIT;

public:
    RimBatchQueueSlurm( RiaDefines::BatchSchedulerType schedulerType = RiaDefines::BatchSchedulerType::SLURM );
    ~RimBatchQueueSlurm() override;

    void queueProcess( std::shared_ptr<RimProcess> process, int nProcessesPerTask ) override;
    void stopProcess() override;

protected:
    void setFinished( bool runOk ) override;

    void onJobInfoUpdate( const SignalEmitter* emitter, const std::map<QString, RiaHpcTools::BatchJobInformation>& information );

    virtual QString jobId() const;

    virtual QStringList generateCommand( QString script,
                                         QString jobName,
                                         QString workDir,
                                         QString stdOutLog,
                                         QString stdErrLog,
                                         QString queueName,
                                         bool    exclusive,
                                         int     numberOfProcesses,
                                         bool    blockingWait );

    QString launchScriptDirective() const override;

    void startJobAsync( QStringList arguments );

protected:
    std::shared_ptr<RimProcessMonitor> m_monitor;
    std::unique_ptr<RimProcess>        m_batchProcess;

private:
    bool    m_isBatchMonitorActivated;
    QString m_jobId;
};
