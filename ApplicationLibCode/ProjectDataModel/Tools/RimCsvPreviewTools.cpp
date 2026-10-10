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

#include "RimCsvPreviewTools.h"

#include "RifCsvHtmlTableTools.h"

#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmXmlFieldHandle.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvPreviewTools::initPreviewField( caf::PdmField<QString>& field )
{
    field.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    field.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    field.uiCapability()->setUiReadOnly( true );
    field.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvPreviewTools::setPreviewEditorAttribute( caf::PdmUiEditorAttribute* attribute )
{
    if ( auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute ) )
    {
        myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
        myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCsvPreviewTools::htmlTableFromText( const QString& csvText, int maxRowCount )
{
    auto result = RifCsvHtmlTableTools::generateHtmlTableFromText( csvText, maxRowCount );
    return result ? *result : result.error();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCsvPreviewTools::htmlTableFromFile( const QString& filePath, int maxRowCount )
{
    auto result = RifCsvHtmlTableTools::generateHtmlTableFromFile( filePath, maxRowCount );
    return result ? *result : result.error();
}
