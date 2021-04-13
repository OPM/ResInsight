/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RicMswItem.h"
#include "RicMswSegment.h"

#include "cafPdmPointer.h"

#include <memory>
#include <vector>

class RimWellPath;

class RicMswCompletion;
class RicMswSegment;

class RicMswBranch : public RicMswItem
{
public:
    RicMswBranch( const QString& label, const RimWellPath* wellPath, double initialMD = 0.0, double initialTVD = 0.0 );
    virtual ~RicMswBranch() = default;

    void               addSegment( std::unique_ptr<RicMswSegment> segment );
    void               insertAfterSegment( const RicMswSegment* insertAfter, std::unique_ptr<RicMswSegment> segment );
    void               sortSegments();
    const RimWellPath* wellPath() const;

    double startMD() const override;
    double startTVD() const override;

    double endMD() const override;
    double endTVD() const override;

    int  branchNumber() const;
    void setBranchNumber( int branchNumber );

    std::vector<const RicMswSegment*> segments() const;
    std::vector<RicMswSegment*>       segments();

    RicMswSegment* findClosestSegmentByMidpoint( double measuredDepth );

    size_t segmentCount() const;

    std::vector<const RicMswBranch*> branches() const;
    std::vector<RicMswBranch*>       branches();

    void addChildBranch( std::unique_ptr<RicMswBranch> branch );

private:
    double m_initialMD;
    double m_initialTVD;

    int m_branchNumber;

    std::vector<std::unique_ptr<RicMswSegment>> m_segments;
    std::vector<std::unique_ptr<RicMswBranch>>  m_branches;

    const RimWellPath* m_wellPath;
};
