/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 - Equinor ASA
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

#include "RiaPreferencesGrid.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmUiCheckBoxEditor.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesGrid, "RifReaderSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGrid::RiaPreferencesGrid()
    : m_gridModelReaderOverride( RiaDefines::GridModelReader::NOT_SET )
{
    CAF_PDM_InitObject( "RiaPreferencesGrid" );

    CAF_PDM_InitFieldNoDefault( &m_gridModelReader, "gridModelReader", "Model Reader" );
    m_gridModelReader = RiaDefines::GridModelReader::RESDATA;

    CAF_PDM_InitField( &m_importFaults, "importFaults", true, "Import Faults" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_importFaults );

    CAF_PDM_InitField( &m_importNNCs, "importSimulationNNCs", true, "Import NNCs" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_importNNCs );

    CAF_PDM_InitField( &m_includeInactiveCellsInFaultGeometry,
                       "includeInactiveCellsInFaultGeometry",
                       false,
                       "Include Inactive Cells in Fault Geometry" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeInactiveCellsInFaultGeometry );

    CAF_PDM_InitField( &m_importAdvancedMswData, "importAdvancedMswData", true, "Import Advanced MSW Data" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_importAdvancedMswData );

    CAF_PDM_InitField( &m_useResultIndexFile,
                       "useResultIndexFile",
                       false,
                       "Use Result Index File",
                       "",
                       "After import of a result file, store index data in an index file in the same folder as the "
                       "result file.\n"
                       "Import of result data if a result index file is present, will reduce file parsing "
                       "significantly.",
                       "" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useResultIndexFile );

    CAF_PDM_InitField( &m_skipWellData, "skipWellData", false, "Skip Import of Simulation Well Data" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_skipWellData );

    CAF_PDM_InitField( &m_includeFileAbsolutePathPrefix,
                       "includeFileAbsolutePathPrefix",
                       QString(),
                       "Include File Absolute Path Prefix",
                       "",
                       "Path used to prefix absolute UNIX paths in include statements on Windows, used when searching "
                       "for FAULTS and EQUIL",
                       "" );

    CAF_PDM_InitField( &m_autoComputeDepthRelatedProperties,
                       "autocomputeDepth",
                       true,
                       "Compute DEPTH Related Properties",
                       "",
                       "DEPTH, DX, DY, DZ, TOP, BOTTOM",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_autoComputeDepthRelatedProperties );

    CAF_PDM_InitField( &m_loadAndShowSoil, "loadAndShowSoil", true, "Load and Show SOIL" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_loadAndShowSoil );

    CAF_PDM_InitField( &m_onlyLoadActiveCells, "onlyLoadActiveCells", false, "Only Load Active Cell Geometry" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_onlyLoadActiveCells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGrid* RiaPreferencesGrid::current()
{
    return RiaApplication::instance()->preferences()->gridPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesGrid::appendItems( caf::PdmUiOrdering& uiOrdering )
{
    auto newCBGroup = uiOrdering.addNewGroup( "Behavior When Loading Data" );
    newCBGroup->add( &m_autoComputeDepthRelatedProperties );
    newCBGroup->add( &m_loadAndShowSoil );

    auto faultGrp = uiOrdering.addNewGroup( "Fault Import" );

    faultGrp->add( &m_importFaults );
    faultGrp->add( &m_includeInactiveCellsInFaultGeometry );
#ifdef WIN32
    faultGrp->add( &m_includeFileAbsolutePathPrefix );
#endif
    faultGrp->add( &m_importNNCs );

    auto wellGrp = uiOrdering.addNewGroup( "Well Import" );

    wellGrp->add( &m_skipWellData );
    wellGrp->add( &m_importAdvancedMswData );

    if ( m_gridModelReaderOverride == RiaDefines::GridModelReader::NOT_SET )
    {
        auto egridGrp = uiOrdering.addNewGroup( "EGRID Settings" );
        egridGrp->add( &m_gridModelReader );
    }

    auto resdataGrp = uiOrdering.addNewGroup( "ResData Reader Settings" );
    resdataGrp->add( &m_useResultIndexFile );

    auto opmcGrp = uiOrdering.addNewGroup( "OPM Common Reader Settings" );
    opmcGrp->add( &m_onlyLoadActiveCells );

    const bool setFaultImportSettingsReadOnly = !importFaults();

    m_includeInactiveCellsInFaultGeometry.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
    m_includeFileAbsolutePathPrefix.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
    m_importNNCs.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderSettings RiaPreferencesGrid::gridOnlyReaderSettings()
{
    RifReaderSettings rs{
        // Disable as much as possible
        false, // import faults
        false, // import NNCs
        false, // includeInactiveCellsInFaultGeometry
        false, // importAdvancedMswData
        false, // useResultIndexFile
        true, // skipWellData
        false, // import summary data
        "", // include prefix,
        false // only active cells
    };
    return rs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderSettings RiaPreferencesGrid::readerSettings()
{
    RifReaderSettings rs{ m_importFaults,
                          m_importNNCs,
                          m_includeInactiveCellsInFaultGeometry,
                          m_importAdvancedMswData,
                          m_useResultIndexFile,
                          m_skipWellData,
                          true, // import summary data
                          m_includeFileAbsolutePathPrefix,
                          m_onlyLoadActiveCells };
    return rs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::importFaults() const
{
    return m_importFaults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::importNNCs() const
{
    return m_importNNCs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::includeInactiveCellsInFaultGeometry() const
{
    return m_includeInactiveCellsInFaultGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::importAdvancedMswData() const
{
    return m_importAdvancedMswData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGrid::includeFileAbsolutePathPrefix() const
{
    return m_includeFileAbsolutePathPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::useResultIndexFile() const
{
    return m_useResultIndexFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::skipWellData() const
{
    return m_skipWellData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::loadAndShowSoil() const
{
    return m_loadAndShowSoil;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::autoComputeDepthRelatedProperties() const
{
    return m_autoComputeDepthRelatedProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGrid::onlyLoadActiveCells() const
{
    return m_onlyLoadActiveCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::GridModelReader RiaPreferencesGrid::gridModelReader() const
{
    if ( m_gridModelReaderOverride != RiaDefines::GridModelReader::NOT_SET )
    {
        return m_gridModelReaderOverride;
    }

    return m_gridModelReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesGrid::setGridModelReaderOverride( const std::string& readerName )
{
    RiaDefines::GridModelReader readerType = RiaDefines::GridModelReader::NOT_SET;
    if ( readerName == "opm_common" )
    {
        readerType = RiaDefines::GridModelReader::OPM_COMMON;
    }
    else if ( readerName == "resdata" )
    {
        readerType = RiaDefines::GridModelReader::RESDATA;
    }
    else
    {
        RiaLogging::warning( QString::fromStdString( "Unknown EGRID reader type specified on command line: " + readerName ) );
        return;
    }

    if ( readerType != RiaDefines::GridModelReader::NOT_SET )
    {
        RiaLogging::info( QString::fromStdString( "Using EGRID reader: " + readerName ) );
    }

    m_gridModelReaderOverride = readerType;
}
