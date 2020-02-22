/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfMath.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf
{
class BoundingBox;
}

#include <vector>

//==================================================================================================
///
//==================================================================================================
class RigWellPath : public cvf::Object
{
public:
    std::vector<cvf::Vec3d> m_wellPathPoints;
    std::vector<double>     m_measuredDepths;

    const std::vector<cvf::Vec3d>& wellPathPoints() const;
    const std::vector<double>&     measureDepths() const;
    std::vector<double>            trueVerticalDepths() const;

    RigWellPath();
    void       setDatumElevation( double value );
    bool       hasDatumElevation() const;
    double     datumElevation() const;
    double     rkbDiff() const;
    cvf::Vec3d interpolatedVectorValuesAlongWellPath( const std::vector<cvf::Vec3d>& vectors,
                                                      double                         measuredDepth,
                                                      double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;
    cvf::Vec3d interpolatedPointAlongWellPath( double  measuredDepth,
                                               double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;

    double wellPathAzimuthAngle( const cvf::Vec3d& position ) const;
    void   twoClosestPoints( const cvf::Vec3d& position, cvf::Vec3d* p1, cvf::Vec3d* p2 ) const;

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>>
        clippedPointSubset( double startMD, double endMD, double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;

    std::vector<cvf::Vec3d> wellPathPointsIncludingInterpolatedIntersectionPoint( double intersectionMeasuredDepth ) const;

    static bool isAnyPointInsideBoundingBox( const std::vector<cvf::Vec3d>& points, const cvf::BoundingBox& boundingBox );

    static std::vector<cvf::Vec3d> clipPolylineStartAboveZ( const std::vector<cvf::Vec3d>& polyLine,
                                                            double                         maxZ,
                                                            double* horizontalLengthAlongWellToClipPoint,
                                                            size_t* indexToFirstVisibleSegment );

private:
    bool   m_hasDatumElevation;
    double m_datumElevation;
};
