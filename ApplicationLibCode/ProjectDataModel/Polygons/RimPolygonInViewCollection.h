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

#include "RimNestedMirrorCollectionInView.h"
#include "RimPolygonContainer.h"
#include "RimPolygonInView.h"

class RimPolygon;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonInViewCollection : public RimNestedMirrorCollectionInView<RimPolygonInViewCollection, RimPolygonContainer, RimPolygonInView>
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonInViewCollection();

    void updateFromPolygonCollection();

    std::vector<RimPolygonInView*> visiblePolygonsInView() const;
    std::vector<RimPolygonInView*> allPolygonsInView() const;

protected:
    std::vector<RimPolygonContainer*> sourceSubCollections() const override;
    std::vector<RimPolygon*>          sourceItems() const override;
    RimPolygonInView*                 createItemInView( RimPolygon* source ) override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
};
