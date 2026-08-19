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
class RimEclipseViewCollection;
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

    virtual void addCase( RimEclipseCase* reservoir )    = 0;
    virtual void removeCase( RimEclipseCase* reservoir ) = 0;

    // Null for ensembles that do not own a view collection.
    virtual RimEclipseViewCollection* viewCollection() const { return nullptr; }

    // Ensembles can opt out of the mobile volume weighted mean, as the data it is derived from can be
    // expensive to fetch. Used by Sumo grid ensembles, see Rim3dOverlayInfoConfig.
    virtual bool doComputeMobileVolumeWeightedMean() const { return true; }
    virtual void setDoComputeMobileVolumeWeightedMean( bool enable ) {}

    // Offer 'candidate' as the shared grid of this ensemble. Returns the grid the caller should use: the
    // shared grid already in memory, when this ensemble shares grids and the dimensions match, otherwise
    // 'candidate', which then becomes the shared grid.
    //
    // Never triggers grid loading, so it is safe to call while opening a case. Calling mainGrid() instead
    // would re-enter the loading of the first realization and publish an empty grid.
    virtual RigMainGrid* shareOrAdoptMainGrid( RigMainGrid* candidate ) { return candidate; }

    virtual RigMainGrid*                          mainGrid();
    virtual RigActiveCellInfo*                    unionOfActiveCells( RiaDefines::PorosityModelType porosityType );
    virtual void                                  computeUnionOfActiveCells();
    virtual std::set<RimEclipseCase*>             casesInViews() const;
    virtual RimCaseCollection*                    statisticsCaseCollection() const;
    virtual RimFormationNames*                    activeFormationNames() const;
    virtual void                                  addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap ) {}
    virtual std::vector<RimStatisticsContourMap*> statisticsContourMaps() const { return {}; }
    virtual RimEclipseStatisticsCase*             createAndAppendStatisticsCase();
};
