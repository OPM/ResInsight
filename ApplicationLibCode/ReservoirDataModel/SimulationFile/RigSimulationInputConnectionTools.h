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
#include "cvfVector3.h"

#include <QString>

#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class RimEclipseCase;
class RigSimulationInputSettings;

namespace Opm
{
class DeckRecord;
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
/// Well connection keyword (COMPDAT, COMPSEGS) handling when exporting simulation input files
///
//==================================================================================================
class RigSimulationInputConnectionTools
{
public:
    // Penetration direction of well connections, keyed on (well name, original 0-based IJK)
    struct ConnectionKey
    {
        std::string  wellName;
        caf::VecIjk0 ijk;

        bool operator<( const ConnectionKey& other ) const;
    };
    using ConnectionKeySet       = std::set<ConnectionKey>;
    using ConnectionDirectionMap = std::map<ConnectionKey, RigRefinement::Dimension>;

    static ConnectionDirectionMap connectionDirections( const std::vector<Opm::DeckKeyword>& compdatKeywords );

    // Segment cells (well name, original 0-based IJK) where the well travels towards decreasing index along the
    // connection penetration direction, derived from neighbouring COMPSEGS cells on the same branch ordered by MD
    static ConnectionKeySet reversedSegmentCells( const std::vector<Opm::DeckKeyword>& compsegsKeywords,
                                                  const ConnectionDirectionMap&        directions );

    // Size and permeability of an original grid cell, ordered I/X, J/Y, K/Z
    struct ConnectionCellProperties
    {
        cvf::Vec3d cellSize;
        cvf::Vec3d permeability;
    };
    using ConnectionCellPropertiesProvider = std::function<std::optional<ConnectionCellProperties>( const caf::VecIjk0& originalIjk )>;

    static ConnectionCellPropertiesProvider connectionCellPropertiesProvider( RimEclipseCase* eclipseCase );

    // Refined connections along the penetration direction are split into one record per sub-cell with
    // explicit CF and KH scaled by the sub-cell length fraction. Perpendicular directions use the center sub-cell,
    // and explicit CF is corrected for the smaller Peaceman pressure equivalent radius of the refined cell.
    static std::expected<std::vector<Opm::DeckRecord>, QString>
        processCompdatRecord( const Opm::DeckRecord&                  record,
                              const std::string&                      wellName,
                              const RigSimulationInputSettings&       settings,
                              const ConnectionCellPropertiesProvider& cellProperties = {} );

    // Segment records are split into one record per sub-cell along the connection penetration direction,
    // with the MD interval divided by the sub-cell length fractions in the order the well travels through them.
    // Without an end MD the record is copied unchanged to each sub-cell. Cell ranges (END_IJK) use the center sub-cell.
    static std::expected<std::vector<Opm::DeckRecord>, QString> processCompsegsRecord( const Opm::DeckRecord&            record,
                                                                                       const std::string&                wellName,
                                                                                       bool                              isWellNameRecord,
                                                                                       const RigSimulationInputSettings& settings,
                                                                                       const ConnectionDirectionMap&     directions,
                                                                                       const ConnectionKeySet& reversedCells = {} );
};
