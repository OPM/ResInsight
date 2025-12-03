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

#include "cafVecIjk.h"
#include "cvfArray.h"
#include "cvfVector3.h"

#include <QString>

#include <expected>
#include <string>
#include <vector>

class RimEclipseView;
class RimEclipseCase;
class RigSimulationInputSettings;
class RifOpmFlowDeckFile;
class RigSimWellData;

namespace Opm
{
class DeckRecord;
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
/// Tool for exporting simulation input files (sector models)
///
//==================================================================================================
class RigSimulationInputTool
{
public:
    static std::expected<void, QString>
        exportSimulationInput( RimEclipseCase& eclipseCase, const RigSimulationInputSettings& settings, cvf::UByteArray* visibility );

    static std::expected<Opm::DeckRecord, QString>
        processEqualsRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processMultiplyRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processBoxRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

private:
    static std::expected<void, QString> updateCornerPointGridInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                         const RigSimulationInputSettings& settings,
                                                                         RifOpmFlowDeckFile&               deckFile );

    static std::expected<void, QString> replaceKeywordValuesInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                        const RigSimulationInputSettings& settings,
                                                                        RifOpmFlowDeckFile&               deckFile );

    static std::expected<void, QString>
        updateWelldimsKeyword( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        addBorderBoundaryConditions( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceEqualsKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceMultiplyKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceBoxKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        addFaultsToDeckFile( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        filterAndUpdateWellKeywords( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString> addOperNumRegionAndOperater( RimEclipseCase*                   eclipseCase,
                                                                     const RigSimulationInputSettings& settings,
                                                                     RifOpmFlowDeckFile&               deckFile,
                                                                     int                               operNumRegion,
                                                                     double                            porvMultiplier );

    static std::vector<RigSimWellData*> findIntersectingWells( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max );

    static std::expected<Opm::DeckRecord, QString>
        processWelspecsRecord( const Opm::DeckRecord& record, const std::string& wellName, const RigSimulationInputSettings& settings );

    static std::expected<Opm::DeckRecord, QString>
        processCompdatRecord( const Opm::DeckRecord& record, const std::string& wellName, const RigSimulationInputSettings& settings );

    static std::expected<Opm::DeckRecord, QString> processCompsegsRecord( const Opm::DeckRecord&            record,
                                                                          const std::string&                wellName,
                                                                          bool                              isWellNameRecord,
                                                                          const RigSimulationInputSettings& settings );
};
