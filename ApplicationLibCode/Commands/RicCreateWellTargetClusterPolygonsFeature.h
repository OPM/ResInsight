/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "cafCmdFeature.h"

#include "cvfArray.h"
#include "cvfObject.h"

class RimEclipseCase;
class RimEclipseView;
class RigEclipseResultAddress;
class RigActiveCellInfo;

//==================================================================================================
///
//==================================================================================================
class RicCreateWellTargetClusterPolygonsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    static void createWellTargetClusterPolygons( RimEclipseCase* ensembleStatisticsCase, RimEclipseView* view );

protected:
    static std::vector<cvf::Vec3d> findCellCentersWithResultInRange( RimEclipseCase&           eclipseCase,
                                                                     RigEclipseResultAddress&  resultAddress,
                                                                     size_t                    timeStepIndex,
                                                                     double                    rangeMinimum,
                                                                     double                    rangeMaximum,
                                                                     cvf::ref<cvf::UByteArray> visibility );

    static std::vector<cvf::Vec3d> findCellCentersWithResultInRange( RimEclipseCase&                eclipseCase,
                                                                     const RigEclipseResultAddress& resultAddress,
                                                                     size_t                         timeStepIndex,
                                                                     double                         targetValue,
                                                                     double                         rangeMinimum,
                                                                     double                         rangeMaximum,
                                                                     cvf::ref<cvf::UByteArray>      visibility,
                                                                     std::vector<int>&              clusters,
                                                                     int                            clusterId );

    static size_t findStartCell( const RigActiveCellInfo&   activeCellInfo,
                                 const std::vector<double>& values,
                                 double                     targetValue,
                                 double                     minRange,
                                 double                     maxRange,
                                 cvf::ref<cvf::UByteArray>  visibility,
                                 const std::vector<int>&    clusters );

    static std::vector<size_t> findCandidates( const RimEclipseCase&      eclipseCase,
                                               const std::vector<size_t>& previousCells,
                                               const std::vector<double>& volume,
                                               double                     minRange,
                                               double                     maxRange,
                                               cvf::ref<cvf::UByteArray>  visibility,
                                               std::vector<int>&          clusters );

    static void assignClusterIdToCells( const RigActiveCellInfo&   activeCellInfo,
                                        const std::vector<size_t>& cells,
                                        std::vector<int>&          clusters,
                                        int                        clusterId );

    static void growCluster( RimEclipseCase*            eclipseCase,
                             size_t                     startCell,
                             const std::vector<double>& volume,
                             double                     minRange,
                             double                     maxRange,
                             cvf::ref<cvf::UByteArray>  visibility,
                             std::vector<int>&          clusters,
                             int                        clusterId );

    static std::vector<double> smoothHistogram( const std::vector<size_t>& histogram, size_t windowSize );

    static std::vector<std::pair<size_t, double>> findPeaksWithDynamicProminence( const std::vector<double>& smoothedHistogram );

    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
