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
#include "RicfExportWellLogPlotData.h"

#include "RiaLogging.h"

#include "ExportCommands/RicExportToLasFileFeature.h"
#include "WellLogCommands/RicAsciiExportWellLogPlotFeature.h"

#include "RimProject.h"
#include "RimWellLogPlot.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace caf
{
template <>
void RicfExportWellLogPlotData::ExportFormatEnum::setUp()
{
    addItem( RicfExportWellLogPlotData::ExportFormat::LAS, "LAS", "LAS" );
    addItem( RicfExportWellLogPlotData::ExportFormat::ASCII, "ASCII", "ASCII" );
    setDefault( RicfExportWellLogPlotData::ExportFormat::LAS );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RicfExportWellLogPlotDataResult, "exportWellLogPlotDataResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellLogPlotDataResult::RicfExportWellLogPlotDataResult()
{
    CAF_PDM_InitObject( "export_well_data_result", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &this->exportedFiles, "exportedFiles", "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfExportWellLogPlotData, "exportWellLogPlotData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellLogPlotData::RicfExportWellLogPlotData()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_format, "exportFormat", "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_folder, "exportFolder", QString(), "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_filePrefix, "filePrefix", QString(), "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_exportTvdRkb, "exportTvdRkb", false, "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_capitalizeFileNames, "capitalizeFileNames", false, "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_resampleInterval, "resampleInterval", 0.0, "", "", "", "" );
    CAF_PDM_InitScriptableField( &m_convertCurveUnits, "convertCurveUnits", false, "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportWellLogPlotData::execute()
{
    QStringList errorMessages;

    caf::PdmScriptResponse response;

    if ( QFileInfo::exists( m_folder ) )
    {
        std::vector<RimWellLogPlot*> plots;
        RimProject::current()->descendantsIncludingThisOfType( plots );
        RicfExportWellLogPlotDataResult* result = new RicfExportWellLogPlotDataResult;

        for ( RimWellLogPlot* plot : plots )
        {
            if ( plot->id() == m_viewId() )
            {
                if ( m_format() == ExportFormat::ASCII )
                {
                    QString validFileName =
                        RicAsciiExportWellLogPlotFeature::makeValidExportFileName( plot,
                                                                                   m_folder(),
                                                                                   m_filePrefix(),
                                                                                   m_capitalizeFileNames() );
                    if ( RicAsciiExportWellLogPlotFeature::exportAsciiForWellLogPlot( validFileName, plot ) )
                    {
                        result->exportedFiles.v().push_back( validFileName );
                    }
                }
                else
                {
                    std::vector<QString> exportedFiles =
                        RicExportToLasFileFeature::exportToLasFiles( m_folder(),
                                                                     m_filePrefix(),
                                                                     plot,
                                                                     m_exportTvdRkb(),
                                                                     m_capitalizeFileNames(),
                                                                     true,
                                                                     m_resampleInterval(),
                                                                     m_convertCurveUnits() );
                    if ( exportedFiles.empty() )
                    {
                        errorMessages << QString( "No files exported for '%1'" ).arg( plot->description() );
                    }
                    else
                    {
                        result->exportedFiles.v().insert( result->exportedFiles.v().end(),
                                                          exportedFiles.begin(),
                                                          exportedFiles.end() );
                    }
                }
            }
        }
        response.setResult( result );
    }
    else
    {
        errorMessages << ( m_folder() + " does not exist" );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }
    return response;
}
