/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RiaPorosityModel.h"

#include <vector>

class RigActiveCellInfo;
class RigMainGrid;
class RimCaseCollection;
class RimEclipseCase;
class RimEclipseStatisticsCase;

class RimReservoirGridEnsembleBase
{
public:
    virtual ~RimReservoirGridEnsembleBase() = default;

    virtual RigMainGrid*                 mainGrid()                                                       = 0;
    virtual RimEclipseCase*              mainCase()                                                       = 0;
    virtual std::vector<RimEclipseCase*> sourceCases() const                                              = 0;
    virtual RigActiveCellInfo*           unionOfActiveCells( RiaDefines::PorosityModelType porosityType ) = 0;
    virtual void                         computeUnionOfActiveCells()                                      = 0;
    virtual RimCaseCollection*           statisticsCaseCollection() const                                 = 0;

    virtual RimEclipseStatisticsCase* createAndAppendStatisticsCase();
};
