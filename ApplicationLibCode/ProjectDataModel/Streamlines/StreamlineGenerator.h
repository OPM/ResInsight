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

#include <list>
#include <set>

#include "RiaDefines.h"

#include <QString>

class RigCell;
class RimStreamline;
class RigMainGrid;
class RigGridBase;
class RigResultAccessor;
class RigEclipseCaseData;
class StreamlineDataAccess;

#include "cvfStructGrid.h"

class StreamlineGenerator
{
public:
    StreamlineGenerator( std::set<size_t>& wellCells );
    ~StreamlineGenerator();

    void setLimits( double flowThreshold, int maxDays, double resolutionInDays );

    void initGenerator( StreamlineDataAccess* dataAccess, std::list<RiaDefines::PhaseType> phases, int density );

    void generateTracer( RigCell cell, double direction, QString simWellName, std::list<RimStreamline*>& outStreamlines );

protected:
    void generateStartPositions( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, std::list<cvf::Vec3d>& positions );

    double m_flowThreshold;
    int    m_maxDays;
    double m_resolution;
    int    m_density;

    std::set<size_t> m_visitedCells;

    std::set<size_t>& m_wellCells;

    StreamlineDataAccess*            m_dataAccess;
    std::list<RiaDefines::PhaseType> m_phases;
};
