/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicfExportPropertyInViews.h"

#include "RiaLogging.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimcEclipseView.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RicfExportPropertyInViews, "exportPropertyInViews" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportPropertyInViews::RicfExportPropertyInViews()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_viewIds, "viewIds", std::vector<int>(), "View IDs" );
    CAF_PDM_InitScriptableField( &m_viewNames, "viewNames", std::vector<QString>(), "View Names" );
    CAF_PDM_InitScriptableField( &m_undefinedValue, "undefinedValue", 0.0, "Undefined Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportPropertyInViews::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    // Select the views by id or name. No filter means all views of the case.
    std::vector<RimEclipseView*> viewsForExport;
    for ( Rim3dView* v : eclipseCase->views() )
    {
        auto* view = dynamic_cast<RimEclipseView*>( v );
        if ( !view ) continue;

        bool matching = m_viewNames().empty() && m_viewIds().empty();

        if ( !matching )
        {
            matching = std::find( m_viewIds().begin(), m_viewIds().end(), view->id() ) != m_viewIds().end();
        }

        if ( !matching )
        {
            for ( const auto& viewName : m_viewNames() )
            {
                if ( view->name().compare( viewName, Qt::CaseInsensitive ) == 0 )
                {
                    matching = true;
                    break;
                }
            }
        }

        if ( matching ) viewsForExport.push_back( view );
    }

    // Resolve the default export folder from the command file executor state. The Rimc method requires an explicit file.
    QDir propertiesDir( RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::PROPERTIES ) );

    caf::PdmScriptResponse response;

    for ( RimEclipseView* view : viewsForExport )
    {
        RimEclipseView_exportCurrentProperty method( view );
        method.setExportFile( propertiesDir.filePath( RimEclipseView_exportCurrentProperty::defaultFileBaseName( view ) ) );
        method.setUndefinedValue( m_undefinedValue() );

        auto result = method.execute();
        if ( !result )
        {
            // Legacy behavior: a missing property in one view is a warning, not an error, and the export continues
            QString warning = QString( "%1: %2" ).arg( commandName ).arg( result.error() );
            RiaLogging::warning( warning.toStdString() );
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
        }
    }

    return response;
}
