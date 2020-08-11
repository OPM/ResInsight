/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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
#include <memory>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RicMswSubSegmentCellIntersection
{
public:
    RicMswSubSegmentCellIntersection( const QString&     gridName, // Pass in empty string for main grid
                                      size_t             globalCellIndex,
                                      const cvf::Vec3st& gridLocalCellIJK,
                                      const cvf::Vec3d&  lengthsInCell );
    const QString&    gridName() const;
    size_t            globalCellIndex() const;
    cvf::Vec3st       gridLocalCellIJK() const;
    const cvf::Vec3d& lengthsInCell() const;

private:
    QString     m_gridName;
    size_t      m_globalCellIndex;
    cvf::Vec3st m_gridLocalCellIJK;
    cvf::Vec3d  m_lengthsInCell;
};

//==================================================================================================
///
//==================================================================================================
class RicMswSubSegment
{
public:
    RicMswSubSegment( double startMD, double endMD, double startTVD, double endTVD );

    double startMD() const;
    double endMD() const;
    double deltaMD() const;
    double startTVD() const;
    double endTVD() const;
    double deltaTVD() const;

    int  segmentNumber() const;
    void setSegmentNumber( int segmentNumber );
    void addIntersection( std::shared_ptr<RicMswSubSegmentCellIntersection> intersection );

    const std::vector<std::shared_ptr<RicMswSubSegmentCellIntersection>>& intersections() const;
    std::vector<std::shared_ptr<RicMswSubSegmentCellIntersection>>&       intersections();

private:
    double m_startMD;
    double m_endMD;
    double m_startTVD;
    double m_endTVD;
    int    m_segmentNumber;

    std::vector<std::shared_ptr<RicMswSubSegmentCellIntersection>> m_intersections;
};
