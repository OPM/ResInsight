/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimStreamlineGeneratorBase.h"

#include <QString>

#include <list>
#include <set>
#include <utility>

class RigCell;
class RimStreamline;

class RimStreamlineGenerator2 : public RimStreamlineGeneratorBase
{
public:
    RimStreamlineGenerator2( std::set<size_t>& wellCells );
    ~RimStreamlineGenerator2();

    void generateTracer( RigCell cell, double direction, QString simWellName, std::list<RimStreamline*>& outStreamlines ) override;

protected:
    void growStreamline( RimStreamline*                     streamline,
                         size_t                             cellIdx,
                         cvf::StructGridInterface::FaceType faceIdx,
                         double                             direction );

    bool growStreamlineFromTo( RimStreamline*        streamline,
                               cvf::Vec3d            startPos,
                               cvf::Vec3d            endpos,
                               double                flowVelocity,
                               RiaDefines::PhaseType dominantPhase );

    std::list<std::pair<size_t, cvf::StructGridInterface::FaceType>> m_seeds;
};
