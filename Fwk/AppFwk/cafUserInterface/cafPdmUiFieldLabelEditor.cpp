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

#include "cafPdmUiFieldLabelEditor.h"
#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmPythonGenerator.h"
#include "cafPdmUiFieldHandle.h"
#include "cafQShortenedLabel.h"

#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* caf::PdmUiFieldEditorHandleLabel::createLabelWidget( QWidget* parent )
{
    m_label = createLabel( parent, this );

    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::QShortenedLabel* caf::PdmUiFieldEditorHandleLabel::createLabel( QWidget*                     parent,
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
