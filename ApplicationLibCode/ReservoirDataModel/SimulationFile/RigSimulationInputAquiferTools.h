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

class RimEclipseCase;
class RigSimulationInputSettings;
class RifOpmFlowDeckFile;

namespace Opm
{
class DeckRecord;
} // namespace Opm

//==================================================================================================
///
/// Aquifer keyword (AQUCON, AQUANCON, AQUNUM) handling when exporting simulation input files
///
//==================================================================================================
class RigSimulationInputAquiferTools
{
public:
    static std::expected<void, QString>
        replaceAquconKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceAquanconKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceAqunumKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<Opm::DeckRecord, QString>
        processAquconRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const RigRefinement& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processAquanconRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const RigRefinement& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processAqunumRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const RigRefinement& refinement );
};
