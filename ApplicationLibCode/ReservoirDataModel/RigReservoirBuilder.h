/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

class RigGridBase;
class RigCell;
class RigEclipseCaseData;

class LocalGridRefinement
{
public:
    LocalGridRefinement( const cvf::Vec3st& mainGridMin, const cvf::Vec3st& mainGridMax, const cvf::Vec3st& singleCellRefinementFactors )
    {
        m_mainGridMinCellPosition     = mainGridMin;
        m_mainGridMaxCellPosition     = mainGridMax;
        m_singleCellRefinementFactors = singleCellRefinementFactors;
    }

    cvf::Vec3st m_mainGridMinCellPosition;
    cvf::Vec3st m_mainGridMaxCellPosition;
    cvf::Vec3st m_singleCellRefinementFactors;
};

class RigReservoirBuilder
{
public:
    RigReservoirBuilder();

    void setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate );

    void        setIJKCount( const cvf::Vec3st& ijkCount );
    cvf::Vec3st ijkCount() const;

    void addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                 const cvf::Vec3st& maxCellPosition,
                                 const cvf::Vec3st& singleCellRefinementFactors );

    void createGridsAndCells( RigEclipseCaseData* eclipseCase );

private:
    static void appendCells( size_t nodeStartIndex, size_t cellCount, RigGridBase* hostGrid, std::vector<RigCell>& cells );
    static void appendNodes( const cvf::Vec3d& min, const cvf::Vec3d& max, const cvf::Vec3st& cubeDimension, std::vector<cvf::Vec3d>& nodes );
    static void appendCubeNodes( const cvf::Vec3d& min, const cvf::Vec3d& max, std::vector<cvf::Vec3d>& nodes );

    size_t cellIndexFromIJK( size_t i, size_t j, size_t k ) const;

private:
    cvf::Vec3d  m_minWorldCoordinate;
    cvf::Vec3d  m_maxWorldCoordinate;
    cvf::Vec3st m_ijkCount;

    std::vector<LocalGridRefinement> m_localGridRefinements;
};
