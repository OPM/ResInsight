/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimSummaryAddressCollection.h"

#include "cafCmdFeature.h"

#include <set>
#include <vector>

class RimSummaryAddressCollection;
class RimSummaryMultiPlot;
class RifEclipseSummaryAddress;
class RimSummaryPlot;
class RimSummaryCase;
class RimSummaryEnsemble;

//==================================================================================================
///
//==================================================================================================
class RicAppendSummaryPlotsForObjectsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static std::vector<RimSummaryAddressCollection*> selectedCollections();

    static std::vector<RimSummaryPlot*> plotsForOneInstanceOfObjectType( const std::vector<RimSummaryPlot*>&                sourcePlots,
                                                                         RimSummaryAddressCollection::CollectionContentType objectType );

    static bool isSelectionCompatibleWithPlot( const std::vector<RimSummaryAddressCollection*>& selection,
                                               RimSummaryMultiPlot*                             summaryMultiPlot );

    static void appendPlots( RimSummaryMultiPlot* summaryMultiPlot, const std::vector<RimSummaryAddressCollection*>& selection );

    static void appendPlots( RimSummaryMultiPlot*                    summaryMultiPlot,
                             const std::vector<RimSummaryCase*>&     cases,
                             const std::vector<RimSummaryEnsemble*>& ensembles );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static std::vector<RimSummaryAddressCollection*> createAddressCollections( const std::vector<RimSummaryCase*>&     cases,
                                                                               const std::vector<RimSummaryEnsemble*>& ensembles );
};
