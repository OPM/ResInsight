/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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
#include "RimcWellLogPlot.h"

#include "RiaApplication.h"

#include "ExportCommands/RicExportToLasFileFeature.h"
#include "WellLogCommands/RicAsciiExportWellLogPlotFeature.h"
#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimcDataContainerString.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogPlot, RimcWellLogPlot_newWellLogTrack, "NewWellLogTrack" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellLogPlot_newWellLogTrack::RimcWellLogPlot_newWellLogTrack( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Well Log Track", "", "", "Create a new well log track" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_title, "Title", "", "", "", "Title" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellLogPlot_newWellLogTrack::execute()
{
    RimWellLogPlot* wellLogPlot = self<RimWellLogPlot>();

    if ( !wellLogPlot ) return std::unexpected( "No well log plot found" );

    return createWellLogTrack( wellLogPlot, m_case(), m_wellPath(), m_title() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RimcWellLogPlot_newWellLogTrack::createWellLogTrack( RimWellLogPlot* wellLogPlot,
                                                                      RimEclipseCase* eclipseCase,
                                                                      RimWellPath*    wellPath,
                                                                      const QString&  title )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, title, wellLogPlot );
    if ( eclipseCase ) plotTrack->setFormationCase( eclipseCase );
    if ( wellPath ) plotTrack->setFormationWellPath( wellPath );

    plotTrack->setColSpan( RimPlot::TWO );
    plotTrack->setLegendsVisible( true );
    plotTrack->setPlotTitleVisible( true );
    plotTrack->setShowWindow( true );
    plotTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setAutoScalePropertyValuesEnabled( true );
    plotTrack->updateConnectedEditors();

    wellLogPlot->setShowWindow( true );
    wellLogPlot->updateConnectedEditors();
    wellLogPlot->loadDataAndUpdate();

    return plotTrack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellLogPlot_newWellLogTrack::classKeywordReturnedType() const
{
    return RimWellLogTrack::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogPlot, RimWellLogPlot_exportDataAsLas, "exportDataAsLas" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot_exportDataAsLas::RimWellLogPlot_exportDataAsLas( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Export Data As LAS", "", "", "Export the curves of the plot to LAS files and return the exported file names" );

    CAF_PDM_InitScriptableField( &m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "Folder to write the LAS files to. Must exist." );
    CAF_PDM_InitScriptableField( &m_filePrefix, "FilePrefix", QString(), "File Prefix", "", "", "Prefix for the generated file names" );
    CAF_PDM_InitScriptableField( &m_exportTvdRkb, "ExportTvdRkb", false, "Export TVD RKB", "", "", "Export in TVD-RKB format" );
    CAF_PDM_InitScriptableField( &m_capitalizeFileNames, "CapitalizeFileNames", false, "Capitalize File Names", "", "", "Make all file names upper case" );
    CAF_PDM_InitScriptableField( &m_resampleInterval,
                                 "ResampleInterval",
                                 0.0,
                                 "Resample Interval",
                                 "",
                                 "",
                                 "If > 0.0 the curves are resampled with this interval" );
    CAF_PDM_InitScriptableField( &m_convertToStandardUnits,
                                 "ConvertToStandardUnits",
                                 false,
                                 "Convert To Standard Units",
                                 "",
                                 "",
                                 "Convert curve units to standard units" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setFilePrefix( const QString& filePrefix )
{
    m_filePrefix = filePrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setExportTvdRkb( bool enable )
{
    m_exportTvdRkb = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setCapitalizeFileNames( bool enable )
{
    m_capitalizeFileNames = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setResampleInterval( double interval )
{
    m_resampleInterval = interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsLas::setConvertToStandardUnits( bool enable )
{
    m_convertToStandardUnits = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimWellLogPlot_exportDataAsLas::execute()
{
    auto* plot = self<RimWellLogPlot>();
    if ( !plot ) return std::unexpected( "No well log plot is available." );

    if ( m_exportFolder().isEmpty() ) return std::unexpected( "No export folder specified." );
    if ( !QFileInfo::exists( m_exportFolder() ) ) return std::unexpected( m_exportFolder() + " does not exist" );

    const bool           exportAllCurves = true;
    std::vector<QString> exportedFiles   = RicExportToLasFileFeature::exportToLasFiles( m_exportFolder(),
                                                                                      m_filePrefix(),
                                                                                      plot,
                                                                                      m_exportTvdRkb(),
                                                                                      m_capitalizeFileNames(),
                                                                                      exportAllCurves,
                                                                                      m_resampleInterval(),
                                                                                      m_convertToStandardUnits() );
    if ( exportedFiles.empty() ) return std::unexpected( QString( "No files exported for '%1'" ).arg( plot->description() ) );

    auto* result           = new RimcDataContainerString();
    result->m_stringValues = exportedFiles;
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot_exportDataAsLas::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellLogPlot, RimWellLogPlot_exportDataAsAscii, "exportDataAsAscii" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot_exportDataAsAscii::RimWellLogPlot_exportDataAsAscii( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Export Data As ASCII", "", "", "Export the curves of the plot to a single ASCII file and return the file name" );

    CAF_PDM_InitScriptableField( &m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "Folder to write the file to. Must exist." );
    CAF_PDM_InitScriptableField( &m_filePrefix, "FilePrefix", QString(), "File Prefix", "", "", "Prefix for the generated file name" );
    CAF_PDM_InitScriptableField( &m_capitalizeFileNames, "CapitalizeFileNames", false, "Capitalize File Names", "", "", "Make the file name upper case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsAscii::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsAscii::setFilePrefix( const QString& filePrefix )
{
    m_filePrefix = filePrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot_exportDataAsAscii::setCapitalizeFileNames( bool enable )
{
    m_capitalizeFileNames = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimWellLogPlot_exportDataAsAscii::execute()
{
    auto* plot = self<RimWellLogPlot>();
    if ( !plot ) return std::unexpected( "No well log plot is available." );

    if ( m_exportFolder().isEmpty() ) return std::unexpected( "No export folder specified." );
    if ( !QFileInfo::exists( m_exportFolder() ) ) return std::unexpected( m_exportFolder() + " does not exist" );

    QString fileName =
        RicAsciiExportWellLogPlotFeature::makeValidExportFileName( plot, m_exportFolder(), m_filePrefix(), m_capitalizeFileNames() );
    if ( !RicAsciiExportWellLogPlotFeature::exportAsciiForWellLogPlot( fileName, plot ) )
    {
        return std::unexpected( QString( "Could not export '%1' to %2" ).arg( plot->description() ).arg( fileName ) );
    }

    auto* result           = new RimcDataContainerString();
    result->m_stringValues = { fileName };
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot_exportDataAsAscii::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}
