/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStimPlanModelPropertyCalculator.h"

#include "RiaStimPlanModelDefines.h"

#include <vector>

class RimStimPlanModelCalculator;
class RimStimPlanModel;
class RimColorLegend;

class QString;

class RimStimPlanModelElasticPropertyCalculator : public RimStimPlanModelPropertyCalculator
{
public:
    RimStimPlanModelElasticPropertyCalculator( RimStimPlanModelCalculator* calculator );

    bool calculate( RiaDefines::CurveProperty curveProperty,
                    const RimStimPlanModel*   stimPlanModel,
                    int                       timeStep,
                    std::vector<double>&      values,
                    std::vector<double>&      measuredDepthValues,
                    std::vector<double>&      tvDepthValues,
                    double&                   rkbDiff ) const override;

    bool isMatching( RiaDefines::CurveProperty curveProperty ) const override;

    static QString findFaciesName( const RimColorLegend& colorLegend, double value );

protected:
    static void addOverburden( std::vector<QString>& formationNames,
                               std::vector<double>&  formationValues,
                               std::vector<double>&  tvDepthValues,
                               std::vector<double>&  measuredDepthValues,
                               double                overburdenHeight,
                               const QString&        formationName );

    static void addUnderburden( std::vector<QString>& formationNames,
                                std::vector<double>&  formationValues,
                                std::vector<double>&  tvDepthValues,
                                std::vector<double>&  measuredDepthValues,
                                double                underburdenHeight,
                                const QString&        formationName );

private:
    RimStimPlanModelCalculator* m_stimPlanModelCalculator;
};
