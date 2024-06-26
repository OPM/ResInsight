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
#include <vector>

class RigMainGrid;
class RigFaultReactivationModel;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessor();
    ~RimFaultReactivationDataAccessor();

    virtual void setModelAndTimeStep( const RigFaultReactivationModel& model, size_t timeStep );

    virtual bool isMatching( RimFaultReactivation::Property property ) const = 0;

    virtual double valueAtPosition( const cvf::Vec3d&                position,
                                    const RigFaultReactivationModel& model,
                                    RimFaultReactivation::GridPart   gridPart,
                                    double                           topDepth     = std::numeric_limits<double>::infinity(),
                                    double                           bottomDepth  = std::numeric_limits<double>::infinity(),
                                    size_t                           elementIndex = std::numeric_limits<size_t>::max() ) const = 0;

protected:
    virtual void updateResultAccessor() = 0;

    static std::pair<bool, RimFaultReactivation::ElementSets>
        findElementSetForElementIndex( const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets,
                                       int                                                                           elementIndex );

    const RigFaultReactivationModel* m_model;
    size_t                           m_timeStep;
};
