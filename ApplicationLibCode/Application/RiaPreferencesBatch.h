/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaHpcDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesBatch : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaPreferencesBatch();

    static RiaPreferencesBatch* current();

    void appendItems( caf::PdmUiOrdering& uiOrdering );

    size_t                             maxParallelJobs() const;
    RiaDefines::BatchSchedulerType     batchScheduler() const;
    QString                            queueName() const;
    QString                            batchSchedulerOptions() const;
    bool                               exclusiveJob() const;
    RiaDefines::BatchJobMonitoringType jobMonitoring() const;

    static QString selectQueue( RiaDefines::BatchSchedulerType scheduler, QString currentQueue );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<size_t>  m_maxParallelJobs;
    caf::PdmField<QString> m_queueName;
    caf::PdmField<QString> m_additionalLsfOptions;
    caf::PdmField<QString> m_additionalSlurmOptions;
    caf::PdmField<bool>    m_exclusive;
    caf::PdmField<int>     m_updateInterval;

    caf::PdmField<caf::AppEnum<RiaDefines::BatchSchedulerType>>     m_batchScheduler;
    caf::PdmField<caf::AppEnum<RiaDefines::BatchJobMonitoringType>> m_batchJobMonitoring;
};
