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

#include "RicfCommandForwarding.h"

#include "RimWellLogPlot.h"
#include "RimcDataContainerString.h"
#include "RimcWellLogPlot.h"

#include "cafPdmFieldScriptingCapability.h"

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
    CAF_PDM_InitObject( "export_well_data_result" );
    CAF_PDM_InitFieldNoDefault( &exportedFiles, "exportedFiles", "" );
}

CAF_PDM_SOURCE_INIT( RicfExportWellLogPlotData, "exportWellLogPlotData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellLogPlotData::RicfExportWellLogPlotData()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_format, "exportFormat", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "" );
    CAF_PDM_InitScriptableField( &m_folder, "exportFolder", QString(), "" );
    CAF_PDM_InitScriptableField( &m_filePrefix, "filePrefix", QString(), "" );
    CAF_PDM_InitScriptableField( &m_exportTvdRkb, "exportTvdRkb", false, "" );
    CAF_PDM_InitScriptableField( &m_capitalizeFileNames, "capitalizeFileNames", false, "" );
    CAF_PDM_InitScriptableField( &m_resampleInterval, "resampleInterval", 0.0, "" );
    CAF_PDM_InitScriptableField( &m_convertCurveUnits, "convertCurveUnits", false, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportWellLogPlotData::execute()
{
    const QString commandName = classKeyword();

    auto plot = RicfForwarding::findWellLogPlot( m_viewId() );
    if ( !plot )
    {
        // Legacy behavior: an unknown view id produced an empty result, not an error
        caf::PdmScriptResponse response;
        response.setResult( new RicfExportWellLogPlotDataResult );
        return response;
    }

    std::expected<caf::PdmObjectHandle*, QString> result;
    if ( m_format() == ExportFormat::ASCII )
    {
        RimWellLogPlot_exportDataAsAscii method( plot.value() );
        method.setExportFolder( m_folder() );
        method.setFilePrefix( m_filePrefix() );
        method.setCapitalizeFileNames( m_capitalizeFileNames() );
        result = method.execute();
    }
    else
    {
        RimWellLogPlot_exportDataAsLas method( plot.value() );
        method.setExportFolder( m_folder() );
        method.setFilePrefix( m_filePrefix() );
        method.setExportTvdRkb( m_exportTvdRkb() );
        method.setCapitalizeFileNames( m_capitalizeFileNames() );
        method.setResampleInterval( m_resampleInterval() );
        method.setConvertToStandardUnits( m_convertCurveUnits() );
        result = method.execute();
    }

    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    auto* files = dynamic_cast<RimcDataContainerString*>( result.value() );

    auto* exportResult = new RicfExportWellLogPlotDataResult;
    if ( files ) exportResult->exportedFiles = files->m_stringValues();
    delete files;

    caf::PdmScriptResponse response;
    response.setResult( exportResult );
    return response;
}
