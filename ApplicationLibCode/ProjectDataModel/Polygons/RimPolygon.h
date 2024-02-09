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

#include "RimNamedObject.h"

#include "RimPolylinesDataInterface.h"

#include "cafPdmFieldCvfVec3d.h"

#include "cvfVector3.h"

class RimPolygonAppearance;

class RimPolygon : public RimNamedObject, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimPolygon();

    void                    setPointsInDomainCoords( const std::vector<cvf::Vec3d>& points );
    std::vector<cvf::Vec3d> pointsInDomainCoords() const;
    bool                    isClosed() const;

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

private:
    caf::PdmField<bool>                       m_isReadOnly;
    caf::PdmField<std::vector<cvf::Vec3d>>    m_pointsInDomainCoords;
    caf::PdmChildField<RimPolygonAppearance*> m_appearance;
};
