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
#include <queue>
#include <set>
#include <utility>

class RigCell;
class RimStreamline;
class StreamlineSeedPoint;

class RimStreamlineGenerator : public RimStreamlineGeneratorBase
{
public:
    using CellFaceType = cvf::StructGridInterface::FaceType;

public:
    RimStreamlineGenerator( std::set<size_t>& wellCells );
    ~RimStreamlineGenerator();

    void generateTracer( RigCell cell, double direction, QString simWellName, std::list<RimStreamline*>& outStreamlines ) override;

protected:
    void growStreamline( RimStreamline* streamline, size_t index, CellFaceType faceIdx, double direction );

    bool growStreamlineFromTo( RimStreamline* streamline, cvf::Vec3d startPos, cvf::Vec3d endpos, double rate, RiaDefines::PhaseType dominantPhase );

    std::list<StreamlineSeedPoint> nncCandidates( size_t cellIdx, std::list<RiaDefines::PhaseType> phases, double direction );

    std::priority_queue<StreamlineSeedPoint> m_seeds;
};
