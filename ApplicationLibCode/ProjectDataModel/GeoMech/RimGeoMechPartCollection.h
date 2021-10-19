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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfVector3.h"

#include <vector>

class RimGeoMechPart;
class RimGeoMechCase;

class RimGeoMechPartCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechPartCollection();
    ~RimGeoMechPartCollection() override;

    void syncWithCase( RimGeoMechCase* geoCase );

    bool shouldRebuildPartVisualization( int currentTimeStep, bool showDisplacement, double scaleFactor );
    bool shouldReloadDisplacements( int currentTimeStep, bool showDisplacement, double scaleFactor );
    bool shouldBeVisibleInTree() const;

    bool isPartEnabled( int partId ) const;

    const std::vector<cvf::Vec3f>& displacements( int partId ) const;

    void   setCurrentDisplacementSettings( int currentTimeStep, bool showDisplacement, double scaleFactor );
    bool   isDisplacementsUsed() const;
    int    currentDisplacementTimeStep() const;
    double currentDisplacementScaleFactor() const;

    std::vector<RimGeoMechPart*> parts() const;

private:
    RimGeoMechPart* part( int partId ) const;

    caf::PdmChildArrayField<RimGeoMechPart*> m_parts;
    RimGeoMechCase*                          m_case;

    int    m_currentDisplacementTimeStep;
    double m_currentScaleFactor;
    bool   m_displacementsUsed;
};
