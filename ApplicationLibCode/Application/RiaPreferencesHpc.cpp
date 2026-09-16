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

#include "RiaPreferencesHpc.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaWslTools.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesHpc, "RiaPreferencesHpc" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesHpc::RiaPreferencesHpc()
{
    CAF_PDM_InitFieldNoDefault( &m_maxParallelJobs, "maxParallelJobs", "Maximum number of jobs to run in parallel" );
    m_maxParallelJobs = 1;
    m_maxParallelJobs.setRange( 1, 100 );

    m_availableWslDists = RiaWslTools::wslDistributionList();
    // if ( !m_availableWslDists.isEmpty() ) m_wslDistribution = m_availableWslDists.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesHpc* RiaPreferencesHpc::current()
{
    return RiaApplication::instance()->preferences()->hpcPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesHpc::appendItems( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* hpcGrp = uiOrdering.addNewGroup( "General Settings" );
    hpcGrp->add( &m_maxParallelJobs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferencesHpc::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    // if ( fieldNeedingOptions == &m_wslDistribution )
    //{
    //     for ( auto dist : m_availableWslDists )
    //     {
    //         options.push_back( caf::PdmOptionItemInfo( dist, QVariant::fromValue( dist ) ) );
    //     }
    // }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaPreferencesHpc::maxParallelJobs() const
{
    return m_maxParallelJobs();
}
