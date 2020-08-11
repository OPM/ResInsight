/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-  Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <ctime>

class RimAnalysisPlot;
class RimSummaryCaseCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimAnalysisPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnalysisPlotCollection();
    ~RimAnalysisPlotCollection() override;

    RimAnalysisPlot* createAnalysisPlot();
    RimAnalysisPlot*
        createAnalysisPlot( RimSummaryCaseCollection* ensemble, const QString& quantityName, std::time_t timeStep );

    void updateSummaryNameHasChanged();
    void removeSummaryPlot( RimAnalysisPlot* analysisPlot );

    std::vector<RimAnalysisPlot*> plots();

    void deleteAllChildObjects();

private:
    void applyFirstEnsembleFieldAddressesToPlot( RimAnalysisPlot* plot, const std::string& quantityName = "" );
    void applyEnsembleFieldAndTimeStepToPlot( RimAnalysisPlot*          plot,
                                              RimSummaryCaseCollection* ensemble,
                                              const std::string&        quantityName,
                                              std::time_t               timeStep );

private:
    caf::PdmChildArrayField<RimAnalysisPlot*> m_analysisPlots;
};
