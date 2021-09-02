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
class RimWellIAStressData
{
public:
    RimWellIAStressData( RimGeoMechCase* thecase );
    ~RimWellIAStressData();

    bool extractData( cvf::Vec3d position );

    double sxx() const;
    double syy() const;
    double szz() const;
    double sxy() const;
    double sxz() const;
    double syz() const;
    double pp() const;

private:
    double stressValue( RigGeoMechCaseData* caseData, QString componentName, size_t resultIndex );
    double ppValue( RigGeoMechCaseData* caseData, size_t cellIdx, cvf::Vec3d position );

    RimGeoMechCase*     m_case;
    std::vector<double> m_stressValues;
    double              m_pp;
};
