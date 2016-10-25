/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigFemResultAddress.h"

#include "cvfStructGrid.h"
#include "cvfVector3.h"

class RigGeoMechCaseData;


class RigFemTimeHistoryResultAccessor
{
public:
    RigFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData, 
                                    RigFemResultAddress femResultAddress,
                                    size_t gridIndex, 
                                    size_t cellIndex,
                                    int face, 
                                    const cvf::Vec3d& intersectionPoint);

    QString             topologyText() const;
    std::vector<double> timeHistoryValues() const;

private:
    void                computeTimeHistoryData();

private:
    RigGeoMechCaseData*     m_geoMechCaseData;
    RigFemResultAddress     m_femResultAddress;

    size_t                  m_gridIndex;
    size_t                  m_cellIndex;
    size_t                  m_scalarResultIndex;
    int                     m_face;

    cvf::Vec3d              m_intersectionPoint;

    std::vector<double>     m_timeHistoryValues;
};

