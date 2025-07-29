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

#include "Histogram/RimHistogramMultiPlotCollection.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QPointer>

class RimAnalysisPlotCollection;
class RimCorrelationPlotCollection;
class RimWellLogPlotCollection;
class RimRftPlotCollection;
class RimPltPlotCollection;
class RimGridCrossPlotCollection;
class RimMultiPlotCollection;
class RimSummaryMultiPlotCollection;
class RimSummaryCrossPlotCollection;
class RimSummaryTableCollection;
class RimSummaryPlot;
class RimSummaryPlotCollection;
class RifReaderEclipseSummary;
class RimEclipseResultCase;
class RimFlowPlotCollection;
class RimSaturationPressurePlotCollection;
class RimStimPlanModelPlotCollection;
class RimVfpPlotCollection;
class RimPlotCollection;
class RimHistogramMultiPlotCollection;

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

    static RimMainPlotCollection* current();

    RimWellLogPlotCollection*            wellLogPlotCollection() const;
    RimRftPlotCollection*                rftPlotCollection() const;
    RimPltPlotCollection*                pltPlotCollection() const;
    RimSummaryMultiPlotCollection*       summaryMultiPlotCollection() const;
    RimSummaryTableCollection*           summaryTableCollection() const;
    RimAnalysisPlotCollection*           analysisPlotCollection() const;
    RimCorrelationPlotCollection*        correlationPlotCollection() const;
    RimFlowPlotCollection*               flowPlotCollection() const;
    RimGridCrossPlotCollection*          gridCrossPlotCollection() const;
    RimSaturationPressurePlotCollection* saturationPressurePlotCollection() const;
    RimMultiPlotCollection*              multiPlotCollection() const;
    RimStimPlanModelPlotCollection*      stimPlanModelPlotCollection() const;
    RimVfpPlotCollection*                vfpPlotCollection() const;
    RimHistogramMultiPlotCollection*     histogramMultiPlotCollection() const;

    void deleteAllContainedObjects();
    void updateCurrentTimeStepInPlots();
    void updatePlotsWithFormations();
    void scheduleUpdatePlotsWithCompletions();
    void deleteAllCachedData();
    void ensureDefaultFlowPlotsAreCreated();
    void loadDataAndUpdateAllPlots();
    void updateSelectedWell( const QString& wellName, int timeStep );

protected:
    void initAfterRead() override;

private:
    // Overridden PDM methods
    caf::PdmFieldHandle* objectToggleField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void                            loadDataAndUpdatePlotCollectionsWithProgressInfo( const std::vector<RimPlotCollection*>& );
    void                            loadDataAndUpdatePlotCollections( const std::vector<RimPlotCollection*>& );
    std::vector<RimPlotCollection*> plotCollectionsWithFormations() const;
    std::vector<RimPlotCollection*> plotCollectionsWithCompletions() const;
    std::vector<RimPlotCollection*> allPlotCollections() const;

private:
    caf::PdmChildField<RimWellLogPlotCollection*>            m_wellLogPlotCollection;
    caf::PdmChildField<RimRftPlotCollection*>                m_rftPlotCollection;
    caf::PdmChildField<RimPltPlotCollection*>                m_pltPlotCollection;
    caf::PdmChildField<RimSummaryMultiPlotCollection*>       m_summaryMultiPlotCollection;
    caf::PdmChildField<RimSummaryCrossPlotCollection*>       m_summaryCrossPlotCollection_OBSOLETE;
    caf::PdmChildField<RimSummaryTableCollection*>           m_summaryTableCollection;
    caf::PdmChildField<RimAnalysisPlotCollection*>           m_analysisPlotCollection;
    caf::PdmChildField<RimCorrelationPlotCollection*>        m_correlationPlotCollection;
    caf::PdmChildField<RimFlowPlotCollection*>               m_flowPlotCollection;
    caf::PdmChildField<RimGridCrossPlotCollection*>          m_gridCrossPlotCollection;
    caf::PdmChildField<RimSaturationPressurePlotCollection*> m_saturationPressurePlotCollection;
    caf::PdmChildField<RimMultiPlotCollection*>              m_multiPlotCollection;
    caf::PdmChildField<RimStimPlanModelPlotCollection*>      m_stimPlanModelPlotCollection;
    caf::PdmChildField<RimVfpPlotCollection*>                m_vfpPlotCollection;
    caf::PdmChildField<RimHistogramMultiPlotCollection*>     m_histogramMultiPlotCollection;

    caf::PdmField<bool> m_show;

    caf::PdmChildField<RimSummaryPlotCollection*> m_summaryPlotCollection_OBSOLETE;
};
