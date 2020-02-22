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

class RimEnsembleCurveSet;
class RimSummaryPlotSourceStepping;
class RimSummaryCurve;
class QwtPlot;
class QwtPlotCurve;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveSetCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleCurveSetCollection();
    ~RimEnsembleCurveSetCollection() override;

    bool isCurveSetsVisible();

    void loadDataAndUpdate( bool updateParentPlot );
    void setParentQwtPlotAndReplot( QwtPlot* plot );
    void detachQwtCurves();
    void reattachQwtCurves();

    RimSummaryCurve*     findRimCurveFromQwtCurve( const QwtPlotCurve* qwtCurve ) const;
    RimEnsembleCurveSet* findRimCurveSetFromQwtCurve( const QwtPlotCurve* qwtCurve ) const;

    void addCurveSet( RimEnsembleCurveSet* curveSet );
    void deleteCurveSet( RimEnsembleCurveSet* curveSet );
    void deleteCurveSets( const std::vector<RimEnsembleCurveSet*> curveSets );

    std::vector<RimEnsembleCurveSet*> curveSets() const;
    size_t                            curveSetCount() const;

    void deleteAllCurveSets();

    // Functions related to source stepping
    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();
    void                              setCurveSetForSourceStepping( RimEnsembleCurveSet* curve );
    RimEnsembleCurveSet*              curveSetForSourceStepping() const;
    std::vector<RimEnsembleCurveSet*> curveSetsForSourceStepping() const;
    RimSummaryPlotSourceStepping*     sourceSteppingObject() const;

private:
    caf::PdmFieldHandle* objectToggleField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<bool>                           m_showCurves;
    caf::PdmChildArrayField<RimEnsembleCurveSet*> m_curveSets;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_ySourceStepping;

    caf::PdmPointer<RimEnsembleCurveSet> m_curveSetForSourceStepping;
};
