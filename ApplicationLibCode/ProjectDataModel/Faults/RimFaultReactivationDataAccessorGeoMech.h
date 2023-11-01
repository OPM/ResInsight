/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -    Equinor ASA
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

#include "RimFaultReactivationDataAccessor.h"
#include "RimFaultReactivationEnums.h"

#include "RigFemResultAddress.h"

class RigFemPartResultsCollection;
class RimGeoMechCase;
class RigGeoMechCaseData;
class RigFemScalarResultFrames;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorGeoMech : public RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessorGeoMech( RimGeoMechCase* geoMechCase, RimFaultReactivation::Property property );
    ~RimFaultReactivationDataAccessorGeoMech();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d& position ) const override;

    bool hasValidDataAtPosition( const cvf::Vec3d& position ) const override;

private:
    void updateResultAccessor() override;

    static RigFemResultAddress getResultAddress( RimFaultReactivation::Property property );

    RimGeoMechCase*                m_geoMechCase;
    RimFaultReactivation::Property m_property;
    RigGeoMechCaseData*            m_geoMechCaseData;
    RigFemScalarResultFrames*      m_resultFrames;
};
