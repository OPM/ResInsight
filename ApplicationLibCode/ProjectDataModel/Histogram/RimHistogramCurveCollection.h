/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

class RimHistogramPlot;
class RimHistogramCurve;
class RiuPlotWidget;
class RiuPlotCurve;
class QKeyEvent;

//==================================================================================================
///
//==================================================================================================
class RimHistogramCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

private:
    caf::Signal<> curvesChanged;

public:
    RimHistogramCurveCollection();

    bool isCurvesVisible();

    std::vector<RimHistogramCurve*> curves() const;

    void loadDataAndUpdate( bool updateParentPlot );

    void updateCurveOrder();

private:
    void setParentPlotAndReplot( RiuPlotWidget* plot );
    void setParentPlotNoReplot( RiuPlotWidget* plot );
    void detachPlotCurves();
    void reattachPlotCurves();

    void addCurve( RimHistogramCurve* curve );
    void insertCurve( RimHistogramCurve* curve, size_t index );
    void deleteCurve( RimHistogramCurve* curve );
    void removeCurve( RimHistogramCurve* curve );

    void deleteAllCurves();
    void updateCaseNameHasChanged();

    void setCurrentHistogramCurve( RimHistogramCurve* curve );

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

private:
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onCurvesReordered( const SignalEmitter* emitter );
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

private:
    friend class RimHistogramPlot;

    caf::PdmField<bool>                         m_showCurves;
    caf::PdmChildArrayField<RimHistogramCurve*> m_curves;

    caf::PdmPointer<RimHistogramCurve> m_currentHistogramCurve;
};
