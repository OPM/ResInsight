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

#include "RimAbstractPlotCollection.h"
#include "RimAnalysisPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <ctime>

class RimSummaryCaseCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimAnalysisPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimAnalysisPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnalysisPlotCollection();
    ~RimAnalysisPlotCollection() override;

    RimAnalysisPlot* createAnalysisPlot();
    RimAnalysisPlot*
        createAnalysisPlot( RimSummaryCaseCollection* ensemble, const QString& quantityName, std::time_t timeStep );

    void updateSummaryNameHasChanged();

    std::vector<RimAnalysisPlot*> plots() const final;
    size_t                        plotCount() const final;
    void                          insertPlot( RimAnalysisPlot* analysisPlot, size_t index ) final;
    void                          removePlot( RimAnalysisPlot* analysisPlot ) final;

private:
    void applyFirstEnsembleFieldAddressesToPlot( RimAnalysisPlot* plot, const std::string& quantityName = "" );
    void applyFirstSummaryCaseCollectionAndFieldAddressesToPlot( RimAnalysisPlot*   plot,
                                                                 const std::string& quantityName = "" );
    void applyAllSummaryCasesAndFieldAddressesToPlot( RimAnalysisPlot* plot, const std::string& quantityName = "" );
    void applySummaryCaseCollectionAndFieldAddressToPlot( RimAnalysisPlot*          plot,
                                                          RimSummaryCaseCollection* summaryCaseCollection,
                                                          const std::string&        quantityName );

    RimSummaryCaseCollection* firstEnsemble() const;
    RimSummaryCaseCollection* firstSummaryCaseCollection() const;

private:
    caf::PdmChildArrayField<RimAnalysisPlot*> m_analysisPlots;
};
