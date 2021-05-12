/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RigHistogramData.h"

#include "cvfObject.h"

#include <functional>
#include <vector>

class RimEnsembleFractureStatistics;
class RigFractureGrid;
class RigStimPlanFractureDefinition;

//==================================================================================================
///
///
//==================================================================================================
class RigEnsembleFractureStatisticsCalculator
{
public:
    enum class PropertyType
    {
        HEIGHT,
        AREA,
        WIDTH,
        XF,
        KFWF,
        PERMEABILITY
    };

    static RigHistogramData
        createStatisticsData( RimEnsembleFractureStatistics* esf, PropertyType propertyType, int numBins );

private:
    static std::vector<double> calculateGridStatistics( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& defs,
                                                        double( func )( cvf::cref<RigFractureGrid> ) );

    static std::vector<double>
        calculateAreaWeightedStatistics( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& defs,
                                         double( func )( cvf::cref<RigFractureGrid>,
                                                         cvf::cref<RigFractureGrid>,
                                                         RiaDefines::EclipseUnitSystem,
                                                         const QString& ) );

    static double calculateHeight( cvf::cref<RigFractureGrid> fractureGrid );
    static double calculateArea( cvf::cref<RigFractureGrid> fractureGrid );
    static double calculateXf( cvf::cref<RigFractureGrid> fractureGrid );
    static double calculateKfWf( cvf::cref<RigFractureGrid> fractureGrid );

    static double calculateAreaWeightedWidth( cvf::cref<RigFractureGrid>    conductivityGrid,
                                              cvf::cref<RigFractureGrid>    widthGrid,
                                              RiaDefines::EclipseUnitSystem widthUnitSystem,
                                              const QString&                widthUnit );
    static double calculateAreaWeightedPermeability( cvf::cref<RigFractureGrid>    conductivityGrid,
                                                     cvf::cref<RigFractureGrid>    widthGrid,
                                                     RiaDefines::EclipseUnitSystem widthUnitSystem,
                                                     const QString&                widthUnit );

    static double convertUnit( double value, RiaDefines::EclipseUnitSystem unitSystem, const QString& unitName );
};
