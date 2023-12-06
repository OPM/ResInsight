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

#include "cvfVector3.h"

#include "RimFaultReactivationEnums.h"

#include <limits>
#include <map>

class RigMainGrid;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessor();
    ~RimFaultReactivationDataAccessor();

    virtual void setTimeStep( size_t timeStep );

    virtual bool isMatching( RimFaultReactivation::Property property ) const = 0;

    virtual double valueAtPosition( const cvf::Vec3d& position,
                                    double            topDepth    = std::numeric_limits<double>::infinity(),
                                    double            bottomDepth = std::numeric_limits<double>::infinity() ) const = 0;

protected:
    virtual void updateResultAccessor() = 0;

    size_t m_timeStep;
};
