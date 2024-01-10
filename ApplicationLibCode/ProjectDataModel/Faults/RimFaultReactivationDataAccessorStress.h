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

#include <vector>

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RigGriddedPart3d;
class RimModeledWellPath;
class RigWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorStress : public RimFaultReactivationDataAccessor
{
public:
    enum class StressType
    {
        S11,
        S22,
        S33
    };

    RimFaultReactivationDataAccessorStress( RimFaultReactivation::Property property, double gradient, double seabedDepth );
    virtual ~RimFaultReactivationDataAccessorStress();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d&                position,
                            const RigFaultReactivationModel& model,
                            RimFaultReactivation::GridPart   gridPart,
                            double                           topDepth     = std::numeric_limits<double>::infinity(),
                            double                           bottomDepth  = std::numeric_limits<double>::infinity(),
                            size_t                           elementIndex = std::numeric_limits<size_t>::max() ) const override;

protected:
    virtual bool isDataAvailable() const = 0;

    virtual double extractStressValue( StressType stressType, const cvf::Vec3d& position ) const = 0;

    virtual std::pair<double, cvf::Vec3d>
        calculatePorBar( const cvf::Vec3d& position, double gradient, RimFaultReactivation::GridPart gridPart ) const = 0;

    virtual bool isPositionValid( const cvf::Vec3d& position, const cvf::Vec3d& topPosition, const cvf::Vec3d& bottomPosition ) const = 0;

    RimFaultReactivation::Property m_property;
    double                         m_gradient;
    double                         m_seabedDepth;
};
