/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <QString>

#include <vector>

class RigCaseCellResultsData;
class RigEclipseCaseData;
class RimEclipseInputPropertyCollection;

//==================================================================================================
/// Helpers for "nested hybrid grids" (see RigNestedHybridGridReconstructor): discovery and import of
/// the sidecar files describing the nesting, orchestration of the LGR reconstruction, and result
/// helpers for the reconstructed grid.
///
/// The result helpers operate on the results stored on a RigCaseCellResultsData and its owner main
/// grid, using the nested-hybrid parent/source mappings populated on the main grid during
/// reconstruction. Kept out of RigCaseCellResultsData to avoid growing that already very large class.
//==================================================================================================
class RigNestedHybridGridResultTools
{
public:
    // Path to the "<grid-basename>_REFINE.grdecl" sidecar next to the grid file, or empty if absent.
    static QString refineSidecarFilePath( const QString& gridFileName );

    // Path to the "<grid-basename>_OLDIJK.grdecl" sidecar next to the grid file, or empty if absent.
    static QString oldIjkSidecarFilePath( const QString& gridFileName );

    static void importRefineSidecarIfPresent( const QString&                     gridFileName,
                                              RimEclipseInputPropertyCollection* inputPropertyCollection,
                                              RigEclipseCaseData*                eclipseCaseData );
    static void importOldIjkSidecarIfPresent( const QString&                     gridFileName,
                                              RimEclipseInputPropertyCollection* inputPropertyCollection,
                                              RigEclipseCaseData*                eclipseCaseData );

    static void reconstructNestedHybridGridIfPresent( const QString& gridFileName, RigEclipseCaseData* eclipseCaseData );

    // Fill LGR cells of already-loaded active-cell-indexed results from their source flat cells
    // (results loaded later are handled during loading).
    static void extendLgrResults( RigCaseCellResultsData* cellResults );

    // How refined-cell values are aggregated onto their parent cell.
    enum class AggregationMode
    {
        VOLUME_WEIGHTED_AVERAGE, // intensive quantities (pressure, saturations)
        SUM // extensive quantities (fluid in place)
    };

    // Aggregate a source result onto each refined cell's parent COARSE cell (volume-weighted average
    // or sum), broadcast back onto every cell of that parent (and unrefined cells keep their own
    // value). Stores the result under "<sourceName>_COARSE" for all time steps and returns its
    // address. Returns an invalid address if there is no nested-hybrid parent mapping.
    static RigEclipseResultAddress computeCoarseAggregate( RigCaseCellResultsData*        cellResults,
                                                           const RigEclipseResultAddress& sourceAddress,
                                                           AggregationMode                mode = AggregationMode::VOLUME_WEIGHTED_AVERAGE );

    // Per refinement level, the aggregate (volume-weighted average or sum) of a source result over
    // the cells of each immediate parent, broadcast back onto that level's cells; all other cells are
    // left undefined so each level's result shows only that level. Stores one result
    // "<sourceName>_COARSE_L<level>" per level present and returns their addresses.
    static std::vector<RigEclipseResultAddress> computePerLevelAggregate( RigCaseCellResultsData*        cellResults,
                                                                          const RigEclipseResultAddress& sourceAddress,
                                                                          AggregationMode mode = AggregationMode::VOLUME_WEIGHTED_AVERAGE );

    // Copy result values onto the reconstructed LGR cells from the source flat refined cells they were
    // built from. Resizes the array to cover the LGR active cells.
    static void assignValuesToLgrs( RigCaseCellResultsData* cellResults, std::vector<double>& values );
};
