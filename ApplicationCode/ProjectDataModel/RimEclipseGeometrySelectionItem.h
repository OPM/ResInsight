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

class RimEclipseCase;
class RiuEclipseSelectionItem;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseGeometrySelectionItem : public RimGeometrySelectionItem
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseGeometrySelectionItem();
    virtual ~RimEclipseGeometrySelectionItem() override;

    void            setFromSelectionItem(const RiuEclipseSelectionItem* selectionItem);

    virtual QString geometrySelectionText() const override;

    RimEclipseCase* eclipseCase() const;
    size_t          gridIndex() const;
    size_t          cellIndex() const;

private:
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;

    caf::PdmField<size_t> m_gridIndex;
    caf::PdmField<size_t> m_cellIndex;
    caf::PdmField<cvf::Vec3d> m_localIntersectionPoint;
};

