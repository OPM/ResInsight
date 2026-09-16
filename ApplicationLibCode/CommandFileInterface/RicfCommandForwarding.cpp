/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RicfCommandForwarding.h"

#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseResultCase.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmObjectHandle.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimCase*, QString> RicfForwarding::findCase( int caseId )
{
    RimProject* project = RimProject::current();
    if ( !project ) return std::unexpected( "No project is available." );

    std::vector<RimCase*> allCases = project->allGridCases();
    if ( caseId < 0 )
    {
        // Matches RiaProjectModifier::firstCaseId(): the first occurrence is the first Eclipse result case
        for ( RimCase* rimCase : allCases )
        {
            if ( dynamic_cast<RimEclipseResultCase*>( rimCase ) ) return rimCase;
        }
        return std::unexpected( "No Eclipse result cases found in project." );
    }

    for ( RimCase* rimCase : allCases )
    {
        if ( rimCase && rimCase->caseId() == caseId ) return rimCase;
    }

    return std::unexpected( QString( "Could not find case with ID %1" ).arg( caseId ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Rim3dView*, QString> RicfForwarding::findView( RimCase* rimCase, int viewId )
{
    if ( !rimCase ) return std::unexpected( "No case given when looking up view." );

    for ( Rim3dView* view : rimCase->views() )
    {
        if ( view && view->id() == viewId ) return view;
    }

    return std::unexpected( QString( "Could not find view with ID %1 in case %2" ).arg( viewId ).arg( rimCase->caseId() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Rim3dView*, QString> RicfForwarding::findView( int viewId )
{
    RimProject* project = RimProject::current();
    if ( !project ) return std::unexpected( "No project is available." );

    for ( Rim3dView* view : project->allViews() )
    {
        if ( view && view->id() == viewId ) return view;
    }

    return std::unexpected( QString( "Could not find view with ID %1" ).arg( viewId ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimFractureTemplate*, QString> RicfForwarding::findFractureTemplate( int templateId )
{
    RimProject* project = RimProject::current();
    if ( !project ) return std::unexpected( "No project is available." );

    for ( RimFractureTemplate* fractureTemplate : project->allFractureTemplates() )
    {
        if ( fractureTemplate && fractureTemplate->id() == templateId ) return fractureTemplate;
    }

    return std::unexpected( QString( "Could not find fracture template with ID %1" ).arg( templateId ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfForwarding::toScriptResponse( const std::expected<caf::PdmObjectHandle*, QString>& result, const QString& commandName )
{
    if ( !result.has_value() ) return errorResponse( result.error(), commandName );

    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfForwarding::errorResponse( const QString& message, const QString& commandName )
{
    QString fullMessage = QString( "%1: %2" ).arg( commandName ).arg( message );
    RiaLogging::error( fullMessage.toStdString() );
    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, fullMessage );
}
