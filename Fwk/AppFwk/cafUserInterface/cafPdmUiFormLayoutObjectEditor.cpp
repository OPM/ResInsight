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


#include "cafPdmUiFormLayoutObjectEditor.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldEditorHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmXmlObjectHandle.h"

#include "cafAssert.h"

#include "QMinimizePanel.h"

#include <QGridLayout>
#include <QFrame>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiFormLayoutObjectEditor::PdmUiFormLayoutObjectEditor()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiFormLayoutObjectEditor::~PdmUiFormLayoutObjectEditor()
{
    // If there are field editor present, the usage of this editor has not cleared correctly
    // The intended usage is to call the method setPdmObject(NULL) before closing the dialog
    CAF_ASSERT(m_fieldViews.size() == 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInGridLayoutColumn(
    const PdmUiOrdering& uiOrdering,
    QWidget*             containerWidgetWithGridLayout,
    const QString&       uiConfigName,
    QWidget**            previousTabOrderWidget)
{
    CAF_ASSERT(containerWidgetWithGridLayout);

    // Currently, only QGridLayout is supported
    QGridLayout* parentLayout = dynamic_cast<QGridLayout*>(containerWidgetWithGridLayout->layout());    
    CAF_ASSERT(parentLayout);

    PdmUiOrdering::TableLayout tableLayout = uiOrdering.calculateTableLayout(uiConfigName);

    int totalRows    = static_cast<int>(tableLayout.size());
    int totalColumns = uiOrdering.nrOfColumns(tableLayout);

    for (int currentRowIndex = 0; currentRowIndex < totalRows; ++currentRowIndex)
    {
        int currentColumn = 0;

        const PdmUiOrdering::RowLayout& uiItemsInRow = tableLayout[currentRowIndex];

        int columnsRequiredForCurrentRow = uiOrdering.nrOfRequiredColumnsInRow(uiItemsInRow);
        int nrOfExpandingItemsInRow      = uiOrdering.nrOfExpandingItemsInRow(uiItemsInRow);
        int spareColumnsInRow            = totalColumns - columnsRequiredForCurrentRow;

        std::div_t columnsDiv = { 0, 0 };
        if (spareColumnsInRow && nrOfExpandingItemsInRow)
        {
            columnsDiv = std::div(spareColumnsInRow, nrOfExpandingItemsInRow);
        }

        for (size_t i = 0; i < uiItemsInRow.size(); ++i)
        {
            PdmUiItem*                   currentItem   = uiItemsInRow[i].first;
            PdmUiOrdering::LayoutOptions currentLayout = uiItemsInRow[i].second;

            int minimumItemColumnSpan = 0, minimumLabelColumnSpan = 0, minimumFieldColumnSpan = 0;
            uiOrdering.nrOfColumnsRequiredForItem(uiItemsInRow[i], &minimumItemColumnSpan, &minimumLabelColumnSpan, &minimumFieldColumnSpan);
            bool isExpandingItem = currentLayout.totalColumnSpan == PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN;

            int spareColumnsToAssign = 0;
            if (isExpandingItem)
            {
                spareColumnsToAssign += columnsDiv.quot;
                if (i == 0) spareColumnsToAssign += columnsDiv.rem;
            }

            int itemColumnSpan  = minimumItemColumnSpan + spareColumnsToAssign;
            
            if (currentItem->isUiGroup())
            {
                recursivelyAddGroupToGridLayout(currentItem, containerWidgetWithGridLayout,
                                                uiConfigName, parentLayout, currentRowIndex,
                                                currentColumn, itemColumnSpan, previousTabOrderWidget);
                currentColumn += itemColumnSpan;
            }
            else
            {
                // Also assign required item space that isn't taken up by field and label
                spareColumnsToAssign += minimumItemColumnSpan - (minimumLabelColumnSpan + minimumFieldColumnSpan);

                PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(currentItem);

                PdmUiFieldEditorHandle* fieldEditor = findOrCreateFieldEditor(containerWidgetWithGridLayout, field, uiConfigName);

                if (fieldEditor)
                {
                    fieldEditor->setUiField(field);

                    // Place the widget(s) into the correct parent and layout
                    QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                    if (fieldCombinedWidget)
                    {
                        fieldCombinedWidget->setParent(containerWidgetWithGridLayout);
                        parentLayout->addWidget(fieldCombinedWidget, currentRowIndex, currentColumn, 1, itemColumnSpan);
                    }
                    else
                    {
                        QWidget* fieldEditorWidget = fieldEditor->editorWidget();
                        if (!fieldEditorWidget) continue;
                        int fieldColumnSpan = minimumFieldColumnSpan;

                        QWidget* fieldLabelWidget = fieldEditor->labelWidget();                                                
                        PdmUiItemInfo::LabelPosType labelPos = PdmUiItemInfo::HIDDEN;

                        if (fieldLabelWidget)
                        {
                            labelPos = field->uiLabelPosition(uiConfigName);

                            if (labelPos == PdmUiItemInfo::HIDDEN)
                            {
                                fieldLabelWidget->hide();
                            }
                            else if (labelPos == PdmUiItemInfo::TOP)
                            {
                                QVBoxLayout* labelAndFieldVerticalLayout = new QVBoxLayout();
                                parentLayout->addLayout(
                                    labelAndFieldVerticalLayout, currentRowIndex, currentColumn, 1, itemColumnSpan, Qt::AlignTop);
                                labelAndFieldVerticalLayout->addWidget(fieldLabelWidget, 0, Qt::AlignTop);
                                labelAndFieldVerticalLayout->addWidget(fieldEditorWidget, 1, Qt::AlignTop);
                                
                                // Apply margins determined by the editor type
                                fieldLabelWidget->setContentsMargins(fieldEditor->labelContentMargins());
                            }
                            else
                            {
                                CAF_ASSERT(labelPos == PdmUiItemInfo::LEFT);
                                int leftLabelColumnSpan = minimumLabelColumnSpan;
                                if (currentLayout.leftLabelColumnSpan == PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN &&
                                    currentLayout.totalColumnSpan     != PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN)
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign;
                                    spareColumnsToAssign = 0;
                                }
                                else if (currentLayout.leftLabelColumnSpan == PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN)
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign / 2;
                                    spareColumnsToAssign -= spareColumnsToAssign / 2;
                                }

                                parentLayout->addWidget(fieldLabelWidget, currentRowIndex, currentColumn, 1, leftLabelColumnSpan, Qt::AlignTop);
                                currentColumn += leftLabelColumnSpan;

                                // Apply margins determined by the editor type
                                fieldLabelWidget->setContentsMargins(fieldEditor->labelContentMargins());
                            }
                        }

                        if (labelPos != PdmUiItemInfo::TOP) // Already added if TOP
                        {
                            fieldColumnSpan += spareColumnsToAssign;

                            CAF_ASSERT(fieldColumnSpan >= 1 && "Need at least one column for the field");
                            fieldColumnSpan = std::max(1, fieldColumnSpan);

                            parentLayout->addWidget(fieldEditorWidget, currentRowIndex, currentColumn, 1, fieldColumnSpan, Qt::AlignTop);
                            currentColumn += fieldColumnSpan;
                        }

                        if (previousTabOrderWidget && *previousTabOrderWidget)
                        {
                            QWidget::setTabOrder(*previousTabOrderWidget, fieldEditorWidget);
                        }

                        //previousTabOrderWidget = &fieldEditorWidget;
                    }
                    fieldEditor->updateUi(uiConfigName);
                }
            }
        }
        int stretchFactor = currentRowIndex == totalRows - 1 ? 1 : 0;
        parentLayout->setRowStretch(currentRowIndex, stretchFactor);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::recursivelyAddGroupToGridLayout(PdmUiItem*     currentItem,
                                                                       QWidget*       containerWidgetWithGridLayout,
                                                                       const QString& uiConfigName,
                                                                       QGridLayout*   parentLayout,
                                                                       int            currentRowIndex,
                                                                       int            currentColumn,
                                                                       int            itemColumnSpan,
                                                                       QWidget** previousTabOrderWidget)
{
    PdmUiGroup* group = static_cast<PdmUiGroup*>(currentItem);

    QMinimizePanel* groupBox = findOrCreateGroupBox(containerWidgetWithGridLayout, group, uiConfigName);

    /// Insert the group box at the correct position of the parent layout
    parentLayout->addWidget(groupBox, currentRowIndex, currentColumn, 1, itemColumnSpan);

    recursivelyConfigureAndUpdateUiOrderingInGridLayoutColumn(*group, groupBox->contentFrame(), uiConfigName, previousTabOrderWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiFormLayoutObjectEditor::isUiGroupExpanded(const PdmUiGroup* uiGroup) const
{
    if (uiGroup->hasForcedExpandedState()) return uiGroup->forcedExpandedState();

    auto kwMapPair = m_objectKeywordGroupUiNameExpandedState.find(pdmObject()->xmlCapability()->classKeyword());
    if (kwMapPair != m_objectKeywordGroupUiNameExpandedState.end())
    {
        QString keyword = uiGroup->keyword();

        auto uiNameExpStatePair = kwMapPair->second.find(keyword);
        if (uiNameExpStatePair != kwMapPair->second.end())
        {
            return uiNameExpStatePair->second;
        }
    }

    return uiGroup->isExpandedByDefault();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMinimizePanel* caf::PdmUiFormLayoutObjectEditor::findOrCreateGroupBox(QWidget* parent, PdmUiGroup* group, const QString& uiConfigName)
{
    QString groupBoxKey = group->keyword();
    QMinimizePanel* groupBox = nullptr;
    QGridLayout* groupBoxLayout = nullptr;

    // Find or create groupBox
    std::map<QString, QPointer<QMinimizePanel> >::iterator it;
    it = m_groupBoxes.find(groupBoxKey);

    if (it == m_groupBoxes.end())
    {
        groupBox = new QMinimizePanel(parent);
        groupBox->enableFrame(group->enableFrame());
        groupBox->setTitle(group->uiName(uiConfigName));
        groupBox->setObjectName(group->keyword());
        groupBoxLayout = new QGridLayout();
        if (!group->enableFrame())
        {
            groupBoxLayout->setContentsMargins(0, 0, 0, 0);
            groupBoxLayout->setHorizontalSpacing(0);
        }
        groupBox->contentFrame()->setLayout(groupBoxLayout);
        connect(groupBox, SIGNAL(expandedChanged(bool)), this, SLOT(groupBoxExpandedStateToggled(bool)));

        m_newGroupBoxes[groupBoxKey] = groupBox;
    }
    else
    {
        groupBox = it->second;
        CAF_ASSERT(groupBox);

        m_newGroupBoxes[groupBoxKey] = groupBox;
    }

    // Set Expanded state
    bool isExpanded = isUiGroupExpanded(group);
    groupBox->setExpanded(isExpanded);

    // Update the title to be able to support dynamic group names
    groupBox->setTitle(group->uiName(uiConfigName));

    return groupBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldEditorHandle* caf::PdmUiFormLayoutObjectEditor::findOrCreateFieldEditor(QWidget* parent, PdmUiFieldHandle* field, const QString& uiConfigName)
{
    caf::PdmUiFieldEditorHandle* fieldEditor = nullptr;

    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it = m_fieldViews.find(field->fieldHandle());

    if (it == m_fieldViews.end())
    {
        fieldEditor = PdmUiFieldEditorHelper::fieldEditorForField(field, uiConfigName);

        if (fieldEditor)
        {
            m_fieldViews[field->fieldHandle()] = fieldEditor;
            fieldEditor->createWidgets(parent);
        }
        else
        {
            // This assert happens if no editor is available for a given field
            // If the macro for registering the editor is put as the single statement
            // in a cpp file, a dummy static class must be used to make sure the compile unit
            // is included
            //
            // See cafPdmUiCoreColor3f and cafPdmUiCoreVec3d

            // This assert will trigger for PdmChildArrayField and PdmChildField
            // Consider to exclude assert or add editors for these types if the assert is reintroduced
            //CAF_ASSERT(false);
        }
    }
    else
    {
        fieldEditor = it->second;
    }

    return fieldEditor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::groupBoxExpandedStateToggled(bool isExpanded)
{
    if (!this->pdmObject()->xmlCapability()) return;

    QString objKeyword = this->pdmObject()->xmlCapability()->classKeyword();
    QMinimizePanel* panel = dynamic_cast<QMinimizePanel*>(this->sender());

    if (!panel) return;

    m_objectKeywordGroupUiNameExpandedState[objKeyword][panel->objectName()] = isExpanded;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::cleanupBeforeSettingPdmObject()
{
    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        PdmUiFieldEditorHandle* fvh = it->second;
        delete fvh;
    }
    m_fieldViews.clear();

    m_newGroupBoxes.clear();

    std::map<QString, QPointer<QMinimizePanel> >::iterator groupIt;
    for (groupIt = m_groupBoxes.begin(); groupIt != m_groupBoxes.end(); ++groupIt)
    {
        if (!groupIt->second.isNull()) groupIt->second->deleteLater();
    }

    m_groupBoxes.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    caf::PdmUiOrdering config;
    if (pdmObject())
    {
        caf::PdmUiObjectHandle* uiObject = uiObj(pdmObject());
        if (uiObject)
        {
            uiObject->uiOrdering(uiConfigName, config);
        }
    }

    // Set all fieldViews to be unvisited
    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        it->second->setUiField(nullptr);
    }

    // Set all group Boxes to be unvisited
    m_newGroupBoxes.clear();

    recursivelyConfigureAndUpdateTopLevelUiOrdering(config, uiConfigName);

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector< PdmFieldHandle* > fvhToRemoveFromMap;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        if (it->second->uiField() == nullptr)
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

    std::map<QString, QPointer<QMinimizePanel> >::iterator itOld;
    std::map<QString, QPointer<QMinimizePanel> >::iterator itNew;

    for (itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld)
    {
        itNew = m_newGroupBoxes.find(itOld->first);
        if (itNew == m_newGroupBoxes.end())
        {
            // The old groupBox is not present anymore, get rid of it
            if (!itOld->second.isNull()) delete itOld->second;
        }
    }
    m_groupBoxes = m_newGroupBoxes;

    // Notify pdm object when widgets have been created
    caf::PdmUiObjectHandle* uiObject = uiObj(pdmObject());
    if (uiObject)
    {
        uiObject->onEditorWidgetsCreated();
    }
}

//--------------------------------------------------------------------------------------------------
/// Unused. Should probably remove
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::recursiveVerifyUniqueNames(const std::vector<PdmUiItem*>& uiItems, 
                                                                   const QString& uiConfigName, 
                                                                   std::set<QString>* fieldKeywordNames, 
                                                                   std::set<QString>* groupNames)
{
    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = group->keyword();

            if (groupNames->find(groupBoxKey) != groupNames->end())
            {
                // It is not supported to have two groups with identical names
                CAF_ASSERT(false);
            }
            else
            {
                groupNames->insert(groupBoxKey);
            }

            recursiveVerifyUniqueNames(groupChildren, uiConfigName, fieldKeywordNames, groupNames);
        }
        else
        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItems[i]);

            QString fieldKeyword = field->fieldHandle()->keyword();
            if (fieldKeywordNames->find(fieldKeyword) != fieldKeywordNames->end())
            {
                // It is not supported to have two fields with identical names
                CAF_ASSERT(false);
            }
            else
            {
                fieldKeywordNames->insert(fieldKeyword);
            }
        }
    }
}

