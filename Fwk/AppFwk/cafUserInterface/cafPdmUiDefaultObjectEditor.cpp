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

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiListEditor.h"


#include <QWidget>
#include <QGridLayout>

#include <assert.h>
#include <typeinfo>



namespace caf
{

// Register default field editor for selected types
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiCheckBoxEditor, bool);

CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, QString);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, int);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, double);
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(PdmUiLineEditor, float);
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
    m_mainWidget = new QWidget(parent);
    m_layout     = new QGridLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_mainWidget->setLayout(m_layout);
    
    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmUiOrdering config;
    if (pdmObject())
    {
        pdmObject()->uiOrdering(uiConfigName, config);
    }

    // Set all fieldViews to be unvisited
    std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        it->second->setField(NULL);
    }

    // Set all group Boxes to be unvisited
    m_newGroupBoxes.clear();

    const std::vector<PdmUiItem*>& uiItems = config.uiItems();

    recursiveSetupFieldsAndGroups(uiItems, m_mainWidget, m_layout, uiConfigName);

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector< QString > fvhToRemoveFromMap;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        if (it->second->field() == 0)
        {
            PdmUiFieldEditorHandle* fvh = it->second;
            delete fvh;
            fvhToRemoveFromMap.push_back(it->first);               
        }
    }

    for (size_t i = 0; i < fvhToRemoveFromMap.size(); ++i)
    {
        m_fieldViews.erase(fvhToRemoveFromMap[i]);
    }

    // Remove all unmentioned group boxes

    std::map<QString, QPointer<QGroupBox> >::iterator itOld;
    std::map<QString, QPointer<QGroupBox> >::iterator itNew;

    for (itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld )
    {
        itNew = m_newGroupBoxes.find(itOld->first);
        if (itNew == m_newGroupBoxes.end()) 
        {
            // The old groupBox is not present anymore, get rid of it
            if (!itOld->second.isNull()) delete itOld->second;
        }
    }
    m_groupBoxes = m_newGroupBoxes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDefaultObjectEditor::recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName)
{
    int currentRowIndex = 0;
    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(uiConfigName)) continue;

        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = uiItems[i]->uiName();
            QGroupBox* groupBox = NULL;
            QGridLayout* groupBoxLayout = NULL;

            // Find or create groupBox
            std::map<QString, QPointer<QGroupBox> >::iterator it;
            it = m_groupBoxes.find(groupBoxKey);

            if (it == m_groupBoxes.end())
            {
                groupBox = new QGroupBox( parent );
                groupBox->setTitle(uiItems[i]->uiName());
                groupBoxLayout = new QGridLayout();
                groupBox->setLayout(groupBoxLayout);

                m_newGroupBoxes[groupBoxKey] = groupBox;
            }
            else
            {
                groupBox = it->second;
                assert(groupBox);
                
                groupBoxLayout = dynamic_cast<QGridLayout*>(groupBox->layout());
                assert(groupBoxLayout);
                
                m_newGroupBoxes[groupBoxKey] = groupBox;
            }

            /// Insert the group box at the correct position of the parent layout

            parentLayout->addWidget(groupBox, currentRowIndex, 0, 1, 2);
            recursiveSetupFieldsAndGroups(groupChildren, groupBox, groupBoxLayout, uiConfigName);
            currentRowIndex++;
        }
        else
        {
            PdmFieldHandle* field = dynamic_cast<PdmFieldHandle*>(uiItems[i]);
            PdmUiFieldEditorHandle* fieldEditor = NULL;

            // Find or create FieldEditor
            std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
            it = m_fieldViews.find(field->keyword());

            if (it == m_fieldViews.end())
            {
                // If editor type is specified, find in factory
                if ( !uiItems[i]->uiEditorTypeName(uiConfigName).isEmpty() )
                {
                    fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(field->uiEditorTypeName(uiConfigName));
                }
                else
                { 
                    // Find the default field editor

                    QString editorTypeName = qStringTypeName(*field);

                    // Handle a single value field with valueOptions: Make a combobox

                    if (field->uiValue().type() != QVariant::List)
                    {
                        bool useOptionsOnly = true; 
                        QList<PdmOptionItemInfo> options = field->valueOptions( &useOptionsOnly);

                        if (!options.empty())
                        {
                            editorTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
                        }
                    }

                    fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(editorTypeName);
                }

                if (fieldEditor)
                {
                    m_fieldViews[field->keyword()] = fieldEditor;
                    fieldEditor->createWidgets(parent);
                }
            }
            else
            {
                fieldEditor = it->second;
            }

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
                    }

                }

                fieldEditor->updateUi(uiConfigName);

                currentRowIndex++;
            }

        }
    }
}


} // end namespace caf
