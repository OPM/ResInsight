/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicCaseAndFileExportSettingsUi.h"

#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RicCaseAndFileExportSettingsUi, "RicCaseAndFileExportSettingsUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCaseAndFileExportSettingsUi::RicCaseAndFileExportSettingsUi()
{
    CAF_PDM_InitObject( "RimCaseAndFileExportSettings" );

    CAF_PDM_InitFieldNoDefault( &folder, "Folder", "Export Folder" );
    folder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    folder.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::SELECT_DIRECTORY, true );

    CAF_PDM_InitFieldNoDefault( &caseToApply, "CaseToApply", "Case to Apply" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicCaseAndFileExportSettingsUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &caseToApply )
    {
        RimTools::caseOptionItems( &options );
    }

    return options;
}
