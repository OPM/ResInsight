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

#include "RimSummaryPlotSourceStepping.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

class QwtPlot;
class QwtPlotCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryCrossPlot;
class RimSummaryPlot;
class QKeyEvent;

//==================================================================================================
///
//==================================================================================================
class RimSummaryCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

private:
    caf::Signal<> curvesReordered;

public:
    RimSummaryCurveCollection();
    ~RimSummaryCurveCollection() override;

    bool isCurvesVisible();

    void             setCurveForSourceStepping( RimSummaryCurve* curve );
    RimSummaryCurve* curveForSourceStepping() const;

    RimSummaryPlotSourceStepping*
        sourceSteppingObject( RimSummaryPlotSourceStepping::SourceSteppingType sourceSteppingType ) const;

    std::vector<RimSummaryCurve*> curves() const;
    std::vector<RimSummaryCurve*>
        curvesForSourceStepping( RimSummaryPlotSourceStepping::SourceSteppingType steppingType ) const;

    void setCurveAsTopZWithinCategory( RimSummaryCurve* curve );

    void loadDataAndUpdate( bool updateParentPlot );

private:
    void setParentQwtPlotAndReplot( QwtPlot* plot );
    void detachQwtCurves();
    void reattachQwtCurves();

    RimSummaryCurve* findRimCurveFromQwtCurve( const QwtPlotCurve* qwtCurve ) const;

    void addCurve( RimSummaryCurve* curve );
    void insertCurve( RimSummaryCurve* curve, size_t index );
    void deleteCurve( RimSummaryCurve* curve );
    void removeCurve( RimSummaryCurve* curve );

    void deleteCurvesAssosiatedWithCase( RimSummaryCase* summaryCase );
    void deleteAllCurves();
    void updateCaseNameHasChanged();

    void setCurrentSummaryCurve( RimSummaryCurve* curve );

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

private:
    caf::PdmFieldHandle* objectToggleField() override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void onCurvesReordered( const SignalEmitter* emitter );

private:
    friend class RimSummaryCrossPlot;
    friend class RimSummaryPlot;

    caf::PdmField<bool>                       m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;
    caf::PdmField<bool>                       m_editPlot;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_ySourceStepping;
    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_xSourceStepping;
    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_unionSourceStepping;

    caf::PdmPointer<RimSummaryCurve> m_currentSummaryCurve;
    caf::PdmPointer<RimSummaryCurve> m_curveForSourceStepping;
};
