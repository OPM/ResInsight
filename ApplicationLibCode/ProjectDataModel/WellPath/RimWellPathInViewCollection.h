/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimNestedMirrorCollectionInView.h"
#include "RimWellPathCollection.h"
#include "RimWellPathInView.h"

class RimWellPath;

//==================================================================================================
///
/// Per-view mirror of the global RimWellPathCollection, following the same pattern as
/// RimPolygonInViewCollection. Well paths have no sub-collections in the source model, so
/// sourceSubCollections() is always empty and the mirror stays a flat list.
///
//==================================================================================================
class RimWellPathInViewCollection
    : public RimNestedMirrorCollectionInView<RimWellPathInViewCollection, RimWellPathCollection, RimWellPathInView>
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathInViewCollection();

    void updateFromWellPathCollection();

    std::vector<RimWellPathInView*> visibleWellPathsInView() const;
    std::vector<RimWellPathInView*> allWellPathsInView() const;

    bool isWellPathVisible( const RimWellPath* wellPath ) const;
    bool setWellPathVisible( RimWellPath* wellPath, bool visible );

protected:
    std::vector<RimWellPathCollection*> sourceSubCollections() const override;
    std::vector<RimWellPath*>           sourceItems() const override;
    RimWellPathInView*                  createItemInView( RimWellPath* source ) override;

private:
    RimWellPathInView* findWellPathInView( const RimWellPath* wellPath ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
};
