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

#include "cafSignal.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf
{
class BoundingBox;
}

//==================================================================================================
///
//==================================================================================================
class RigWellPath : public cvf::Object, public caf::SignalEmitter
{
public:
    caf::Signal<> objectBeingDeleted;

public:
    RigWellPath();
    RigWellPath( const std::vector<cvf::Vec3d>& wellPathPoints, const std::vector<double>& measuredDepths );
    RigWellPath( const RigWellPath& rhs );
    RigWellPath& operator=( const RigWellPath& rhs );
    ~RigWellPath() override;

    const std::vector<cvf::Vec3d>& wellPathPoints() const;
    const std::vector<double>&     measuredDepths() const;
    std::vector<double>            trueVerticalDepths() const;

    void setWellPathPoints( const std::vector<cvf::Vec3d>& wellPathPoints );
    void setMeasuredDepths( const std::vector<double>& measuredDepths );

    void addWellPathPoint( const cvf::Vec3d& wellPathPoint );
    void addMeasuredDepth( double measuredDepth );

    void       setDatumElevation( double value );
    bool       hasDatumElevation() const;
    double     datumElevation() const;
    double     rkbDiff() const;
    cvf::Vec3d interpolatedVectorValuesAlongWellPath( const std::vector<cvf::Vec3d>& vectors,
                                                      double                         measuredDepth,
                                                      double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;
    cvf::Vec3d interpolatedPointAlongWellPath( double  measuredDepth,
                                               double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;

    cvf::Vec3d tangentAlongWellPath( double measuredDepth ) const;

    double wellPathAzimuthAngle( const cvf::Vec3d& position ) const;
    void   twoClosestPoints( const cvf::Vec3d& position, cvf::Vec3d* p1, cvf::Vec3d* p2 ) const;
    double identicalTubeLength( const RigWellPath& otherWellPathGeometry ) const;

    static cvf::ref<RigWellPath> commonGeometry( const std::vector<const RigWellPath*>& allGeometries );
    void                         setUniqueStartAndEndIndex( size_t uniqueStartIndex, size_t uniqueEndIndex );
    size_t                       uniqueStartIndex() const;
    size_t                       uniqueEndIndex() const;
    std::vector<cvf::Vec3d>      uniqueWellPathPoints() const;
    std::vector<double>          uniqueMeasuredDepths() const;

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>>
        clippedPointSubset( double startMD, double endMD, double* horizontalLengthAlongWellToStartClipPoint = nullptr ) const;

    std::vector<cvf::Vec3d> wellPathPointsIncludingInterpolatedIntersectionPoint( double intersectionMeasuredDepth ) const;

    static bool isAnyPointInsideBoundingBox( const std::vector<cvf::Vec3d>& points, const cvf::BoundingBox& boundingBox );

    static std::vector<cvf::Vec3d> clipPolylineStartAboveZ( const std::vector<cvf::Vec3d>& polyLine,
                                                            double                         maxZ,
                                                            double* horizontalLengthAlongWellToClipPoint,
                                                            size_t* indexToFirstVisibleSegment );

private:
    std::vector<cvf::Vec3d> m_wellPathPoints;
    std::vector<double>     m_measuredDepths;

    bool   m_hasDatumElevation;
    double m_datumElevation;
    size_t m_uniqueStartIndex;
    size_t m_uniqueEndIndex;
};
