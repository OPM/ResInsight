/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include <QString>

//==================================================================================================
//
// Sumo Explorer Asset
//
//==================================================================================================
struct SumoExplorerAsset
{
    QString assetId;
    QString kind;
    QString name;
};

//==================================================================================================
//
// Sumo Explorer Case
//
//==================================================================================================
struct SumoExplorerCase
{
    QString caseId;
    QString kind;
    QString name;
    QString assetId;
};

//==================================================================================================
//
// Sumo Explorer Ensemble
//
//==================================================================================================
struct SumoExplorerEnsemble
{
    QString ensembleName;
    QString caseId;
};

//==================================================================================================
//
// Sumo Explorer Vector Info
//
//==================================================================================================
struct SumoExplorerVectorInfo
{
    QString name;
};

//==================================================================================================
//
// Sumo Explorer Realization Info
//
//==================================================================================================
struct SumoExplorerRealizationInfo
{
    int realizationId;
};

//==================================================================================================
//
// Sumo Explorer Defines
//
//==================================================================================================
namespace RiaSumoExplorerDefines
{
QString      defaultServerPath();
unsigned int defaultPort();
int          requestTimeoutMillis();
}; // namespace RiaSumoExplorerDefines
