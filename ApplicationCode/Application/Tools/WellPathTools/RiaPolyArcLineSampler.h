/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include <vector>

class RiaPolyArcLineSampler
{
public:
    RiaPolyArcLineSampler( const cvf::Vec3d& startTangent, const std::vector<cvf::Vec3d>& lineArcEndPoints );

    void sampledPointsAndMDs( double                   maxSampleInterval,
                              bool                     isResamplingLines,
                              std::vector<cvf::Vec3d>* points,
                              std::vector<double>*     mds );

    static std::vector<cvf::Vec3d> pointsWithoutDuplicates( const std::vector<cvf::Vec3d>& points );

private:
    void sampleLine( cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent );
    void sampleArc( cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent );
    void sampleSegment( cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent );

    std::vector<cvf::Vec3d>* m_points; // Internal temporary pointers to collections being filled.
    std::vector<double>*     m_meshDs;

    double       m_maxSamplingsInterval;
    const double m_maxSamplingArcAngle = 0.07310818; // Angle from 6 deg dogleg on 10 m
    bool         m_isResamplingLines;
    double       m_totalMD;

    cvf::Vec3d              m_startTangent;
    std::vector<cvf::Vec3d> m_lineArcEndPoints;
};
