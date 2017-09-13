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

#pragma once

#include "cafPdmUiWidgetBasedObjectEditor.h"

#include <QPointer>

#include <vector>

class QGridLayout;
class QString;
class QWidget;

class WidgetCellIds;

namespace caf 
{

class PdmUiItem;
class PdmUiGroup;
};

//==================================================================================================
/// Automatically layout top level groups into a grid layout
///
/// User defined external widgets can be inserted into grid layout cells, and these cells
/// are excluded for automatic layout
//==================================================================================================
class RiuCustomObjectEditor : public caf::PdmUiWidgetBasedObjectEditor
{
    Q_OBJECT
public:
    RiuCustomObjectEditor();
    ~RiuCustomObjectEditor();

    void defineGridLayout(int rowCount, int columnCount);

    // See QGridLayout::addWidget
    void addWidget(QWidget* widget, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment = 0);
    void removeWidget(QWidget* widget);

    void addBlankCell(int row, int column);

private:
    virtual QWidget*    createWidget(QWidget* parent) override;
    virtual void        recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem*>& topLevelUiItems,
                                                                     const QString& uiConfigName) override;

    bool                isAreaAvailable(int row, int column, int rowSpan, int columnSpan) const;
    bool                isCellIdAvailable(int cellId) const;
    void                resetCellId();
    int                 getNextAvailableCellId();
    int                 cellId(int row, int column) const;
    std::pair<int, int> rowAndColumn(int cellId) const;
    std::vector<int>    cellIds(int row, int column, int rowSpan, int columnSpan) const;

private:
    QPointer<QGridLayout>       m_layout;

    int                         m_rowCount;
    int                         m_columnCount;
    int                         m_currentCellId;

    std::vector<WidgetCellIds>  m_customWidgetAreas;
};
