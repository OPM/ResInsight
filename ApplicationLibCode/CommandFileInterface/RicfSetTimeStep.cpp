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

#include "RicfSetTimeStep.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfSetTimeStep, "setTimeStep" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetTimeStep::RicfSetTimeStep()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_timeStepIndex, "timeStep", -1, "Time Step Index", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfSetTimeStep::setCaseId( int caseId )
{
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfSetTimeStep::setViewId( int viewId )
{
    m_viewId = viewId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfSetTimeStep::setTimeStepIndex( int timeStepIndex )
{
    m_timeStepIndex = timeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetTimeStep::execute()
{
    RimCase*              rimCase = nullptr;
    std::vector<RimCase*> allCases;

    {
        RimProject::current()->allCases( allCases );

        bool foundCase = false;
        for ( RimCase* c : allCases )
        {
            if ( c->caseId == m_caseId )
            {
                rimCase   = c;
                foundCase = true;
                break;
            }
        }
        if ( !foundCase )
        {
            QString error = QString( "setTimeStep: Could not find case with ID %1" ).arg( m_caseId() );
            RiaLogging::error( error );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
        }
    }

    int maxTimeStep = rimCase->timeStepStrings().size() - 1;
    if ( m_timeStepIndex() > maxTimeStep )
    {
        QString error = QString( "setTimeStep: Step %1 is larger than the maximum of %2 for case %3" )
                            .arg( m_timeStepIndex() )
                            .arg( maxTimeStep )
                            .arg( m_caseId() );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    for ( Rim3dView* view : rimCase->views() )
    {
        if ( m_viewId() == -1 || view->id() == m_viewId() )
        {
            view->setCurrentTimeStepAndUpdate( m_timeStepIndex );
            view->createDisplayModelAndRedraw();
        }
    }

    return caf::PdmScriptResponse();
}
