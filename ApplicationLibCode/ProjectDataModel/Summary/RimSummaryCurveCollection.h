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

#include "RiaSummaryDefines.h"
#include "RimSummaryPlotSourceStepping.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryPlot;
class RiuPlotWidget;
class RiuPlotCurve;
class QKeyEvent;

//==================================================================================================
///
//==================================================================================================
class RimSummaryCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

private:
    caf::Signal<> curvesChanged;

public:
    RimSummaryCurveCollection();

    bool isCurvesVisible();

    void             setCurveForSourceStepping( RimSummaryCurve* curve );
    RimSummaryCurve* curveForSourceStepping() const;

    RimSummaryPlotSourceStepping* sourceSteppingObject() const;

    std::set<RiaDefines::HorizontalAxisType> horizontalAxisTypes() const;
    std::vector<RimSummaryCurve*>            curves() const;

    void setCurveAsTopZWithinCategory( RimSummaryCurve* curve );

    void loadDataAndUpdate( bool updateParentPlot );

    void updateCurveOrder();

private:
    void setParentPlotAndReplot( RiuPlotWidget* plot );
    void setParentPlotNoReplot( RiuPlotWidget* plot );
    void detachPlotCurves();
    void reattachPlotCurves();

    RimSummaryCurve* findRimCurveFromPlotCurve( const RiuPlotCurve* curve ) const;

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
    void                 defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void onCurvesReordered( const SignalEmitter* emitter );
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

private:
    friend class RimSummaryPlot;

    caf::PdmField<bool>                       m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;
    caf::PdmField<bool>                       m_editPlot;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_ySourceStepping;

    caf::PdmPointer<RimSummaryCurve> m_currentSummaryCurve;
    caf::PdmPointer<RimSummaryCurve> m_curveForSourceStepping;
};
