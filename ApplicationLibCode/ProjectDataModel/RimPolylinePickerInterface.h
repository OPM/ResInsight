/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020  Equinor ASA
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

#include <utility>
#include <vector>

namespace caf
{
class PickEventHandler;
}

class RimPolylineTarget;

class RimPolylinePickerInterface
{
public:
    virtual void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) = 0;
    virtual void deleteTarget( RimPolylineTarget* targetToDelete )                                                = 0;
    virtual void updateEditorsAndVisualization()                                                                  = 0;
    virtual void updateVisualization()                                                                            = 0;
    virtual std::vector<RimPolylineTarget*> activeTargets() const                                                 = 0;
    virtual bool                            pickingEnabled() const                                                = 0;
    virtual caf::PickEventHandler*          pickEventHandler() const                                              = 0;
    virtual double                          scalingFactorForTarget() const;

    std::pair<RimPolylineTarget*, RimPolylineTarget*> findActiveTargetsAroundInsertionPoint( const RimPolylineTarget* targetToInsertBefore );
};
