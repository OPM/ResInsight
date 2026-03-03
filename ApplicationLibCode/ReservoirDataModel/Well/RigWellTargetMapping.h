/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "RigEclipseResultAddress.h"

#include "cvfVector3.h"

#include <vector>

class RigCaseCellResultsData;
class RimEclipseCase;
class RimRegularGridCase;
class RigFloodingSettings;

//==================================================================================================
///
///
//==================================================================================================
class RigWellTargetMapping
{
public:
    enum class VolumeType
    {
        OIL,
        GAS,
        HYDROCARBON
    };

    enum class VolumeResultType
    {
        MOBILE,
        TOTAL
    };

    enum class VolumesType
    {
        RESERVOIR_VOLUMES,
        SURFACE_VOLUMES_SFIP,
        SURFACE_VOLUMES_FIP,
        RESERVOIR_VOLUMES_COMPUTED
    };

    struct ClusteringLimits
    {
        double                  saturationOil;
        double                  saturationGas;
        double                  permeability;
        double                  pressure;
        double                  transmissibility;
        int                     maxNumTargets;
        int                     maxIterations;
        RigEclipseResultAddress filterAddress;
        std::vector<double>     filter;
    };

    static void generateCandidates( RimEclipseCase*            eclipseCase,
                                    size_t                     timeStepIdx,
                                    VolumeType                 volumeType,
                                    VolumesType                volumesType,
                                    VolumeResultType           volumeResultType,
                                    const RigFloodingSettings& floodingSettings,
                                    const ClusteringLimits&    limits,
                                    bool                       skipUndefinedResults,
                                    bool                       setTimeStepInView );

    static std::vector<double> getVolumeVector( RigCaseCellResultsData&       resultsData,
                                                RiaDefines::EclipseUnitSystem unitsType,
                                                VolumeType                    volumeType,
                                                VolumesType                   volumesType,
                                                VolumeResultType              volumeResultType,
                                                size_t                        timeStepIdx,
                                                const RigFloodingSettings&    floodingSettings );

    static RimRegularGridCase* generateEnsembleCandidates( const std::vector<RimEclipseCase*>& cases,
                                                           size_t                              timeStepIdx,
                                                           const cvf::Vec3st&                  resultGridCellCount,
                                                           VolumeType                          volumeType,
                                                           VolumesType                         volumesType,
                                                           VolumeResultType                    volumeResultType,
                                                           const RigFloodingSettings&          floodingSettings,
                                                           const ClusteringLimits&             limits );

    static QString wellTargetResultName();
};
