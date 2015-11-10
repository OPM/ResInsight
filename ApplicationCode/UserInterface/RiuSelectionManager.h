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

#include <vector>

class RimEclipseView;

class RiuSelectionItem;

//==================================================================================================
//
// 
//
//==================================================================================================
class RiuSelectionManager
{
public:
    static RiuSelectionManager* instance();

    // Returns selected items
    // Selection manager owns the selection items and is responsible for delete
    void selectedItems(std::vector<RiuSelectionItem*>& items) const;
    
    // Set vector of items as selected items in SelectionManager
    // SelectionManager takes ownership of the selection items
    void setSelectedItems(const std::vector<RiuSelectionItem*>& items);

    // Append item to selected items in SelectionManager
    // SelectionManager takes ownership of the item
    void appendItemToSelection(RiuSelectionItem* item);

    // Deletes all items in the SelectionManager
    void deleteAllItems();

    bool isEmpty() const;

private:
    std::vector < RiuSelectionItem* > m_selection;
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

    virtual RiuSelectionType type() = 0;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuEclipseSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuEclipseSelectionItem(RimEclipseView* view, size_t gridIndex, size_t cellIndex, cvf::Color3f color);
    virtual ~RiuEclipseSelectionItem() {};

    virtual RiuSelectionType type()
    {
        return ECLIPSE_SELECTION_OBJECT;
    }

public:
    caf::PdmPointer<RimEclipseView> m_view;
    size_t m_gridIndex;
    size_t m_cellIndex;
    cvf::Color3f m_color;
};


