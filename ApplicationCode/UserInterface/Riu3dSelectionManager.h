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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfColor3.h"
#include "cvfStructGrid.h"

#include <array>
#include <cassert>
#include <vector>
// #include "RivWellPathSourceInfo.h"
// #include "RivWellPipeSourceInfo.h"

class RimGridView;
class RimEclipseView;
class RimGeoMechView;
class RimSimWellInView;
class Rim2dIntersectionView;
class RimWellPath;
class RimWellPathComponentInterface;
class RiuSelectionChangedHandler;
class RiuSelectionItem;
class RivSimWellPipeSourceInfo;
class RivWellPathSourceInfo;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;

//==================================================================================================
//
//
//
//==================================================================================================
class Riu3dSelectionManager
{
public:
    enum SelectionRole
    {
        RUI_APPLICATION_GLOBAL, // Selection role intended to manage the cells selected by left mouse click in the 3D view
        RUI_TEMPORARY // Selection role intended to manage the items selected by a right mouse click in the 3D view
    };

public:
    static Riu3dSelectionManager* instance();

    // Returns selected items
    // Selection manager owns the selection items
    void selectedItems( std::vector<RiuSelectionItem*>& items, int role = RUI_APPLICATION_GLOBAL ) const;

    // Returns selected items
    // Selection manager owns the selection items
    RiuSelectionItem* selectedItem( int role = RUI_APPLICATION_GLOBAL ) const;
    //    PdmUiItem*  selectedItem(int role = SelectionManager::APPLICATION_GLOBAL);

    // Append item to selected items
    // SelectionManager takes ownership of the item
    void appendItemToSelection( RiuSelectionItem* item, int role = RUI_APPLICATION_GLOBAL );

    // Set one selected item
    // SelectionManager takes ownership of the item
    void setSelectedItem( RiuSelectionItem* item, int role = RUI_APPLICATION_GLOBAL );

    // Deletes all items in the SelectionManager
    void deleteAllItems( int role = RUI_APPLICATION_GLOBAL );

    bool isEmpty( int role = RUI_APPLICATION_GLOBAL ) const;

private:
    Riu3dSelectionManager();
    ~Riu3dSelectionManager();

    Riu3dSelectionManager( const Riu3dSelectionManager& ) = delete;
    void operator=( const Riu3dSelectionManager& o ) = delete;

    void deleteAllItemsFromSelection( int role );

private:
    std::vector<std::vector<RiuSelectionItem*>> m_selection;

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
        GEOMECH_SELECTION_OBJECT,
        WELLPATH_SELECTION_OBJECT,
        SIMWELL_SELECTION_OBJECT,
        GENERAL_SELECTION_OBJECT,
        INTERSECTION_SELECTION_OBJECT
    };

public:
    RiuSelectionItem() {}
    virtual ~RiuSelectionItem(){};

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
    explicit RiuEclipseSelectionItem( RimGridView*                       view,
                                      RimEclipseResultDefinition*        resultDefinition,
                                      size_t                             timestepIdx,
                                      size_t                             gridIndex,
                                      size_t                             gridLocalCellIndex,
                                      size_t                             nncIndex,
                                      cvf::Color3f                       color,
                                      cvf::StructGridInterface::FaceType face,
                                      const cvf::Vec3d&                  localIntersectionPointInDisplay );

    ~RiuEclipseSelectionItem() override{};

    RiuSelectionType type() const override { return ECLIPSE_SELECTION_OBJECT; }

public:
    caf::PdmPointer<RimGridView>                m_view;
    caf::PdmPointer<RimEclipseResultDefinition> m_resultDefinition;
    size_t                                      m_timestepIdx;
    size_t                                      m_gridIndex;
    size_t                                      m_gridLocalCellIndex;
    size_t                                      m_nncIndex;
    cvf::Color3f                                m_color;
    cvf::StructGridInterface::FaceType          m_face;
    cvf::Vec3d                                  m_localIntersectionPointInDisplay;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuGeoMechSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuGeoMechSelectionItem( RimGridView*                view,
                                      RimGeoMechResultDefinition* resultDefinition,
                                      size_t                      timestepIdx,
                                      size_t                      gridIndex,
                                      size_t                      cellIndex,
                                      cvf::Color3f                color,
                                      int                         elementFace,
                                      const cvf::Vec3d&           localIntersectionPointInDisplay );

    explicit RiuGeoMechSelectionItem( RimGridView*                     view,
                                      RimGeoMechResultDefinition*      resultDefinition,
                                      size_t                           timestepIdx,
                                      size_t                           gridIndex,
                                      size_t                           cellIndex,
                                      cvf::Color3f                     color,
                                      int                              elementFace,
                                      const cvf::Vec3d&                localIntersectionPointInDisplay,
                                      const std::array<cvf::Vec3f, 3>& intersectionTriangle );
    ~RiuGeoMechSelectionItem() override{};

    RiuSelectionType type() const override { return GEOMECH_SELECTION_OBJECT; }

public:
    caf::PdmPointer<RimGridView>                m_view;
    caf::PdmPointer<RimGeoMechResultDefinition> m_resultDefinition;
    size_t                                      m_timestepIdx;
    size_t                                      m_gridIndex;
    size_t                                      m_cellIndex;
    cvf::Color3f                                m_color;
    int                                         m_elementFace;
    bool                                        m_hasIntersectionTriangle;
    std::array<cvf::Vec3f, 3>                   m_intersectionTriangle;
    cvf::Vec3d                                  m_localIntersectionPointInDisplay;
};

//==================================================================================================
//
//
//
//==================================================================================================
class Riu2dIntersectionSelectionItem : public RiuSelectionItem
{
public:
    explicit Riu2dIntersectionSelectionItem( Rim2dIntersectionView* view, RiuSelectionItem* selItem );

    ~Riu2dIntersectionSelectionItem() override;

    RiuSelectionType type() const override { return INTERSECTION_SELECTION_OBJECT; }

public:
    caf::PdmPointer<Rim2dIntersectionView> view() const;
    RiuEclipseSelectionItem*               eclipseSelectionItem() const;
    RiuGeoMechSelectionItem*               geoMechSelectionItem() const;

private:
    caf::PdmPointer<Rim2dIntersectionView> m_view;
    RiuEclipseSelectionItem*               m_eclipseSelItem;
    RiuGeoMechSelectionItem*               m_geoMechSelItem;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuWellPathSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuWellPathSelectionItem( const RivWellPathSourceInfo*   wellPathSourceInfo,
                                       const cvf::Vec3d&              pipeCenterLineIntersectionInDomainCoords,
                                       double                         measuredDepth,
                                       RimWellPathComponentInterface* wellPathComponent = nullptr );

    ~RiuWellPathSelectionItem() override{};

    RiuSelectionType type() const override { return WELLPATH_SELECTION_OBJECT; }

public:
    RimWellPath*                   m_wellpath;
    cvf::Vec3d                     m_pipeCenterlineIntersectionInDomainCoords;
    double                         m_measuredDepth;
    RimWellPathComponentInterface* m_wellPathComponent;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSimWellSelectionItem : public RiuSelectionItem
{
public:
    explicit RiuSimWellSelectionItem( RimSimWellInView* simwell, cvf::Vec3d domainCoord, size_t branchIndex );

    ~RiuSimWellSelectionItem() override{};

    RiuSelectionType type() const override { return SIMWELL_SELECTION_OBJECT; }

public:
    RimSimWellInView* m_simWell;
    cvf::Vec3d        m_domainCoord;
    size_t            m_branchIndex;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuGeneralSelectionItem : public RiuSelectionItem
{
public:
    RiuGeneralSelectionItem( caf::PdmObject* object );

    ~RiuGeneralSelectionItem() override{};

    RiuSelectionType type() const override { return GENERAL_SELECTION_OBJECT; }

public:
    caf::PdmObject* m_object;
};
