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

#include "cvfStructGrid.h"

#include <QString>

#include <list>
#include <set>

class RigCell;
class RimStreamline;
class RigMainGrid;
class RigGridBase;
class RigResultAccessor;
class RigEclipseCaseData;

class RimStreamlineDataAccess
{
public:
    RimStreamlineDataAccess();
    ~RimStreamlineDataAccess();

    bool setupDataAccess( RigMainGrid* grid, RigEclipseCaseData* data, std::list<RiaDefines::PhaseType> phases, int timeIdx );

    double faceValueDividedByArea( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase ) const;
    double combinedFaceValueByArea( RigCell                            cell,
                                    cvf::StructGridInterface::FaceType faceIdx,
                                    std::list<RiaDefines::PhaseType>   phases,
                                    RiaDefines::PhaseType&             dominantPhaseOut ) const;

    const RigMainGrid* grid() const { return m_grid; }

protected:
    cvf::ref<RigResultAccessor>
            getDataAccessor( cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase, int timeIdx );
    QString gridResultNameFromPhase( RiaDefines::PhaseType phase, cvf::StructGridInterface::FaceType faceIdx ) const;

    double posFaceValueDividedByArea( RigCell                            cell,
                                      cvf::StructGridInterface::FaceType faceIdx,
                                      RiaDefines::PhaseType              phase ) const;
    double negFaceValueDividedByArea( RigCell                            cell,
                                      cvf::StructGridInterface::FaceType faceIdx,
                                      RiaDefines::PhaseType              phase ) const;

private:
    RigMainGrid*        m_grid;
    RigEclipseCaseData* m_data;

    std::map<RiaDefines::PhaseType, std::vector<cvf::ref<RigResultAccessor>>> m_dataAccess;
};
