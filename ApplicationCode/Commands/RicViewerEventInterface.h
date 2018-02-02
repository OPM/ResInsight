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

#include "cafCmdFeature.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf { 
    class Part; 
} 


//==================================================================================================
/// 
//==================================================================================================
class RicViewerEventObject
{
public:
    RicViewerEventObject(cvf::Vec3d globalIntersectionPoint, const std::vector<std::pair<const cvf::Part*, cvf::uint>>& partAndTriangleIndexPairs)
        : m_globalIntersectionPoint(globalIntersectionPoint),
        m_partAndTriangleIndexPairs(partAndTriangleIndexPairs)
    {
    }

    cvf::Vec3d  m_globalIntersectionPoint;
    std::vector<std::pair<const cvf::Part*, cvf::uint>> m_partAndTriangleIndexPairs;
};


//==================================================================================================
/// 
//==================================================================================================
class RicViewerEventInterface
{
public:
    virtual bool handleEvent(const RicViewerEventObject& eventObject) = 0;
};

