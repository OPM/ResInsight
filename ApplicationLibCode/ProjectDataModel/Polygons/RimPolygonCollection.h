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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimPolygon;
class RimPolygonFile;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCollection();

    void        loadData();
    RimPolygon* createUserDefinedPolygon();
    RimPolygon* appendUserDefinedPolygon();
    void        addUserDefinedPolygon( RimPolygon* polygon );
    void        deleteUserDefinedPolygons();

    void addPolygonFile( RimPolygonFile* polygonFile );

    std::vector<RimPolygon*>     userDefinedPolygons() const;
    std::vector<RimPolygonFile*> polygonFiles() const;
    std::vector<RimPolygon*>     allPolygons() const;

    static void appendPolygonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder );

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void updateViewTreeItems();
    void scheduleRedrawViews();

    void connectPolygonSignals( RimPolygon* polygon );
    void connectPolygonFileSignals( RimPolygonFile* polygonFile );
    void onPolygonChanged( const caf::SignalEmitter* emitter );
    void onPolygonFileChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmChildArrayField<RimPolygon*>     m_polygons;
    caf::PdmChildArrayField<RimPolygonFile*> m_polygonFiles;

protected:
    void initAfterRead() override;
};
