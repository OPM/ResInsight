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

#include "RigFemResultPosEnum.h"

#include "cvfVector3.h"

#include <QString>

#include <vector>

class RimGeoMechCase;
class RigGeoMechCaseData;

//==================================================================================================
///
///
//==================================================================================================
class RimWellIADataAccess
{
public:
    RimWellIADataAccess( RimGeoMechCase* thecase );
    ~RimWellIADataAccess();

    int    resultIndex( RigFemResultPosEnum resultType, cvf::Vec3d position );
    int    elementIndex( cvf::Vec3d position );
    double resultValue( QString             fieldName,
                        QString             componentName,
                        RigFemResultPosEnum resultType,
                        size_t              resultIndex,
                        int                 timeStep );
    double interpolatedResultValue( QString             fieldname,
                                    QString             componentName,
                                    RigFemResultPosEnum resultType,
                                    cvf::Vec3d          position,
                                    int                 timeStep );

private:
    RimGeoMechCase*     m_case;
    RigGeoMechCaseData* m_caseData;
};
