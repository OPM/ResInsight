/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimBinaryExportSettings.h"

#include "RiaPreferences.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RimBinaryExportSettings, "RimBinaryExportSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBinaryExportSettings::RimBinaryExportSettings()
{
    CAF_PDM_InitObject( "RimBinaryExportSettings" );

    CAF_PDM_InitFieldNoDefault( &fileName, "Filename", "Export Filename" );
    fileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    fileName.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::SELECT_SAVE_FILE_NAME, true );
    fileName.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::FILE_SELECTION_FILTER,
                                           QString( "GRDECL files (*.grdecl *.GRDECL);;All files (*.*)" ) );
    CAF_PDM_InitFieldNoDefault( &eclipseKeyword, "EclipseKeyword", "Eclipse Keyword" );
    CAF_PDM_InitField( &undefinedValue, "UndefinedValue", 0.0, "Undefined Value" );

    CAF_PDM_InitField( &writeEchoInGrdeclFiles,
                       "WriteEchoInGrdeclFiles",
                       RiaPreferences::current()->writeEchoInGrdeclFiles(),
                       "Write NOECHO and ECHO" );
}
