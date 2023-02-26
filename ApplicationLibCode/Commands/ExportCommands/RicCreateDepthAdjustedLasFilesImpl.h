/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "cvfObject.h"

class RigEclipseWellLogExtractor;
class RigGeoMechWellLogExtractor;
class RigResultAccessor;
class RigWellLogFile;
class RimCase;
class RimEclipseCase;
class RimWellPath;

//==================================================================================================
/// Object to hold Depth value and its original index in a LAS file vector categorized by K-layer.
//==================================================================================================
class LasDepthValueAndIndexPerKLayer
{
public:
    LasDepthValueAndIndexPerKLayer() = default;

    void                     insertIndexAndValue( int kLayer, size_t index, double value );
    std::map<size_t, double> indexAndValuePairs( int kLayer ) const;
    bool                     hasKLayer( int kLayer ) const;

private:
    // Map of K-layer and Index and Value pairs for LAS file depth vectors
    std::map<int, std::map<size_t, double>> m_kLayerIndexAndValuePairsMap;
};

//==================================================================================================
///
//==================================================================================================
namespace RicCreateDepthAdjustedLasFilesImpl
{
struct IndexKDepthData
{
    double mdTop     = 0.0;
    double mdBottom  = 0.0;
    double tvdTop    = 0.0;
    double tvdBottom = 0.0;
};

cvf::ref<RigResultAccessor> createIndexKResultAccessor( RimEclipseCase* selectedCase );

void createDestinationWellsLasFiles( RimCase*                        selectedCase,
                                     RimWellPath*                    sourceWell,
                                     const std::vector<RimWellPath*> destinationWells,
                                     const std::vector<QString>&     selectedResultProperties,
                                     const QString&                  exportFolder,
                                     double                          rkbDiff );

void createDestinationWellLasFile( const QString&                                wellName,
                                   const QString&                                caseDescription,
                                   const std::vector<double>&                    mdValues,
                                   const std::vector<double>&                    tvdMslValues,
                                   const std::vector<double>&                    tvdRkbValues,
                                   const std::map<QString, std::vector<double>>& propertyMap,
                                   const RigWellLogFile*                         sourceWellLogData,
                                   const QString&                                exportFolder );

std::string createDepthUnitText( RiaDefines::DepthUnitType depthUnitType );
std::string createDepthUnitComment( RiaDefines::DepthUnitType depthUnitType );

LasDepthValueAndIndexPerKLayer createLasDepthIndexAndPercValuePerKLayerFromMap( const std::vector<double>&            lasWellDepths,
                                                                                const std::map<int, IndexKDepthData>& indexKDepthDataMap );
std::map<int, IndexKDepthData> createIndexKDepthDataMapFromCase( RimCase* selectedCase, RimWellPath* wellPath );
std::map<int, IndexKDepthData> createIndexKDepthDataMap( cvf::ref<RigEclipseWellLogExtractor> wellExtractor,
                                                         cvf::ref<RigResultAccessor>          indexKResAcc );
std::map<int, IndexKDepthData> createIndexKDepthDataMap( cvf::ref<RigGeoMechWellLogExtractor> wellExtractor );

std::map<int, IndexKDepthData> createIndexKDepthDataMapFromVectors( const std::vector<double>& wellMdValues,
                                                                    const std::vector<double>& wellTvdValues,
                                                                    const std::vector<double>& wellIndexKValues );

std::map<QString, std::vector<double>> createDefaultPropertyMap( const std::vector<QString>& selectedProperties,
                                                                 const RigWellLogFile*       wellLogFile );

}; // namespace RicCreateDepthAdjustedLasFilesImpl
