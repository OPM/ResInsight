/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimGeometrySelectionItem.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmPtrField.h"

class RiuGeoMechSelectionItem;
class RimGeoMechCase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechGeometrySelectionItem : public RimGeometrySelectionItem
{
    CAF_PDM_HEADER_INIT;
public:
    RimGeoMechGeometrySelectionItem();
    virtual ~RimGeoMechGeometrySelectionItem() override;

    void            setFromSelectionItem(const RiuGeoMechSelectionItem* selectionItem);

    virtual QString geometrySelectionText() const override;
    RimGeoMechCase* geoMechCase() const;

public:
    caf::PdmField<size_t> m_gridIndex;
    caf::PdmField<size_t> m_cellIndex;
    caf::PdmField<int> m_elementFace;
    caf::PdmField<bool> m_hasIntersectionTriangle;
    
    caf::PdmField<cvf::Vec3d> m_intersectionTriangle_0;
    caf::PdmField<cvf::Vec3d> m_intersectionTriangle_1;
    caf::PdmField<cvf::Vec3d> m_intersectionTriangle_2;

    caf::PdmField<cvf::Vec3d> m_localIntersectionPoint;

private:
    caf::PdmPtrField<RimGeoMechCase*> m_geoMechCase;
};

