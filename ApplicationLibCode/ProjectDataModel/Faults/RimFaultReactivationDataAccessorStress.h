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

#include <vector>

class RigFemPartResultsCollection;
class RimGeoMechCase;
class RigGeoMechCaseData;
class RigFemScalarResultFrames;
class RigFemPart;
class RimWellIADataAccess;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorStress : public RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessorStress( RimGeoMechCase* geoMechCase, RimFaultReactivation::Property property );
    ~RimFaultReactivationDataAccessorStress();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d&                position,
                            const RigFaultReactivationModel& model,
                            RimFaultReactivation::GridPart   gridPart,
                            double                           topDepth    = std::numeric_limits<double>::infinity(),
                            double                           bottomDepth = std::numeric_limits<double>::infinity() ) const override;

private:
    void updateResultAccessor() override;

    static RigFemResultAddress getResultAddress( const std::string& fieldName, const std::string& componentName );

    double interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                    const RigFemPart*         femPart,
                                    const cvf::Vec3d&         position,
                                    const std::vector<float>& scalarResults ) const;

    RimGeoMechCase*                m_geoMechCase;
    RimFaultReactivation::Property m_property;
    RigGeoMechCaseData*            m_geoMechCaseData;
    RigFemScalarResultFrames*      m_s11Frames;
    RigFemScalarResultFrames*      m_s22Frames;
    RigFemScalarResultFrames*      m_s33Frames;
    RigFemScalarResultFrames*      m_porFrames;
    const RigFemPart*              m_femPart;
};
