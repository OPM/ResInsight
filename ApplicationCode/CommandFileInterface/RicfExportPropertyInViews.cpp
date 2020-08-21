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

#include "RicfExportProperty.h"

#include "ExportCommands/RicEclipseCellResultToFileImpl.h"

#include "RiaLogging.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"
#include "RicfExportPropertyInViews.h"

#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafUtils.h"

#include <limits>

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfExportPropertyInViews, "exportPropertyInViews" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportPropertyInViews::RicfExportPropertyInViews()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewIds, "viewIds", std::vector<int>(), "View IDs", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewNames, "viewNames", std::vector<QString>(), "View Names", "", "", "" );
    CAF_PDM_InitScriptableField( &m_undefinedValue, "undefinedValue", 0.0, "Undefined Value", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportPropertyInViews::execute()
{
    using TOOLS = RicfApplicationTools;

    RimEclipseCase* eclipseCase = TOOLS::caseFromId( m_caseId() );
    if ( !eclipseCase )
    {
        QString error( QString( "exportProperty: Could not find case with ID %1" ).arg( m_caseId() ) );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    std::vector<RimEclipseView*> viewsForExport;

    for ( Rim3dView* v : eclipseCase->views() )
    {
        RimEclipseView* view = dynamic_cast<RimEclipseView*>( v );
        if ( !view ) continue;

        if ( m_viewNames().empty() && m_viewIds().empty() )
        {
            viewsForExport.push_back( view );
        }
        else
        {
            bool matchingIdOrName = false;

            for ( auto viewId : m_viewIds() )
            {
                if ( view->id() == viewId )
                {
                    matchingIdOrName = true;
                    break;
                }
            }

            if ( !matchingIdOrName )
            {
                for ( const auto& viewName : m_viewNames() )
                {
                    if ( view->name().compare( viewName, Qt::CaseInsensitive ) == 0 )
                    {
                        matchingIdOrName = true;
                    }
                }
            }

            if ( matchingIdOrName )
            {
                viewsForExport.push_back( view );
            }
        }
    }

    caf::PdmScriptResponse response;

    for ( const auto& view : viewsForExport )
    {
        cvf::ref<RigResultAccessor> resultAccessor = nullptr;
        {
            const int mainGridIndex = 0;

            resultAccessor = RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                                   mainGridIndex,
                                                                                   view->currentTimeStep(),
                                                                                   view->cellResult() );
        }

        const QString propertyName = view->cellResult()->resultVariableUiShortName();

        if ( resultAccessor.isNull() )
        {
            QString warning =
                QString( "exportProperty: Could not find property. Case ID %1, time step %2, property '%3'" )
                    .arg( m_caseId )
                    .arg( view->currentTimeStep() )
                    .arg( propertyName );
            RiaLogging::warning( warning );
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
            continue;
        }

        QDir propertiesDir(
            RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::PROPERTIES ) );

        QString fileName = QString( "%1-%2-T%3-%4" )
                               .arg( eclipseCase->caseUserDescription() )
                               .arg( view->name() )
                               .arg( view->currentTimeStep() )
                               .arg( propertyName );

        fileName               = caf::Utils::makeValidFileBasename( fileName );
        const QString filePath = propertiesDir.filePath( fileName );

        QString errorMsg;

        bool worked = RicEclipseCellResultToFileImpl::writeResultToTextFile( filePath,
                                                                             eclipseCase->eclipseCaseData(),
                                                                             resultAccessor.p(),
                                                                             propertyName,
                                                                             m_undefinedValue,
                                                                             "exportPropertiesInViews",
                                                                             &errorMsg );
        if ( !worked )
        {
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errorMsg );
        }
    }
    return response;
}
