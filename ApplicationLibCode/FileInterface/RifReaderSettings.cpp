/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RifReaderSettings.h"

#include "cafPdmUiCheckBoxEditor.h"

CAF_PDM_SOURCE_INIT( RifReaderSettings, "RifReaderSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderSettings::RifReaderSettings()
{
    CAF_PDM_InitObject( "RifReaderSettings", "", "", "" );

    CAF_PDM_InitField( &importFaults, "importFaults", true, "Import Faults", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &importFaults );

    CAF_PDM_InitField( &importNNCs, "importSimulationNNCs", true, "Import NNCs", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &importNNCs );

    CAF_PDM_InitField( &includeInactiveCellsInFaultGeometry,
                       "includeInactiveCellsInFaultGeometry",
                       false,
                       "Include Inactive Cells",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &includeInactiveCellsInFaultGeometry );

    CAF_PDM_InitField( &importAdvancedMswData, "importAdvancedMswData", false, "Import Advanced MSW Data", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &importAdvancedMswData );

    CAF_PDM_InitField( &useResultIndexFile,
                       "useResultIndexFile",
                       false,
                       "Use Result Index File",
                       "",
                       "After import of a result file, store index data in an index file in the same folder as the "
                       "result file.\n"
                       "Import of result data if a result index file is present, will reduce file parsing "
                       "significantly.",
                       "" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &useResultIndexFile );

    CAF_PDM_InitField( &skipWellData, "skipWellData", false, "Skip Import of Simulation Well Data", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &skipWellData );

    CAF_PDM_InitField( &includeFileAbsolutePathPrefix,
                       "includeFileAbsolutePathPrefix",
                       QString(),
                       "Include File Absolute Path Prefix",
                       "",
                       "Path used to prefix absolute UNIX paths in include statements on Windows, used when searching "
                       "for FAULTS and EQUIL",
                       "" );

    CAF_PDM_InitField( &importSummaryData, "importSummaryData", true, "Import summary data", "", "", "" );
    importSummaryData.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &importFaults );
    uiOrdering.add( &includeInactiveCellsInFaultGeometry );
#ifdef WIN32
    uiOrdering.add( &includeFileAbsolutePathPrefix );
#endif
    uiOrdering.add( &importNNCs );
    uiOrdering.add( &importAdvancedMswData );
    uiOrdering.add( &useResultIndexFile );
    uiOrdering.add( &skipWellData );

    bool setFaultImportSettingsReadOnly = !importFaults();

    includeInactiveCellsInFaultGeometry.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
    includeFileAbsolutePathPrefix.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
    importNNCs.uiCapability()->setUiReadOnly( setFaultImportSettingsReadOnly );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RifReaderSettings> RifReaderSettings::createGridOnlyReaderSettings()
{
    std::shared_ptr<RifReaderSettings> readerSettings = std::make_shared<RifReaderSettings>();
    // Disable as much as possible
    readerSettings->importNNCs                          = false;
    readerSettings->importFaults                        = false;
    readerSettings->skipWellData                        = true;
    readerSettings->includeInactiveCellsInFaultGeometry = false;
    readerSettings->importAdvancedMswData               = false;
    readerSettings->importSummaryData                   = false;
    return readerSettings;
}
