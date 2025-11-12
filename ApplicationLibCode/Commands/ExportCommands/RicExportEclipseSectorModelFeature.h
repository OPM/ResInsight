/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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
#include "cvfVector3.h"

#include <expected>

class RimEclipseView;
class RimEclipseCase;
class RicExportEclipseSectorModelUi;
class RifOpmFlowDeckFile;
class RigSimWellData;

namespace Opm
{
class DeckRecord;
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
//==================================================================================================
class RicExportEclipseSectorModelFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void openDialogAndExecuteCommand( RimEclipseView* view );
    static void executeCommand( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings, const QString& logPrefix );

    static std::pair<cvf::Vec3st, cvf::Vec3st> getVisibleCellRange( RimEclipseView* view, const cvf::UByteArray& cellVisibility );

    static std::expected<Opm::DeckRecord, QString>
        processEqualsRecord( const Opm::DeckRecord& record, const cvf::Vec3st& min, const cvf::Vec3st& max, const cvf::Vec3st& refinement );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    RimEclipseView*                  selectedView() const;
    static cvf::ref<cvf::UByteArray> createVisibilityBasedOnBoxSelection( RimEclipseView*                      view,
                                                                          const RicExportEclipseSectorModelUi& exportSettings );
    static std::expected<void, QString>
        exportSimulationInput( RimEclipseView& view, RimEclipseCase& eclipseCase, const RicExportEclipseSectorModelUi& exportSettings );
    static void exportGrid( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings );
    static void exportFaults( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings );
    static void exportParameters( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings );

    static std::expected<void, QString>
        addFaultsToDeckFile( RimEclipseCase* eclipseCase, const RicExportEclipseSectorModelUi& exportSettings, RifOpmFlowDeckFile& deckFile );

    static std::expected<void, QString> addBorderBoundaryConditions( RimEclipseCase*                      eclipseCase,
                                                                     const RicExportEclipseSectorModelUi& exportSettings,
                                                                     RifOpmFlowDeckFile&                  deckFile );

    static std::expected<void, QString> replaceKeywordValuesInDeckFile( RimEclipseCase*                      eclipseCase,
                                                                        const RicExportEclipseSectorModelUi& exportSettings,
                                                                        RifOpmFlowDeckFile&                  deckFile );

    static std::expected<void, QString> updateCornerPointGridInDeckFile( RimEclipseCase*                      eclipseCase,
                                                                         const RicExportEclipseSectorModelUi& exportSettings,
                                                                         RifOpmFlowDeckFile&                  deckFile );

    static std::expected<void, QString> filterAndUpdateWellKeywords( RimEclipseCase*                      eclipseCase,
                                                                     const RicExportEclipseSectorModelUi& exportSettings,
                                                                     RifOpmFlowDeckFile&                  deckFile );

    static std::expected<void, QString> addOperNumRegionAndOperater( RimEclipseCase*                      eclipseCase,
                                                                     const RicExportEclipseSectorModelUi& exportSettings,
                                                                     RifOpmFlowDeckFile&                  deckFile,
                                                                     int                                  operNumRegion );

    static std::expected<void, QString> replaceEqualsKeywordIndices( RimEclipseCase*                      eclipseCase,
                                                                     const RicExportEclipseSectorModelUi& exportSettings,
                                                                     RifOpmFlowDeckFile&                  deckFile );

    static std::vector<RigSimWellData*> findIntersectingWells( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max );

    static std::expected<Opm::DeckRecord, QString> processWelspecsRecord( const Opm::DeckRecord&               record,
                                                                          const std::string&                   wellName,
                                                                          const RicExportEclipseSectorModelUi& exportSettings );

    static std::expected<Opm::DeckRecord, QString> processCompdatRecord( const Opm::DeckRecord&               record,
                                                                         const std::string&                   wellName,
                                                                         const RicExportEclipseSectorModelUi& exportSettings );

    static std::expected<Opm::DeckRecord, QString> processCompsegsRecord( const Opm::DeckRecord&               record,
                                                                          const std::string&                   wellName,
                                                                          bool                                 isWellNameRecord,
                                                                          const RicExportEclipseSectorModelUi& exportSettings );
};
