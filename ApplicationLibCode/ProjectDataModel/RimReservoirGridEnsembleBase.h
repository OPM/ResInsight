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

#include <QString>

#include <set>
#include <vector>

class RigActiveCellInfo;
class RigMainGrid;
class RimCaseCollection;
class RimEclipseCase;
class RimEclipseStatisticsCase;
class RimFormationNames;
class RimStatisticsContourMap;

class RimReservoirGridEnsembleBase
{
public:
    enum class GridModeType
    {
        SHARED_GRID,
        INDIVIDUAL_GRIDS
    };

    virtual ~RimReservoirGridEnsembleBase() = default;

    virtual QString                      ensembleName() const = 0;
    virtual GridModeType                 gridMode() const     = 0;
    virtual std::vector<RimEclipseCase*> sourceCases() const  = 0;
    virtual RimEclipseCase*              mainCase()           = 0;

    virtual RigMainGrid*              mainGrid();
    virtual RigActiveCellInfo*        unionOfActiveCells( RiaDefines::PorosityModelType porosityType );
    virtual void                      computeUnionOfActiveCells();
    virtual std::set<RimEclipseCase*> casesInViews() const;
    virtual RimCaseCollection*        statisticsCaseCollection() const;
    virtual RimFormationNames*        activeFormationNames() const;
    virtual void                      addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap ) {}
    virtual RimEclipseStatisticsCase* createAndAppendStatisticsCase();
};
