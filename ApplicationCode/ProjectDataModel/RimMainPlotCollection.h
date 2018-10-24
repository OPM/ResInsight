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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildField.h"

#include <QPointer>
#include <QDockWidget>


class RimWellLogPlotCollection;
class RimRftPlotCollection;
class RimPltPlotCollection;
class RimSummaryPlotCollection;
class RimSummaryCrossPlotCollection;
class RimSummaryPlot;
class RifReaderEclipseSummary;
class RimEclipseResultCase;
class RimFlowPlotCollection;


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

    RimWellLogPlotCollection*       wellLogPlotCollection();
    RimRftPlotCollection*           rftPlotCollection();
    RimPltPlotCollection*           pltPlotCollection();
    RimSummaryPlotCollection*       summaryPlotCollection();
    RimSummaryCrossPlotCollection*  summaryCrossPlotCollection();
    RimFlowPlotCollection*          flowPlotCollection();

    void                            deleteAllContainedObjects();
    void                            updateCurrentTimeStepInPlots();
    void                            updatePlotsWithFormations();
    void                            updatePlotsWithCompletions();
    void                            deleteAllCachedData();
private:

    // Overridden PDM methods
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmChildField<RimWellLogPlotCollection*>       m_wellLogPlotCollection;
    caf::PdmChildField<RimRftPlotCollection*>           m_rftPlotCollection;
    caf::PdmChildField<RimPltPlotCollection*>           m_pltPlotCollection;
    caf::PdmChildField<RimSummaryPlotCollection*>       m_summaryPlotCollection;
    caf::PdmChildField<RimSummaryCrossPlotCollection*>  m_summaryCrossPlotCollection;
    caf::PdmChildField<RimFlowPlotCollection*>          m_flowPlotCollection;

    caf::PdmField<bool> m_show;
};
