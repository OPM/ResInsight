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

#include "RiaDefines.h"

#include "cvfVector3.h"

#include "cafVecIjk.h"

#include <QDateTime>

#include <optional>
#include <vector>

//==================================================================================================
/// Stores the info on a significant point in the well. Either a well-to-grid connection, or the
/// bottom position of a connection less well-segment
//==================================================================================================
struct RigWellResultPoint
{
    RigWellResultPoint();

    void setGridIndex( size_t gridIndex );
    void setGridCellIndex( size_t cellIndex );
    void setIsOpen( bool isOpen );
    void setFlowData( double flowRate, double oilRate, double gasRate, double waterRate );
    void setConnectionFactor( double connectionFactor );

    void setSegmentData( int branchId, int segmentId );
    void setOutletSegmentData( int outletBranchId, int outletSegmentId );

    void setBottomPosition( const cvf::Vec3d& bottomPosition );
    void setIsConnectedToValve( bool enable );

    bool isPointValid() const;
    bool isCell() const;
    bool isValid() const;
    bool isOpen() const;
    bool isEqual( const RigWellResultPoint& other ) const;
    bool isConnectedToValve() const;

    double flowRate() const;
    double oilRate() const;
    double gasRate() const;
    double waterRate() const;
    double connectionFactor() const;
    void   clearAllFlow();

    size_t gridIndex() const;
    size_t cellIndex() const;

    int branchId() const;
    int segmentId() const;
    int outletBranchId() const;
    int outletSegmentId() const;

    cvf::Vec3d bottomPosition() const;

    std::optional<caf::VecIjk1> cellIjk() const;
    void                        setIjk( caf::VecIjk1 cellIJK );

private:
    size_t m_gridIndex;
    size_t m_cellIndex; //< Index to cell which is included in the well
    bool   m_isOpen; //< Marks the well as open or closed as of Eclipse simulation

    int m_ertBranchId;
    int m_ertSegmentId;
    int m_ertOutletBranchId;
    int m_ertOutletSegmentId;

    cvf::Vec3d m_bottomPosition; //< The estimated bottom position of the well segment, when we have no grid cell
                                 // connections for the segment.
    double m_flowRate; //< Total reservoir rate
    double m_oilRate; //< Surface oil rate
    double m_gasRate; //< Surface gas rate For Field-unit, converted to [stb/day] to align with oil and water.
    double m_waterRate; //< Surface water rate

    double m_connectionFactor;
    bool   m_isConnectedToValve;

    std::optional<caf::VecIjk1> m_cellIjk;
};
