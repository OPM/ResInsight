/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RigGridCrossPlotCurveCategorization.h"

#include <map>
#include <utility>
#include <vector>

class RigEclipseCaseData;
class RigEclipseResultAddress;

class QString;

struct RigEclipseCrossPlotResult
{
    typedef std::pair<std::vector<double>, std::vector<double>> ResultXYValues;
    
    typedef std::map<int, ResultXYValues> CategorySamplesMap;
    typedef std::map<int, QString>        CategoryNameMap;

    CategorySamplesMap categorySamplesMap;
    CategoryNameMap    categoryNameMap;
};

class RigEclipseCrossPlotDataExtractor
{
public:
    static RigEclipseCrossPlotResult extract(RigEclipseCaseData*                 eclipseCase,
                                             int                                 resultTimeStep,
                                             const RigEclipseResultAddress&      xAddress,
                                             const RigEclipseResultAddress&      yAddress,
                                             RigGridCrossPlotCurveCategorization categorizationType,
                                             const RigEclipseResultAddress&      categoryAddress,
                                             int                                 categoryBinCount);
};
