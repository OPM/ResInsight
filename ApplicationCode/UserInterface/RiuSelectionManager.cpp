/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuSelectionManager.h"

#include "RimEclipseView.h"
#include "RimGeoMechView.h"

#include "RiuSelectionChangedHandler.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionManager::RiuSelectionManager()
    : m_notificationCenter(new RiuSelectionChangedHandler)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionManager::~RiuSelectionManager()
{
    delete m_notificationCenter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionManager* RiuSelectionManager::instance()
{
    static RiuSelectionManager* singleton = new RiuSelectionManager;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionManager::selectedItems(std::vector<RiuSelectionItem*>& items) const
{
    items = m_selection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionManager::appendItemToSelection(RiuSelectionItem* item)
{
    m_selection.push_back(item);

    m_notificationCenter->handleItemAppended(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionManager::setSelectedItem(RiuSelectionItem* item)
{
    deleteAllItemsFromSelection();

    m_selection.push_back(item);

    m_notificationCenter->handleSetSelectedItem(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionManager::deleteAllItems()
{
    if (m_selection.size() == 0) return;

    deleteAllItemsFromSelection();

    m_notificationCenter->handleSelectionDeleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuSelectionManager::isEmpty() const
{
    return m_selection.size() == 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionManager::deleteAllItemsFromSelection()
{
    for (size_t i = 0; i < m_selection.size(); i++)
    {
        delete m_selection[i];
    }

    m_selection.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem::RiuEclipseSelectionItem(RimEclipseView* view, size_t gridIndex, size_t cellIndex, size_t nncIndex, cvf::Color3f color, cvf::StructGridInterface::FaceType face, const cvf::Vec3d& localIntersectionPoint)
    :   m_view(view),
        m_gridIndex(gridIndex),
        m_cellIndex(cellIndex),
        m_nncIndex(nncIndex),
        m_color(color),
        m_face(face),
        m_localIntersectionPoint(localIntersectionPoint)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuGeoMechSelectionItem::RiuGeoMechSelectionItem(RimGeoMechView* view, 
                                                 size_t gridIndex, 
                                                 size_t cellIndex, 
                                                 cvf::Color3f color, 
                                                 int elementFace, 
                                                 const cvf::Vec3d& localIntersectionPoint)
    :   m_view(view),
        m_gridIndex(gridIndex),
        m_cellIndex(cellIndex),
        m_color(color),
        m_elementFace(elementFace),
        m_localIntersectionPoint(localIntersectionPoint)
{
}
