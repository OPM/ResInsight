/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "cvfVector3.h"

#include <map>
#include <memory>
#include <vector>

class RimEclipseCase;
class RimGeoMechCase;
class RimFaultReactivationDataAccessor;
class RigFaultReactivationModel;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccess
{
public:
    RimFaultReactivationDataAccess( RimEclipseCase* eclipseCase, RimGeoMechCase* geoMechCase, const std::vector<size_t>& timeSteps );
    ~RimFaultReactivationDataAccess();

    void extractModelData( const RigFaultReactivationModel& model );

    void clearModelData();

    std::vector<double>
        propertyValues( RimFaultReactivation::GridPart gridPart, RimFaultReactivation::Property property, size_t outputTimeStep ) const;

private:
    std::shared_ptr<RimFaultReactivationDataAccessor> getAccessor( RimFaultReactivation::Property property ) const;

    std::vector<double> extractModelData( const RigFaultReactivationModel& model,
                                          RimFaultReactivation::GridPart   gridPart,
                                          RimFaultReactivation::Property   property,
                                          size_t                           timeStep );

    std::vector<std::shared_ptr<RimFaultReactivationDataAccessor>> m_accessors;
    std::vector<size_t>                                            m_timeSteps;

    std::map<std::pair<RimFaultReactivation::GridPart, RimFaultReactivation::Property>, std::vector<std::vector<double>>> m_propertyValues;
};
