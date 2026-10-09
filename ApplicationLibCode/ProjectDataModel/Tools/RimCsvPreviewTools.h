/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#pragma once

#include "cafPdmField.h"

#include <QString>

namespace caf
{
class PdmUiEditorAttribute;
}

//==================================================================================================
/// Helpers for showing CSV data as a read-only HTML table in the property editor
//==================================================================================================
namespace RimCsvPreviewTools
{
const int defaultMaxRowCount = 1000;

// Configures the field as a read-only, non-persistent HTML text editor without label
void initPreviewField( caf::PdmField<QString>& field );

// Call from defineEditorAttribute() for the preview field
void setPreviewEditorAttribute( caf::PdmUiEditorAttribute* attribute );

// Returns the HTML table, or the error message if the content could not be parsed
QString htmlTableFromText( const QString& csvText, int maxRowCount = defaultMaxRowCount );
QString htmlTableFromFile( const QString& filePath, int maxRowCount = defaultMaxRowCount );

} // namespace RimCsvPreviewTools
