/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

class QwtPlot;
class QwtPlotCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryPlotSourceStepping;
class QKeyEvent;

//==================================================================================================
///  
//==================================================================================================
class RimSummaryCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveCollection();
    virtual ~RimSummaryCurveCollection();

    bool                                    isCurvesVisible();

    void                                    loadDataAndUpdate(bool updateParentPlot);
    void                                    setParentQwtPlotAndReplot(QwtPlot* plot);
    void                                    detachQwtCurves();

    RimSummaryCurve*                        findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const;

    void                                    addCurve(RimSummaryCurve* curve);
    void                                    deleteCurve(RimSummaryCurve* curve);

    std::vector<RimSummaryCurve*>           curves() const;
    void                                    deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase);
    void                                    deleteAllCurves();
    void                                    updateCaseNameHasChanged();

    void                                    setCurrentSummaryCurve(RimSummaryCurve* curve);

    std::vector<caf::PdmFieldHandle*>       fieldsToShowInToolbar();

    void                                    handleKeyPressEvent(QKeyEvent* keyEvent);

private:
    caf::PdmFieldHandle*                    objectToggleField();
    virtual void                            defineObjectEditorAttribute(QString uiConfigName, 
                                                                        caf::PdmUiEditorAttribute* attribute) override;
    
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                             const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<bool>                         m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*>   m_curves;

    caf::PdmChildField<RimSummaryPlotSourceStepping*>   m_ySourceStepping;
    caf::PdmChildField<RimSummaryPlotSourceStepping*>   m_xSourceStepping;
    caf::PdmChildField<RimSummaryPlotSourceStepping*>   m_unionSourceStepping;

    caf::PdmPointer<RimSummaryCurve>                    m_currentSummaryCurve;
};

