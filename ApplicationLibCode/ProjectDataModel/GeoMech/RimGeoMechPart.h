/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfVector3.h"

#include <vector>

class RimGeoMechPart : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechPart();
    ~RimGeoMechPart() override;

    void setPartId( int partId );
    int  partId() const;

    void                           setDisplacements( const std::vector<cvf::Vec3f>& displacements );
    const std::vector<cvf::Vec3f>& displacements() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<int> m_partId;

    std::vector<cvf::Vec3f> m_displacements;
};
