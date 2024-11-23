//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmUiFieldEditorHelper.h"

#include "cafClassTypeName.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmPythonGenerator.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiListEditor.h"
#include "cafQShortenedLabel.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldEditorHandle* caf::PdmUiFieldEditorHelper::createFieldEditorForField( caf::PdmUiFieldHandle* field,
                                                                                     const QString& uiConfigName )
{
    caf::PdmUiFieldEditorHandle* fieldEditor = nullptr;

    // If editor type is specified, find in factory
    if ( !field->uiEditorTypeName( uiConfigName ).isEmpty() )
    {
        fieldEditor =
            caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create( field->uiEditorTypeName( uiConfigName ) );
    }
    else
    {
        // Find the default field editor
        auto    fieldHandle   = field->fieldHandle();
        QString fieldTypeName = qStringTypeName( *fieldHandle );

        if ( fieldTypeName.indexOf( "PdmPtrField" ) != -1 )
        {
            fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
        }
        else if ( fieldTypeName.indexOf( "PdmPtrArrayField" ) != -1 )
        {
            fieldTypeName = caf::PdmUiListEditor::uiEditorTypeName();
        }
        else if ( fieldTypeName.indexOf( "PdmProxyValueField" ) != -1 && fieldTypeName.indexOf( "std::vector" ) != -1 &&
                  fieldTypeName.indexOf( "QString" ) != -1 )
        {
            // The PdmUiTreeSelectionEditor is the default editor for PdmProxyValueField<std::vector<QString>>, but does
            // not work for proxy fields. Use setUiEditorTypeName() to override the default editor.
            // https://github.com/OPM/ResInsight/issues/10483
            fieldTypeName = caf::PdmUiListEditor::uiEditorTypeName();
        }
        else if ( field->toUiBasedQVariant().metaType().id() != QMetaType::QVariantList )
        {
            // Handle a single value field with valueOptions: Make a combobox

            QList<PdmOptionItemInfo> options = field->valueOptions();
            if ( !options.empty() )
            {
                fieldTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
            }
        }

        fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create( fieldTypeName );
    }

    return fieldEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::QShortenedLabel* caf::PdmUiFieldEditorHelper::createLabel( QWidget*                     parent,
                                                                caf::PdmUiFieldEditorHandle* uiFieldEditorHandle )
{
    auto label = new caf::QShortenedLabel( parent );

    if ( uiFieldEditorHandle && uiFieldEditorHandle->uiField() && uiFieldEditorHandle->uiField()->fieldHandle() )
    {
        if ( auto scriptingCapability =
                 uiFieldEditorHandle->uiField()->fieldHandle()->capability<caf::PdmAbstractFieldScriptingCapability>() )
        {
            auto    scriptFieldName     = scriptingCapability->scriptFieldName();
            QString pythonParameterName = caf::PdmPythonGenerator::camelToSnakeCase( scriptFieldName );
            if ( !pythonParameterName.isEmpty() )
            {
                label->configureContextMenu( pythonParameterName );
            }
        }
    }

    return label;
}
