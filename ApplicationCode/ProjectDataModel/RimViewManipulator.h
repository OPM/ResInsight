/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include <vector>
#include "cvfVector3.h"

namespace cvf
{
class BoundingBox;
}

class RimGridView;
class Rim3dView;

class RimViewManipulator
{
public:
    static void applySourceViewCameraOnDestinationViews( RimGridView*               sourceView,
                                                         std::vector<RimGridView*>& destinationViews );
    static cvf::Vec3d calculateEquivalentCamPosOffset(Rim3dView* sourceView, Rim3dView* destView);

private:
    static bool isBoundingBoxesOverlappingOrClose( const cvf::BoundingBox& sourceBB, const cvf::BoundingBox& destBB );
};