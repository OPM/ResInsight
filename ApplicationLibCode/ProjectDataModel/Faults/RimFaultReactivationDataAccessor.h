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

// #include "cvfObject.h"
#include "cvfVector3.h"

#include "RimFaultReactivationEnums.h"

#include <map>
// #include <vector>

// class RimEclipseCase;
// class RigEclipseCaseData;
// class RigResultAccessor;
class RigMainGrid;
// class RigEclipseResultAddress;

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

    virtual double valueAtPosition( const cvf::Vec3d& position ) const = 0;

    virtual bool hasValidDataAtPosition( const cvf::Vec3d& position ) const = 0;

    void useCellIndexAdjustment( const std::map<size_t, size_t>& adjustments );

    static size_t
        findAdjustedCellIndex( const cvf::Vec3d& position, const RigMainGrid* grid, const std::map<size_t, size_t>& cellIndexAdjustmentMap );

protected:
    virtual void updateResultAccessor() = 0;

    std::map<size_t, size_t> m_cellIndexAdjustment;
    size_t                   m_timeStep;
};
