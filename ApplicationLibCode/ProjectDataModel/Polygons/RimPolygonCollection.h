/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonContainer.h"

class RimPolygon;
class RimPolygonFile;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonCollection : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCollection();

    // Construct the single top-level polygon collection. Marks the instance as the topmost folder
    // and applies the branded polygon icon. Sub-folders use the default folder icon from the
    // constructor.
    static RimPolygonCollection* createTopmost();

    void        loadData();
    RimPolygon* createUserDefinedPolygon();
    RimPolygon* appendUserDefinedPolygon();
    void        addUserDefinedPolygon( RimPolygon* polygon );
    void        deleteUserDefinedPolygons();
    void        deleteAllPolygons();

    std::vector<RimPolygon*> allPolygons() const;

    // Adds a polygon file as a sub-collection and wires the runtime side-effects
    // (file-changed signal, view-tree refresh, redraw). Use this when introducing a
    // file at runtime; addSubCollection() alone is fine when the side-effects will
    // be triggered another way (e.g., during initAfterRead).
    void addPolygonFile( RimPolygonFile* polygonFile );

    static void appendPolygonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder );

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void updateViewTreeItems();
    void scheduleRedrawViews();

    void connectPolygonSignals( RimPolygon* polygon );
    void connectPolygonFileSignals( RimPolygonFile* polygonFile );
    void onPolygonChanged( const caf::SignalEmitter* emitter );
    void onPolygonFileChanged( const caf::SignalEmitter* emitter );

    void connectSignalsRecursively();
    void connectSignalsForContainer( RimPolygonContainer* container );

private:
    // Legacy field. Polygon files are now stored polymorphically inside m_subCollections
    // (inherited from RimPolygonContainer). Kept declared so old projects load; migrated
    // into m_subCollections in initAfterRead.
    caf::PdmChildArrayField<RimPolygonFile*> m_polygonFiles_OBSOLETE;

protected:
    void initAfterRead() override;
};
