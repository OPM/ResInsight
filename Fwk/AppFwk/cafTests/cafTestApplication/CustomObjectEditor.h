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


#pragma once

#include "cafPdmUiObjectEditorHandle.h"

#include <map>

#include <QGroupBox>
#include <QPointer>
#include <QString>
#include <QWidget>
#include "QMinimizePanel.h"

class QGridLayout;

namespace caf 
{
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiGroup;


class WidgetAndArea
{
public:
    WidgetAndArea(QWidget* w, const std::vector<int>& occupiedCellIds)
        : m_customWidget(w),
        m_customWidgetCellIds(occupiedCellIds)
    {
    }

    QWidget*           m_customWidget;
    std::vector<int>   m_customWidgetCellIds;
};


//==================================================================================================
/// 
//==================================================================================================

class CustomObjectEditor : public PdmUiObjectEditorHandle
{
    Q_OBJECT
public:
    CustomObjectEditor();
    ~CustomObjectEditor();

    void defineGrid(int rows, int columns);

    // See QGridLayout::addWidget
    void addWidget(QWidget* w, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = 0);

    void addBlankCell(int row, int column);
    bool isCellOccupied(int cellId) const;

    void removeWidget(QWidget* w);
    bool isAreaAvailable(int row, int column, int rowSpan, int columnSpan) const;

protected:
    virtual QWidget*    createWidget(QWidget* parent) override;
    virtual void        configureAndUpdateUi(const QString& uiConfigName) override;
    virtual void        cleanupBeforeSettingPdmObject() override;

protected slots:
    void                groupBoxExpandedStateToggled(bool isExpanded);

private:
    void                resetDynamicCellCounter();
    std::pair<int, int> rowAndCell(int cellId) const;
    int                 cellId(int row, int column) const;

    int                 getNextAvailableCellId();

    std::vector<int>    cellIds(int row, int column, int rowSpan, int columnSpan) const;

    void                recursiveSetupFieldsAndGroupsRoot(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName);
    void                recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName);
    bool                isUiGroupExpanded(const PdmUiGroup* uiGroup);
    void                recursiveVerifyUniqueNames(const std::vector<PdmUiItem*>& uiItems, const QString& uiConfigName, std::set<QString>* fieldKeywordNames, std::set<QString>* groupNames);

    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>  m_fieldViews; 
    std::map<QString, QPointer<QMinimizePanel> >        m_groupBoxes;
    std::map<QString, QPointer<QMinimizePanel> >        m_newGroupBoxes; ///< used temporarily to store the new(complete) set of group boxes

    QPointer<QWidget>                                   m_mainWidget;
    QPointer<QGridLayout>                               m_layout;

    std::map<QString, std::map<QString, bool> >         m_objectKeywordGroupUiNameExpandedState; 

    int m_rowCount;
    int m_columnCount;
    int m_dynamicCellIndex;

    std::vector<WidgetAndArea>  m_customWidgetAreas;
};



} // end namespace caf
