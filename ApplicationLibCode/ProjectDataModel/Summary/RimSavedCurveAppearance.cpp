/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimSavedCurveAppearance.h"

#include "RimEnsembleCurveSet.h"
#include "RimPlotCurve.h"
#include "RimPlotCurveAppearance.h"

CAF_PDM_SOURCE_INIT( RimSavedCurveAppearance, "RimSavedCurveAppearance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSavedCurveAppearance::RimSavedCurveAppearance()
{
    CAF_PDM_InitObject( "Saved Curve Appearance" );

    CAF_PDM_InitFieldNoDefault( &m_curve, "Curve", "Curve" );
    CAF_PDM_InitFieldNoDefault( &m_appearance, "CurveAppearance", "Curve Appearance" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "EnsembleCurveSet", "Ensemble Curve Set" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleColor, "EnsembleColor", "Ensemble Color" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSavedCurveAppearance::setCurve( RimPlotCurve* curve )
{
    m_curve      = curve;
    m_appearance = curve->curveAppearance()->copyObject<RimPlotCurveAppearance>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSavedCurveAppearance::setEnsembleCurveSet( RimEnsembleCurveSet* curveSet )
{
    m_ensembleCurveSet = curveSet;
    m_ensembleColor    = curveSet->color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimSavedCurveAppearance::restore() const
{
    if ( m_curve() && m_appearance() )
    {
        m_curve()->curveAppearance()->copyFrom( *m_appearance() );
        return m_curve();
    }

    if ( m_ensembleCurveSet() )
    {
        m_ensembleCurveSet()->setColorAndUpdateCurves( m_ensembleColor() );
        return m_ensembleCurveSet();
    }

    return nullptr;
}
