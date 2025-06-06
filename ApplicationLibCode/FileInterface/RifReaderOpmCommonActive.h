/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RifReaderOpmCommon.h"

class RigActiveCellGrid;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderOpmCommonActive : public RifReaderOpmCommon
{
public:
    RifReaderOpmCommonActive();
    ~RifReaderOpmCommonActive() override;

protected:
    bool importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* caseData ) override;

    void transferActiveGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                                 Opm::EclIO::EGrid&  opmGrid,
                                 RigActiveCellGrid*  activeGrid,
                                 RigGridBase*        localGrid,
                                 RigEclipseCaseData* eclipseCaseData );
};
