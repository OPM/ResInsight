/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RiaRftDefines.h"

#include "opm/io/eclipse/EclFile.hpp"

#include <set>
#include <string>
#include <tuple>
#include <vector>

class RifRftSegmentData
{
public:
    RifRftSegmentData( int segnxt, int brno, int brnst, int brnen, int segNo );

    int segNext() const;
    int segBrno() const;
    int segBrnst() const;
    int segBrnen() const;
    int segNo() const;

private:
    int m_segNext;
    int m_segbrno;
    int m_brnst;
    int m_brnen;
    int m_segmentNo;
};

class RifRftSegment
{
public:
    void                           setSegmentData( std::vector<RifRftSegmentData> segmentData );
    std::vector<RifRftSegmentData> topology() const;

    void                                       addResultNameAndSize( const Opm::EclIO::EclFile::EclEntry& resultNameAndSize );
    std::vector<Opm::EclIO::EclFile::EclEntry> resultNameAndSize() const;

    std::vector<int>   tubingBranchIds() const;
    std::vector<int>   branchIds() const;
    int                oneBasedBranchIndexForBranchId( int branchId ) const;
    std::map<int, int> branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType branchType ) const;

    const RifRftSegmentData* segmentData( int segmentNumber ) const;
    const RifRftSegmentData* segmentDataByIndex( int segmentIndex ) const;

    void createDeviceBranch( int deviceBranchFirstSegmentNumber, int oneBasedBranchIndex, const std::vector<double>& seglenstValues );

    void setBranchLength( int branchId, double length );
    void setBranchType( int branchId, RiaDefines::RftBranchType branchType );
    void setOneBasedBranchIndex( int branchId, int oneBasedBranchIndex );

    RiaDefines::RftBranchType branchType( int branchId ) const;

    std::vector<size_t> segmentIndicesForBranchNumber( int branchNumber ) const;
    std::vector<size_t> segmentIndicesForBranchIndex( int branchIndex, RiaDefines::RftBranchType branchType ) const;
    std::vector<size_t> packerSegmentIndicesOnAnnulus( int branchIndex ) const;
    std::vector<size_t> nonContinuousDeviceSegmentIndices( int branchIndex ) const;

    std::vector<int> segmentNumbersForBranchIndex( int oneBasedBranchIndex, RiaDefines::RftBranchType branchType ) const;

    std::set<int> uniqueOneBasedBranchIndices( RiaDefines::RftBranchType branchType ) const;

    int segmentIndexFromSegmentNumber( int segmentNumber ) const;

private:
    std::vector<RifRftSegmentData>             m_topology;
    std::vector<Opm::EclIO::EclFile::EclEntry> m_resultNameAndSize;

    std::map<int, double>                    m_branchLength;
    std::map<int, RiaDefines::RftBranchType> m_branchType;
    std::map<int, int>                       m_oneBasedBranchIndexMap;
};
