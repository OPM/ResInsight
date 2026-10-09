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

#include "RimBatchQueueSelector.h"

#include "RiaHpcTools.h"

#include "cafPdmFieldCapability.h"
#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT( RimBatchQueueSelector, "BatchQueueSelector" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSelector::RimBatchQueueSelector()
{
    CAF_PDM_InitObject( "Batch Queue Selector" );

    CAF_PDM_InitFieldNoDefault( &m_queueName, "QueueName", "Select Queue/Partition" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSelector::~RimBatchQueueSelector()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSelector::setBatchSchedulerType( RiaDefines::BatchSchedulerType schedulerType )
{
    m_schedulerType = schedulerType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimBatchQueueSelector::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_queueName )
    {
        auto candidates = RiaHpcTools::availableQueues( m_schedulerType );
        for ( auto& q : candidates )
        {
            options.push_back( caf::PdmOptionItemInfo( q, QVariant::fromValue( q ) ) );
        }
    }
    return options;
}