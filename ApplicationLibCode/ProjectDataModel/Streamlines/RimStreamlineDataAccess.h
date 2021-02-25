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

#include "cvfStructGrid.h"

class RimStreamlineDataAccess
{
public:
    RimStreamlineDataAccess();
    ~RimStreamlineDataAccess();

    bool setupDataAccess( RigMainGrid* grid, RigEclipseCaseData* data, std::list<RiaDefines::PhaseType> phases, int timeIdx );

    double faceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase ) const;
    double combinedFaceValue( RigCell                            cell,
                              cvf::StructGridInterface::FaceType faceIdx,
                              std::list<RiaDefines::PhaseType>   phases,
                              RiaDefines::PhaseType&             dominantPhaseOut ) const;

    const RigMainGrid* grid() const { return m_grid; }

    RigCell getNeighborWithPoint( RigCell curCell, cvf::Vec3d point );

protected:
    cvf::ref<RigResultAccessor>
            getDataAccessor( cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase, int timeIdx );
    QString gridResultNameFromPhase( RiaDefines::PhaseType phase, cvf::StructGridInterface::FaceType faceIdx ) const;

    double posFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase ) const;
    double negFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase ) const;

private:
    RigMainGrid*        m_grid;
    RigEclipseCaseData* m_data;

    std::map<RiaDefines::PhaseType, std::vector<cvf::ref<RigResultAccessor>>> m_dataAccess;
};
