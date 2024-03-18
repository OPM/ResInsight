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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPointer.h"

class RimPolygonInView;
class RimPolygonFile;
class RimPolygon;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonInViewCollection();

    void updateFromPolygonCollection();

    std::vector<RimPolygonInView*> visiblePolygonsInView() const;
    std::vector<RimPolygonInView*> allPolygonsInView() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void            setPolygonFile( RimPolygonFile* polygonFile );
    RimPolygonFile* polygonFile() const;

    void updateAllViewItems();
    void syncCollectionsWithView();
    void syncPolygonsWithView();
    void updateName();

    RimPolygonInViewCollection* getCollectionInViewForPolygonFile( const RimPolygonFile* polygonFile ) const;

private:
    caf::PdmChildArrayField<RimPolygonInView*>           m_polygonsInView;
    caf::PdmChildArrayField<RimPolygonInViewCollection*> m_collectionsInView;

    caf::PdmPointer<RimPolygonFile> m_polygonFile;
};
