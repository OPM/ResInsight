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
#include <vector>

class RimAbstractCorrelationPlot;
class RimCorrelationPlot;
class RimCorrelationMatrixPlot;
class RimCorrelationReportPlot;
class RimParameterResultCrossPlot;
class RimSummaryCaseCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimCorrelationPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCorrelationPlotCollection();
    ~RimCorrelationPlotCollection() override;

    RimCorrelationPlot* createCorrelationPlot( bool defaultToFirstEnsembleFopt = true );
    RimCorrelationPlot*
        createCorrelationPlot( RimSummaryCaseCollection* ensemble, const QString& quantityName, std::time_t timeStep );

    RimCorrelationMatrixPlot* createCorrelationMatrixPlot( bool defaultToFirstEnsembleField = true );
    RimCorrelationMatrixPlot* createCorrelationMatrixPlot( RimSummaryCaseCollection*   ensemble,
                                                           const std::vector<QString>& quantityNames,
                                                           std::time_t                 timeStep );

    RimParameterResultCrossPlot* createParameterResultCrossPlot( bool defaultToFirstEnsembleFopt = true );
    RimParameterResultCrossPlot* createParameterResultCrossPlot( RimSummaryCaseCollection* ensemble,
                                                                 const QString&            paramName,
                                                                 const QString&            quantityName,
                                                                 std::time_t               timeStep );

    RimCorrelationReportPlot* createCorrelationReportPlot( bool defaultToFirstEnsembleField = true );
    RimCorrelationReportPlot* createCorrelationReportPlot( RimSummaryCaseCollection*   ensemble,
                                                           const std::vector<QString>& matrixQuantityNames,
                                                           const QString&              tornadoAndCrossPlotQuantityName,
                                                           std::time_t                 timeStep );

    void removePlot( RimAbstractCorrelationPlot* correlationPlot );
    void removeReport( RimCorrelationReportPlot* correlationReport );

    std::vector<RimAbstractCorrelationPlot*> plots();
    std::vector<RimCorrelationReportPlot*>   reports();

    void deleteAllChildObjects();

private:
    void applyFirstEnsembleFieldAddressesToPlot( RimAbstractCorrelationPlot* plot,
                                                 const std::vector<QString>& quantityNames = {} );
    void applyEnsembleFieldAndTimeStepToPlot( RimAbstractCorrelationPlot* plot,
                                              RimSummaryCaseCollection*   ensemble,
                                              const std::vector<QString>& quantityNames,
                                              std::time_t                 timeStep );
    void applyFirstEnsembleFieldAddressesToReport( RimCorrelationReportPlot*   plot,
                                                   const std::vector<QString>& matrixQuantityNames,
                                                   const QString&              tornadoAndCrossPlotQuantityName );
    void applyEnsembleFieldAndTimeStepToReport( RimCorrelationReportPlot*   plot,
                                                RimSummaryCaseCollection*   ensemble,
                                                const std::vector<QString>& matrixQuantityNames,
                                                const QString&              tornadoAndCrossPlotQuantityName,
                                                std::time_t                 timeStep );

private:
    caf::PdmChildArrayField<RimAbstractCorrelationPlot*> m_correlationPlots;
    caf::PdmChildArrayField<RimCorrelationReportPlot*>   m_correlationReports;
};
