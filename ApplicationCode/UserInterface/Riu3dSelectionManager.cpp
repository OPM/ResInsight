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

#include "Riu3dSelectionManager.h"

#include "RimGridView.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimSimWellInView.h"
#include "Rim2dIntersectionView.h"
#include "RimWellPath.h"

#include "RivSimWellPipeSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "RiuSelectionChangedHandler.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Riu3dSelectionManager::Riu3dSelectionManager()
    : m_notificationCenter(new RiuSelectionChangedHandler)
{
    m_selection.resize(2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Riu3dSelectionManager::~Riu3dSelectionManager()
{
    deleteAllItemsFromSelection(RUI_APPLICATION_GLOBAL);
    deleteAllItemsFromSelection(RUI_TEMPORARY);

    delete m_notificationCenter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Riu3dSelectionManager* Riu3dSelectionManager::instance()
{
    static Riu3dSelectionManager* singleton = new Riu3dSelectionManager;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3dSelectionManager::selectedItems(std::vector<RiuSelectionItem*>& items, int role) const
{
    const std::vector<RiuSelectionItem*>& s = m_selection[role];

    items = s;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionItem* Riu3dSelectionManager::selectedItem(int role /*= RUI_APPLICATION_GLOBAL*/) const
{
    const std::vector<RiuSelectionItem*>& s = m_selection[role];

    if (s.size() == 1)
    {
        if (s[0])
        {
            return s[0];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3dSelectionManager::appendItemToSelection(RiuSelectionItem* item, int role)
{
    std::vector<RiuSelectionItem*>& s = m_selection[role];

    s.push_back(item);

    if (role == RUI_APPLICATION_GLOBAL) m_notificationCenter->handleItemAppended(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3dSelectionManager::setSelectedItem(RiuSelectionItem* item, int role)
{
    deleteAllItemsFromSelection(role);

    std::vector<RiuSelectionItem*>& s = m_selection[role];

    s.push_back(item);

    if (role == RUI_APPLICATION_GLOBAL) m_notificationCenter->handleSetSelectedItem(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3dSelectionManager::deleteAllItems(int role)
{
    if (!isEmpty(role))
    {
        deleteAllItemsFromSelection(role);

        if (role == RUI_APPLICATION_GLOBAL) m_notificationCenter->handleSelectionDeleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Riu3dSelectionManager::isEmpty(int role) const
{
    const std::vector<RiuSelectionItem*>& s = m_selection[role];

    return s.size() == 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3dSelectionManager::deleteAllItemsFromSelection(int role)
{
    std::vector<RiuSelectionItem*>& s = m_selection[role];

    for (RiuSelectionItem* item : s)
    {
        delete item;
    }

    s.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem::RiuEclipseSelectionItem(RimEclipseView* view,
                                                 size_t gridIndex,
                                                 size_t cellIndex,
                                                 size_t nncIndex,
                                                 cvf::Color3f color,
                                                 cvf::StructGridInterface::FaceType face,
                                                 const cvf::Vec3d& localIntersectionPointInDisplay)
    :   m_view(view),
        m_gridIndex(gridIndex),
        m_gridLocalCellIndex(cellIndex),
        m_nncIndex(nncIndex),
        m_color(color),
        m_face(face),
        m_localIntersectionPointInDisplay(localIntersectionPointInDisplay)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuGeoMechSelectionItem::RiuGeoMechSelectionItem(RimGeoMechView*   view,
                                                 size_t            gridIndex,
                                                 size_t            cellIndex,
                                                 cvf::Color3f      color,
                                                 int               elementFace,
                                                 const cvf::Vec3d& localIntersectionPointInDisplay)
    : m_view(view)
    , m_gridIndex(gridIndex)
    , m_cellIndex(cellIndex)
    , m_color(color)
    , m_elementFace(elementFace)
    , m_hasIntersectionTriangle(false)
    , m_localIntersectionPointInDisplay(localIntersectionPointInDisplay)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGeoMechSelectionItem::RiuGeoMechSelectionItem(RimGeoMechView*                  view,
                                                 size_t                           gridIndex,
                                                 size_t                           cellIndex,
                                                 cvf::Color3f                     color,
                                                 int                              elementFace,
                                                 const cvf::Vec3d&                localIntersectionPointInDisplay,
                                                 const std::array<cvf::Vec3f, 3>& intersectionTriangle)
    : m_view(view)
    , m_gridIndex(gridIndex)
    , m_cellIndex(cellIndex)
    , m_color(color)
    , m_elementFace(elementFace)
    , m_hasIntersectionTriangle(true)
    , m_intersectionTriangle(intersectionTriangle)
    , m_localIntersectionPointInDisplay(localIntersectionPointInDisplay)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Riu2dIntersectionSelectionItem::Riu2dIntersectionSelectionItem(Rim2dIntersectionView* view, RiuSelectionItem *selItem)
{
    m_view = view;
    m_eclipseSelItem = dynamic_cast<RiuEclipseSelectionItem*>(selItem);
    m_geoMechSelItem = dynamic_cast<RiuGeoMechSelectionItem*>(selItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Riu2dIntersectionSelectionItem::~Riu2dIntersectionSelectionItem()
{
    if (m_eclipseSelItem) delete m_eclipseSelItem;
    if (m_geoMechSelItem) delete m_geoMechSelItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmPointer<Rim2dIntersectionView> Riu2dIntersectionSelectionItem::view() const
{
    return m_view;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem* Riu2dIntersectionSelectionItem::eclipseSelectionItem() const
{
    return m_eclipseSelItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuGeoMechSelectionItem* Riu2dIntersectionSelectionItem::geoMechSelectionItem() const
{
    return m_geoMechSelItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellPathSelectionItem::RiuWellPathSelectionItem(const RivWellPathSourceInfo* wellPathSourceInfo,
                                                   const cvf::Vec3d& pipeCenterLineIntersectionInDomainCoords,
                                                   double measuredDepth)
    : m_pipeCenterlineIntersectionInDomainCoords(pipeCenterLineIntersectionInDomainCoords),
    m_measuredDepth(measuredDepth)
{
    m_wellpath = wellPathSourceInfo->wellPath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSimWellSelectionItem::RiuSimWellSelectionItem(RimSimWellInView* simwell, 
                                                  cvf::Vec3d m_domainCoord,
                                                  size_t m_branchIndex)
    : m_simWell(simwell),
    m_domainCoord(m_domainCoord),
    m_branchIndex(m_branchIndex)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuGeneralSelectionItem::RiuGeneralSelectionItem(caf::PdmObject* object)
    : m_object(object)
{
}
