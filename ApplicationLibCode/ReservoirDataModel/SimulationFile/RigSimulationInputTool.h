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
#include <functional>
#include <string>
#include <vector>

class RimEclipseView;
class RimEclipseCase;
class RigSimulationInputSettings;
class RifOpmFlowDeckFile;
class RigSimWellData;
template <typename Vec>
class RigBoundingBoxIjk;
class RigGridExportAdapter;
class RigMainGrid;

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
    // Data structures for NNC export

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
        exportSimulationInput( RimEclipseCase& eclipseCase, const RigSimulationInputSettings& settings, cvf::UByteArray* visibility );

    static std::expected<Opm::DeckRecord, QString>
        processEqualsRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processMultiplyRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processBoxRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processCopyRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processAddRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::expected<Opm::DeckRecord, QString>
        processAquconRecord( const Opm::DeckRecord& record, const caf::VecIjk0& min, const caf::VecIjk0& max, const cvf::Vec3st& refinement );

    static std::vector<int> createRefinedVisibility( const RigGridExportAdapter& gridAdapter );

    // Public EDITNNC methods for testing
    static std::vector<NNCConnection> extractDeckEditNncConnections( RifOpmFlowDeckFile& deckFile, const RigMainGrid& mainGrid );
    static std::vector<NNCConnection> filterInternalSectorConnections( const std::vector<NNCConnection>& allConnections,
                                                                       const RigMainGrid&                mainGrid,
                                                                       const caf::VecIjk0&               min,
                                                                       const caf::VecIjk0&               max );
    static std::expected<TransformedNNCConnection, QString> transformNNCToSectorCoordinates( const NNCConnection& connection,
                                                                                             const RigMainGrid&   mainGrid,
                                                                                             const caf::VecIjk0&  min,
                                                                                             const cvf::Vec3st&   refinement );
    static std::vector<TransformedNNCConnection>            refineEditNncConnection( const NNCConnection& connection,
                                                                                     const RigMainGrid&   mainGrid,
                                                                                     const caf::VecIjk0&  min,
                                                                                     const cvf::Vec3st&   refinement );

private:
    static std::expected<void, QString> updateCornerPointGridInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                         const RigSimulationInputSettings& settings,
                                                                         RifOpmFlowDeckFile&               deckFile );

    static std::expected<void, QString> replaceKeywordValuesInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                        const RigSimulationInputSettings& settings,
                                                                        RifOpmFlowDeckFile&               deckFile );

    static std::expected<void, QString>
        updateWelldimsKeyword( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString> addBorderBoundaryConditions( RimEclipseCase*                   eclipseCase,
                                                                     const RigSimulationInputSettings& settings,
                                                                     cvf::ref<cvf::UByteArray>         visibility,
                                                                     RifOpmFlowDeckFile&               deckFile );

    static std::expected<void, QString>
        replaceEqualsKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceMultiplyKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceBoxKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceCopyKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceAddKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        replaceAquconKeywordIndices( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        addFaultsToDeckFile( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        filterAndUpdateWellKeywords( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString>
        exportEditNncKeyword( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings, RifOpmFlowDeckFile& deckFile );

    // Utility: Transform global IJK to sector-relative coordinates with refinement
    static caf::VecIjk0 transformToSectorCoordinates( const caf::VecIjk0& globalIjk, const caf::VecIjk0& min, const cvf::Vec3st& refinement );

    static std::expected<void, QString> addOperNumRegionAndOperater( RifOpmFlowDeckFile&         deckFile,
                                                                     const RigGridExportAdapter& gridAdapter,
                                                                     const std::vector<int>&     operNumResult,
                                                                     int                         operNumRegion,
                                                                     double                      porvMultiplier );

    static std::vector<RigSimWellData*> findIntersectingWells( RimEclipseCase* eclipseCase, const caf::VecIjk0& min, const caf::VecIjk0& max );

    static std::expected<Opm::DeckRecord, QString>
        processWelspecsRecord( const Opm::DeckRecord& record, const std::string& wellName, const RigSimulationInputSettings& settings );

    static std::expected<Opm::DeckRecord, QString>
        processCompdatRecord( const Opm::DeckRecord& record, const std::string& wellName, const RigSimulationInputSettings& settings );

    static std::expected<Opm::DeckRecord, QString> processCompsegsRecord( const Opm::DeckRecord&            record,
                                                                          const std::string&                wellName,
                                                                          bool                              isWellNameRecord,
                                                                          const RigSimulationInputSettings& settings );

    // Generic helper for processing keywords with box indices
    using RecordProcessorFunc =
        std::function<std::expected<Opm::DeckRecord, QString>( const Opm::DeckRecord&, const caf::VecIjk0&, const caf::VecIjk0&, const cvf::Vec3st& )>;

    static std::expected<void, QString> replaceKeywordWithBoxIndices( const std::string&                keywordName,
                                                                      RimEclipseCase*                   eclipseCase,
                                                                      const RigSimulationInputSettings& settings,
                                                                      RifOpmFlowDeckFile&               deckFile,
                                                                      RecordProcessorFunc               processorFunc );

    // Helper function to transform bounding box from global to sector coordinates
    // Returns bounding box with 0-based sector-relative coordinates
    static std::expected<RigBoundingBoxIjk<caf::VecIjk0>, QString>
        transformBoxToSectorCoordinates( const RigBoundingBoxIjk<caf::VecIjk0>& inputBox,
                                         const caf::VecIjk0&                    sectorMin,
                                         const caf::VecIjk0&                    sectorMax,
                                         const cvf::Vec3st&                     refinement,
                                         const QString&                         keywordName,
                                         const QString&                         recordIdentifier );

    // Helper function to extract IJK coordinates from a deck record
    // Returns a VecIjk1 (1-based) constructed from values at the specified item indices
    static caf::VecIjk1 extractIjk( const Opm::DeckRecord& record, size_t indexI, size_t indexJ, size_t indexK );
};
