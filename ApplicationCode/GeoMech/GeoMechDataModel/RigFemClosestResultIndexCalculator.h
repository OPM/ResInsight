/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfVector3.h"

class RigFemPart;

class RigFemClosestResultIndexCalculator
{
public:
    RigFemClosestResultIndexCalculator( RigFemPart*         femPart,
                                        RigFemResultPosEnum resultPosition,
                                        int                 elementIndex,
                                        int                 m_face,
                                        const cvf::Vec3d&   intersectionPointInDomain );

    int resultIndexToClosestResult() const;
    int closestNodeId() const;
    int closestElementNodeResIdx() const;

private:
    int m_resultIndexToClosestResult;
    int m_closestNodeId;
    int m_closestElementNodeResIdx;
};
