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
#include <array>

class RigGeoMechCaseData;


class RiuFemTimeHistoryResultAccessor
{
public:
    RiuFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData, 
                                    RigFemResultAddress femResultAddress,
                                    size_t gridIndex, 
                                    int elementIndex,
                                    int face, 
                                    const cvf::Vec3d& intersectionPoint);

    RiuFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData,
                                    RigFemResultAddress femResultAddress,
                                    size_t gridIndex,
                                    int elementIndex,
                                    int face,
                                    const cvf::Vec3d& intersectionPoint, 
                                    const std::array<cvf::Vec3f, 3>& m_intersectionTriangle);

    QString             geometrySelectionText() const;
    std::vector<double> timeHistoryValues() const;
    int                 closestNodeId() const { return m_closestNodeId; }

private:
    void                computeTimeHistoryData();

private:
    RigGeoMechCaseData*     m_geoMechCaseData;
    RigFemResultAddress     m_femResultAddress;

    size_t                  m_gridIndex;
    int                     m_elementIndex;
    int                     m_face;
    int                     m_closestNodeId;

    cvf::Vec3d              m_intersectionPoint;

    bool                      m_hasIntersectionTriangle;
    std::array<cvf::Vec3f, 3> m_intersectionTriangle;

    std::vector<double>     m_timeHistoryValues;
};

