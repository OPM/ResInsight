/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include <vector>

class RimSummaryPlot;
class RimEnsembleCurveSet;
class RimSummaryEnsemble;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RicNewSummaryEnsembleCurveSetFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimSummaryPlot*                   createPlotForCurveSetsAndUpdate( std::vector<RimSummaryEnsemble*> ensembles );
    static std::vector<RimEnsembleCurveSet*> addDefaultCurveSets( RimSummaryPlot* plot, RimSummaryEnsemble* ensemble );
    static RimEnsembleCurveSet* addCurveSet( RimSummaryPlot* plot, RimSummaryEnsemble* ensemble, const RifEclipseSummaryAddress& address );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    RimSummaryPlot* selectedSummaryPlot() const;
};
