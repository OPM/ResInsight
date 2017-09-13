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

#include "RiuCustomObjectEditor.h"

#include "cafPdmUiGroup.h"

#include "QMinimizePanel.h"

#include <QFrame>
#include <QGridLayout>


//==================================================================================================
/// 
//==================================================================================================
class WidgetCellIds
{
public:
    WidgetCellIds(QWidget* w, const std::vector<int>& occupiedCellIds)
        : m_customWidget(w),
        m_customWidgetCellIds(occupiedCellIds)
    {
    }

    QWidget*           m_customWidget;
    std::vector<int>   m_customWidgetCellIds;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuCustomObjectEditor::RiuCustomObjectEditor()
{
    m_columnCount = 3;
    m_rowCount = 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuCustomObjectEditor::~RiuCustomObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::defineGridLayout(int rowCount, int columnCount)
{
    m_rowCount = rowCount;
    m_columnCount = columnCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::addWidget(QWidget* widget, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment /*= 0*/)
{
    CAF_ASSERT(isAreaAvailable(row, column, rowSpan, columnSpan));

    m_customWidgetAreas.push_back(WidgetCellIds(widget, RiuCustomObjectEditor::cellIds(row, column, rowSpan, columnSpan)));

    // The ownership of item is transferred to the layout, and it's the layout's responsibility to delete it.
    m_layout->addWidget(widget, row, column, rowSpan, columnSpan, alignment);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::removeWidget(QWidget* widget)
{
    size_t indexToRemove = size_t(-1);
    for (size_t i = 0; i < m_customWidgetAreas.size(); i++)
    {
        if (widget == m_customWidgetAreas[i].m_customWidget)
        {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove != size_t(-1))
    {
        m_layout->removeWidget(widget);

        m_customWidgetAreas.erase(m_customWidgetAreas.begin() + indexToRemove);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::addBlankCell(int row, int column)
{
    m_customWidgetAreas.push_back(WidgetCellIds(nullptr, RiuCustomObjectEditor::cellIds(row, column, 1, 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RiuCustomObjectEditor::createWidget(QWidget* parent)
{
    QWidget* widget = new QWidget(parent);

    m_layout = new QGridLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(m_layout);

    return widget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem*>& topLevelUiItems, const QString& uiConfigName)
{
    if (!m_layout) return;

    resetCellId();

    QWidget* previousTabOrderWidget = NULL;

    for (size_t i = 0; i < topLevelUiItems.size(); ++i)
    {
        if (topLevelUiItems[i]->isUiHidden(uiConfigName)) continue;

        if (topLevelUiItems[i]->isUiGroup())
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>(topLevelUiItems[i]);
            QMinimizePanel* groupBox = findOrCreateGroupBox(this->widget(), group, uiConfigName);

            /// Insert the group box at the correct position of the parent layout
            int nextCellId = getNextAvailableCellId();
            std::pair<int, int> rowCol = rowAndColumn(nextCellId);
            m_layout->addWidget(groupBox, rowCol.first, rowCol.second, 1, 1);

            const std::vector<caf::PdmUiItem*>& groupChildren = group->uiItems();
            recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn(groupChildren, groupBox->contentFrame(), uiConfigName);
        }

        // NB! Only groups at top level are handled, fields at top level are not added to layout
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuCustomObjectEditor::isAreaAvailable(int row, int column, int rowSpan, int columnSpan) const
{
    auto candidateCells = RiuCustomObjectEditor::cellIds(row, column, rowSpan, columnSpan);
    for (auto candidateCell : candidateCells)
    {
        if (!isCellIdAvailable(candidateCell))
        {
            return false;
        }
    }

    if (row + rowSpan > m_rowCount) return false;
    if (column + columnSpan > m_columnCount) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuCustomObjectEditor::isCellIdAvailable(int cellId) const
{
    for (auto customArea : m_customWidgetAreas)
    {
        for (auto occupiedCell : customArea.m_customWidgetCellIds)
        {
            if (cellId == occupiedCell)
            {
                return false;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuCustomObjectEditor::resetCellId()
{
    m_currentCellId = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuCustomObjectEditor::rowAndColumn(int cellId) const
{
    int column  = cellId % m_columnCount;
    int row     = cellId / m_columnCount;

    return std::make_pair(row, column);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuCustomObjectEditor::cellId(int row, int column) const
{
    return row * m_columnCount + column;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuCustomObjectEditor::getNextAvailableCellId()
{
    while (!isCellIdAvailable(m_currentCellId) && m_currentCellId < (m_rowCount * m_columnCount))
    {
        m_currentCellId++;
    }

    if (!isCellIdAvailable(m_currentCellId))
    {
        return -1;
    }
    else
    {
        return m_currentCellId++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RiuCustomObjectEditor::cellIds(int row, int column, int rowSpan, int columnSpan) const
{
    std::vector<int> cells;

    for (auto r = row; r < row + rowSpan; r++)
    {
        for (auto c = column; c < column + columnSpan; c++)
        {
            cells.push_back(cellId(r, c));
        }
    }

    return cells;
}
