/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RigRefinement.h"

#include "cafVecIjk.h"

#include <QString>

#include <expected>
#include <vector>

class RimEclipseCase;
class RigSimulationInputSettings;
class RifOpmFlowDeckFile;
class RigMainGrid;

//==================================================================================================
///
/// EDITNNC handling when exporting simulation input files (sector models)
///
//==================================================================================================
class RigSimulationInputNncTools
{
public:
    /// Raw NNC connection with global cell indices
    struct NNCConnection
    {
        size_t c1GlobIdx; // Global cell index for cell 1
        size_t c2GlobIdx; // Global cell index for cell 2
        double transmissibility; // Transmissibility value
    };

    /// Transformed NNC connection with sector-relative 0-based IJK coordinates
    struct TransformedNNCConnection
    {
        caf::VecIjk0 cell1            = caf::VecIjk0::ZERO; // 0-based sector IJK for cell 1
        caf::VecIjk0 cell2            = caf::VecIjk0::ZERO; // 0-based sector IJK for cell 2
        double       transmissibility = 0.0; // Transmissibility value
    };

    static std::expected<void, QString>
        exportEditNncKeyword( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::vector<NNCConnection> extractDeckEditNncConnections( RifOpmFlowDeckFile& deckFile, const RigMainGrid& mainGrid );
    static std::vector<NNCConnection> filterInternalSectorConnections( const std::vector<NNCConnection>& allConnections,
                                                                       const RigMainGrid&                mainGrid,
                                                                       const caf::VecIjk0&               min,
                                                                       const caf::VecIjk0&               max );
    static std::expected<TransformedNNCConnection, QString> transformNNCToSectorCoordinates( const NNCConnection& connection,
                                                                                             const RigMainGrid&   mainGrid,
                                                                                             const caf::VecIjk0&  min,
                                                                                             const RigRefinement& refinement );
    static std::vector<TransformedNNCConnection>            refineEditNncConnection( const NNCConnection& connection,
                                                                                     const RigMainGrid&   mainGrid,
                                                                                     const caf::VecIjk0&  min,
                                                                                     const RigRefinement& refinement );
};
