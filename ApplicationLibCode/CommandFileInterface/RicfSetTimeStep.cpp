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

#include "RicfCommandForwarding.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimcGridView.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfSetTimeStep, "setTimeStep" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetTimeStep::RicfSetTimeStep()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View ID" );
    CAF_PDM_InitScriptableField( &m_timeStepIndex, "timeStep", -1, "Time Step Index" );
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
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    // A view id of -1 applies the time step to all views of the case
    std::vector<Rim3dView*> views;
    if ( m_viewId() == -1 )
    {
        views = rimCase.value()->views();
    }
    else
    {
        auto view = RicfForwarding::findView( rimCase.value(), m_viewId() );
        if ( !view ) return RicfForwarding::errorResponse( view.error(), commandName );
        views.push_back( view.value() );
    }

    for ( Rim3dView* view : views )
    {
        Rim3dView_setTimeStep method( view );
        method.setTimeStep( m_timeStepIndex() );

        auto result = method.execute();
        if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );
    }

    return caf::PdmScriptResponse();
}
