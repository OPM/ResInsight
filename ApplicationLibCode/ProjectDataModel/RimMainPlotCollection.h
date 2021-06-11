/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QDockWidget>
#include <QPointer>

class RimAnalysisPlotCollection;
class RimCorrelationPlotCollection;
class RimWellLogPlotCollection;
class RimRftPlotCollection;
class RimPltPlotCollection;
class RimGridCrossPlotCollection;
class RimMultiPlotCollection;
class RimSummaryPlotCollection;
class RimSummaryCrossPlotCollection;
class RimSummaryPlot;
class RifReaderEclipseSummary;
class RimEclipseResultCase;
class RimFlowPlotCollection;
class RimSaturationPressurePlotCollection;
class RimStimPlanModelPlotCollection;
class RimVfpPlotCollection;
class RimPlotCollection;

#ifdef USE_QTCHARTS
class RimGridStatisticsPlotCollection;
class RimEnsembleFractureStatisticsPlotCollection;
#endif

//==================================================================================================
///
///
//==================================================================================================
class RimMainPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimMainPlotCollection();
    ~RimMainPlotCollection() override;

    RimWellLogPlotCollection*            wellLogPlotCollection() const;
    RimRftPlotCollection*                rftPlotCollection() const;
    RimPltPlotCollection*                pltPlotCollection() const;
    RimSummaryPlotCollection*            summaryPlotCollection() const;
    RimSummaryCrossPlotCollection*       summaryCrossPlotCollection() const;
    RimAnalysisPlotCollection*           analysisPlotCollection() const;
    RimCorrelationPlotCollection*        correlationPlotCollection() const;
    RimFlowPlotCollection*               flowPlotCollection() const;
    RimGridCrossPlotCollection*          gridCrossPlotCollection() const;
    RimSaturationPressurePlotCollection* saturationPressurePlotCollection() const;
    RimMultiPlotCollection*              multiPlotCollection() const;
    RimStimPlanModelPlotCollection*      stimPlanModelPlotCollection() const;
    RimVfpPlotCollection*                vfpPlotCollection() const;

#ifdef USE_QTCHARTS
    RimGridStatisticsPlotCollection*             gridStatisticsPlotCollection() const;
    RimEnsembleFractureStatisticsPlotCollection* ensembleFractureStatisticsPlotCollection() const;
#endif

    void deleteAllContainedObjects();
    void updateCurrentTimeStepInPlots();
    void updatePlotsWithFormations();
    void updatePlotsWithCompletions();
    void deleteAllCachedData();
    void ensureDefaultFlowPlotsAreCreated();
    void ensureCalculationIdsAreAssigned();
    void loadDataAndUpdateAllPlots();

private:
    // Overridden PDM methods
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void loadDataAndUpdatePlotCollectionsWithProgressInfo( const std::vector<RimPlotCollection*>& );
    void loadDataAndUpdatePlotCollections( const std::vector<RimPlotCollection*>& );
    std::vector<RimPlotCollection*> plotCollectionsWithFormations() const;
    std::vector<RimPlotCollection*> plotCollectionsWithCompletions() const;
    std::vector<RimPlotCollection*> allPlotCollections() const;

private:
    caf::PdmChildField<RimWellLogPlotCollection*>            m_wellLogPlotCollection;
    caf::PdmChildField<RimRftPlotCollection*>                m_rftPlotCollection;
    caf::PdmChildField<RimPltPlotCollection*>                m_pltPlotCollection;
    caf::PdmChildField<RimSummaryPlotCollection*>            m_summaryPlotCollection;
    caf::PdmChildField<RimSummaryCrossPlotCollection*>       m_summaryCrossPlotCollection;
    caf::PdmChildField<RimAnalysisPlotCollection*>           m_analysisPlotCollection;
    caf::PdmChildField<RimCorrelationPlotCollection*>        m_correlationPlotCollection;
    caf::PdmChildField<RimFlowPlotCollection*>               m_flowPlotCollection;
    caf::PdmChildField<RimGridCrossPlotCollection*>          m_gridCrossPlotCollection;
    caf::PdmChildField<RimSaturationPressurePlotCollection*> m_saturationPressurePlotCollection;
    caf::PdmChildField<RimMultiPlotCollection*>              m_multiPlotCollection;
    caf::PdmChildField<RimStimPlanModelPlotCollection*>      m_stimPlanModelPlotCollection;
    caf::PdmChildField<RimVfpPlotCollection*>                m_vfpPlotCollection;
#ifdef USE_QTCHARTS
    caf::PdmChildField<RimGridStatisticsPlotCollection*>             m_gridStatisticsPlotCollection;
    caf::PdmChildField<RimEnsembleFractureStatisticsPlotCollection*> m_ensembleFractureStatisticsPlotCollection;
#endif

    caf::PdmField<bool> m_show;
};
