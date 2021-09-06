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

#include "cvfVector3.h"

#include <QString>

#include <vector>

class RimGeoMechCase;
class RigGeoMechCaseData;

//==================================================================================================
///
///
//==================================================================================================
class RimWellIADisplacementData
{
public:
    RimWellIADisplacementData( RimGeoMechCase* thecase );
    ~RimWellIADisplacementData();

    cvf::Vec3d getDisplacement( cvf::Vec3d position, int timeStep );

private:
    double displacementValue( RigGeoMechCaseData* caseData, QString componentName, size_t resultIndex, int timeStep );

    RimGeoMechCase* m_case;
};
