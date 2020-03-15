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

#include "RicfCommandObject.h"

#include "cafCmdFeature.h"
#include "cafPdmPtrArrayField.h"

class RimSummaryPlotCollection;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryPlot;

//==================================================================================================
///
//==================================================================================================
class RicNewSummaryPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

//==================================================================================================
///
//==================================================================================================
class RicNewDefaultSummaryPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimSummaryPlot* createFromSummaryCases( RimSummaryPlotCollection*           plotCollection,
                                                   const std::vector<RimSummaryCase*>& summaryCases );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

//==================================================================================================
///
//==================================================================================================
class RimSummaryPlotCollection_newSummaryPlot : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotCollection_newSummaryPlot( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle* execute();
    bool                  deleteObjectAfterReply() const override;

private:
    caf::PdmPtrArrayField<RimSummaryCase*>           m_summaryCases;
    caf::PdmPtrArrayField<RimSummaryCaseCollection*> m_ensembles;
};
