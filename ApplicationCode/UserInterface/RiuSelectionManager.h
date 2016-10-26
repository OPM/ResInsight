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

#pragma once

#include "cafPdmPointer.h"

#include "cvfBase.h"
#include "cvfColor3.h"
#include "cvfStructGrid.h"

#include <vector>
#include <assert.h>
#include <array>

class RimEclipseView;
class RiuSelectionChangedHandler;
class RiuSelectionItem;
class RimGeoMechView;

//==================================================================================================
//
// 
//
//==================================================================================================
class RiuSelectionManager
{
public:
    RiuSelectionManager();
    ~RiuSelectionManager();

    static RiuSelectionManager* instance();

    // Returns selected items
    // Selection manager owns the selection items
    void selectedItems(std::vector<RiuSelectionItem*>& items) const;
    
    // Append item to selected items
    // SelectionManager takes ownership of the item
    void appendItemToSelection(RiuSelectionItem* item);

    // Set one selected item
    // SelectionManager takes ownership of the item
    void setSelectedItem(RiuSelectionItem* item);

    // Deletes all items in the SelectionManager
    void deleteAllItems();

    bool isEmpty() const;

private:
    void deleteAllItemsFromSelection();

private:
    std::vector < RiuSelectionItem* > m_selection;

    RiuSelectionChangedHandler* m_notificationCenter;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuSelectionItem
{
public:
    enum RiuSelectionType
    {
        ECLIPSE_SELECTION_OBJECT,
        GEOMECH_SELECTION_OBJECT
    };

public:
    RiuSelectionItem() {}
    virtual ~RiuSelectionItem() {};

    virtual RiuSelectionType type() const = 0;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuEclipseSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuEclipseSelectionItem(RimEclipseView* view, size_t gridIndex, size_t cellIndex, size_t nncIndex,
                                     cvf::Color3f color, cvf::StructGridInterface::FaceType face, const cvf::Vec3d& localIntersectionPoint);
    
    virtual ~RiuEclipseSelectionItem() {};

    virtual RiuSelectionType type() const
    {
        return ECLIPSE_SELECTION_OBJECT;
    }

public:
    caf::PdmPointer<RimEclipseView> m_view;
    size_t m_gridIndex;
    size_t m_cellIndex;
    size_t m_nncIndex;
    cvf::Color3f m_color;
    cvf::StructGridInterface::FaceType m_face;
    cvf::Vec3d m_localIntersectionPoint;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuGeoMechSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuGeoMechSelectionItem(RimGeoMechView* view, 
                                     size_t gridIndex, 
                                     size_t cellIndex, 
                                     cvf::Color3f color, 
                                     int elementFace, 
                                     const cvf::Vec3d& localIntersectionPoint);

    explicit RiuGeoMechSelectionItem(RimGeoMechView* view,
                                     size_t gridIndex,
                                     size_t cellIndex,
                                     cvf::Color3f color,
                                     int elementFace,
                                     const cvf::Vec3d& localIntersectionPoint,
                                     const std::array<cvf::Vec3f, 3>& m_intersectionTriangle );
    virtual ~RiuGeoMechSelectionItem() {};

    virtual RiuSelectionType type() const
    {
        return GEOMECH_SELECTION_OBJECT;
    }

public:
    caf::PdmPointer<RimGeoMechView> m_view;
    size_t m_gridIndex;
    size_t m_cellIndex;
    cvf::Color3f m_color;
    int m_elementFace;
    bool m_hasIntersectionTriangle;
    std::array<cvf::Vec3f, 3> m_intersectionTriangle;
    cvf::Vec3d m_localIntersectionPoint;
};

