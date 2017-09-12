//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmProxyValueField.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"

#include "QMinimizePanel.h"

#include <QDate>
#include <QDateTime>
#include <QGridLayout>
#include <QWidget>


namespace caf
{

// Register default field editor for selected types
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiCheckBoxEditor, bool);

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, QString);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiDateEditor, QDate);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiDateEditor, QDateTime);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, int);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, double);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, float);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, quint64);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiListEditor, std::vector<QString>);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiListEditor, std::vector<int>);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiListEditor, std::vector<unsigned int>);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiListEditor, std::vector<float>);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::PdmUiDefaultObjectEditor()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiDefaultObjectEditor::~PdmUiDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDefaultObjectEditor::createWidget(QWidget* parent)
{
    QWidget* widget = PdmUiWidgetBasedObjectEditor::createWidget(parent);
    
    m_layout = new QGridLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(m_layout);
    
    return widget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::setupFieldsAndGroups(const std::vector<PdmUiItem *>& uiItems, QWidget* parent, const QString& uiConfigName)
{
    recursiveSetupFieldsAndGroups(uiItems, parent, m_layout, uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName)
{
    int currentRowIndex = 0;
    QWidget* previousTabOrderWidget = NULL;

    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(uiConfigName)) continue;

        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            
            QMinimizePanel* groupBox = findOrCreateGroupBox(group, parent, uiConfigName);

            /// Insert the group box at the correct position of the parent layout
            parentLayout->addWidget(groupBox, currentRowIndex, 0, 1, 2);

            QGridLayout* groupBoxLayout = this->groupBoxLayout(groupBox);

            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();
            recursiveSetupFieldsAndGroups(groupChildren, groupBox->contentFrame(), groupBoxLayout, uiConfigName);

            currentRowIndex++;
        }
        else
        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItems[i]);

            PdmUiFieldEditorHandle* fieldEditor = findOrCreateFieldEditor(parent, field, uiConfigName);

            if (fieldEditor)
            {
                fieldEditor->setField(field); 

                // Place the widget(s) into the correct parent and layout
                QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                if (fieldCombinedWidget)
                {
                    fieldCombinedWidget->setParent(parent);
                    parentLayout->addWidget(fieldCombinedWidget, currentRowIndex, 0, 1, 2);
                }
                else
                {
                    PdmUiItemInfo::LabelPosType labelPos = field->uiLabelPosition(uiConfigName);
                    bool labelOnTop = (labelPos == PdmUiItemInfo::TOP);
                    bool editorSpanBoth = labelOnTop;

                    QWidget* fieldEditorWidget = fieldEditor->editorWidget();

                    if (labelPos != PdmUiItemInfo::HIDDEN)
                    {
                        QWidget* fieldLabelWidget  = fieldEditor->labelWidget();
                        if (fieldLabelWidget )
                        {
                            fieldLabelWidget->setParent(parent);

                            // Label widget will span two columns if aligned on top
                            int colSpan = labelOnTop ? 2 : 1;
                            // If the label is on the side, and the editor can expand vertically, allign the label with the top edge of the editor
                            if (!labelOnTop && (fieldEditorWidget->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag))
                                parentLayout->addWidget(fieldLabelWidget, currentRowIndex, 0, 1, colSpan, Qt::AlignTop);
                            else
                                parentLayout->addWidget(fieldLabelWidget, currentRowIndex, 0, 1, colSpan, Qt::AlignVCenter);

                            fieldLabelWidget->show();

                            if (labelOnTop) currentRowIndex++;
                        }
                    }
                    else
                    {
                        QWidget* fieldLabelWidget  = fieldEditor->labelWidget();
                        if (fieldLabelWidget ) fieldLabelWidget->hide();
                        editorSpanBoth = true; // To span both columns when there is no label
                    }

                    if (fieldEditorWidget)
                    {
                        fieldEditorWidget->setParent(parent); // To make sure this widget has the current group box as parent.

                        // Label widget will span two columns if aligned on top
                        int colSpan = editorSpanBoth ? 2 : 1;
                        int colIndex = editorSpanBoth ? 0 : 1;
                        parentLayout->addWidget(fieldEditorWidget, currentRowIndex, colIndex, 1, colSpan, Qt::AlignTop);

                        if (previousTabOrderWidget) QWidget::setTabOrder(previousTabOrderWidget, fieldEditorWidget);
                        
                        previousTabOrderWidget = fieldEditorWidget;
                    }
                }

                fieldEditor->updateUi(uiConfigName);

                currentRowIndex++;
            }
        }
    }
}


} // end namespace caf
