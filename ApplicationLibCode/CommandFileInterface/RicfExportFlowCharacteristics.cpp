/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicfExportFlowCharacteristics.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicfApplicationTools.h"

#include "RimEclipseResultCase.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

CAF_PDM_SOURCE_INIT( RicfExportFlowCharacteristics, "exportFlowCharacteristics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportFlowCharacteristics::RicfExportFlowCharacteristics()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_selectedTimeSteps, "timeSteps", std::vector<int>(), "Selected Time Steps", "", "", "" );
    CAF_PDM_InitScriptableField( &m_injectors, "injectors", std::vector<QString>(), "Injectors", "", "", "" );
    CAF_PDM_InitScriptableField( &m_producers, "producers", std::vector<QString>(), "Producers", "", "", "" );
    CAF_PDM_InitScriptableField( &m_fileName, "fileName", QString(), "Export File Name", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minCommunication, "minimumCommunication", 0.0, "Minimum Communication", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maxPvFraction, "aquiferCellThreshold", 0.1, "Aquifer Cell Threshold", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportFlowCharacteristics::execute()
{
    using TOOLS = RicfApplicationTools;

    auto eclipseCase = dynamic_cast<RimEclipseResultCase*>( TOOLS::caseFromId( m_caseId() ) );
    if ( !eclipseCase )
    {
        QString error = QString( "exportFlowCharacteristics: Could not find case with ID %1." ).arg( m_caseId() );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    {
        QString   exportFileName = m_fileName();
        QFileInfo fi( exportFileName );
        if ( !fi.isAbsolute() )
        {
            QString relativePath = fi.path();

            QString exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( relativePath );

            QDir exportDir( exportFolder );
            if ( !exportDir.exists() )
            {
                if ( !exportDir.mkpath( "." ) )
                {
                    QString msg = QString( "Failed to create folder - %1" ).arg( exportFolder );
                    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, msg );
                }
            }

            exportFileName = exportFolder + "/" + fi.fileName();
        }

        RimFlowPlotCollection* flowPlotColl = RimProject::current()->mainPlotCollection->flowPlotCollection();
        if ( flowPlotColl )
        {
            RimFlowCharacteristicsPlot* plot = flowPlotColl->defaultFlowCharacteristicsPlot();
            plot->setFromFlowSolution( eclipseCase->defaultFlowDiagSolution() );
            plot->setTimeSteps( m_selectedTimeSteps );
            plot->setInjectorsAndProducers( m_injectors, m_producers );
            plot->setAquiferCellThreshold( m_maxPvFraction );
            plot->setMinimumCommunication( m_minCommunication );

            plot->loadDataAndUpdate();

            {
                QString content = plot->curveDataAsText();

                QFile file( exportFileName );
                if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
                {
                    QTextStream textstream( &file );
                    textstream << content;
                }
                else
                {
                    QString msg = QString( "Failed to export file - %1" ).arg( exportFileName );
                    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, msg );
                }
            }
        }
    }
    return caf::PdmScriptResponse();
}
