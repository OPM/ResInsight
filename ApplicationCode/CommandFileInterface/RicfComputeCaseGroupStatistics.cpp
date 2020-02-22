/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfComputeCaseGroupStatistics.h"

#include "Rim3dView.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

CAF_PDM_SOURCE_INIT( RicfComputeCaseGroupStatistics, "computeCaseGroupStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfComputeCaseGroupStatistics::RicfComputeCaseGroupStatistics()
{
    RICF_InitField( &m_groupId, "caseGroupId", -1, "Case Group ID", "", "", "" );
    RICF_InitField( &m_caseIds, "caseIds", std::vector<int>(), "Case IDs", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfComputeCaseGroupStatistics::execute()
{
    RicfCommandResponse response;

    std::vector<int> caseIds = m_caseIds.v();

    if ( m_groupId() >= 0 )
    {
        for ( RimIdenticalGridCaseGroup* group :
              RiaApplication::instance()->project()->activeOilField()->analysisModels()->caseGroups )
        {
            for ( RimEclipseCase* c : group->statisticsCaseCollection->reservoirs )
            {
                caseIds.push_back( c->caseId() );
            }
        }
    }

    for ( int caseId : caseIds )
    {
        bool foundCase = false;
        for ( RimIdenticalGridCaseGroup* group :
              RiaApplication::instance()->project()->activeOilField()->analysisModels()->caseGroups )
        {
            for ( RimEclipseCase* c : group->statisticsCaseCollection->reservoirs )
            {
                if ( c->caseId == caseId )
                {
                    RimEclipseStatisticsCase* statsCase = dynamic_cast<RimEclipseStatisticsCase*>( c );
                    if ( statsCase )
                    {
                        statsCase->computeStatisticsAndUpdateViews();
                    }
                    else
                    {
                        QString warning = QString( "computeCaseGroupStatistics: Found case with ID %1, but it is not a "
                                                   "statistics case, cannot compute statistics." )
                                              .arg( caseId );
                        RiaLogging::warning( warning );
                        response.updateStatus( RicfCommandResponse::COMMAND_WARNING, warning );
                    }
                    foundCase = true;
                    break;
                }
            }

            if ( foundCase ) break;
        }

        if ( !foundCase )
        {
            QString warning =
                QString( "computeCaseGroupStatistics: Could not find statistics case with ID %1." ).arg( caseId );

            RiaLogging::warning( warning );
            response.updateStatus( RicfCommandResponse::COMMAND_WARNING, warning );
        }
    }
    return response;
}
