/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimStackablePlotCurve.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStackablePlotCurve::RimStackablePlotCurve()
    : stackingChanged( this )
    , stackingColorsChanged( this )

{
    CAF_PDM_InitObject( "StackableCurve", ":/WellLogCurve16x16.png", "", "" );

    CAF_PDM_InitField( &m_isStacked, "StackCurve", false, "Stack Curve" );
    CAF_PDM_InitField( &m_isStackedWithPhaseColors, "StackPhaseColors", false, "  with Phase Colors" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimStackablePlotCurve::phaseType() const
{
    return RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStackablePlotCurve::assignStackColor( size_t index, size_t count )
{
    cvf::Color3f curveColor( cvf::Color3::BROWN );

    if ( count == 1 )
    {
        curveColor = RiaColorTables::phaseColor( phaseType() );
    }
    else
    {
        auto allPhaseColors = RiaColorTables::phaseColors();
        auto it             = allPhaseColors.find( phaseType() );
        if ( it != allPhaseColors.end() )
        {
            caf::ColorTable interpolatedPhaseColors = it->second.interpolated( count );

            curveColor = interpolatedPhaseColors.cycledColor3f( index );
        }
    }

    setFillColor( curveColor );

    {
        auto moreSaturatedColor = RiaColorTools::toQColor( curveColor );
        moreSaturatedColor      = RiaColorTools::modifySaturation( moreSaturatedColor, 1.2 );

        m_curveAppearance->setColor( RiaColorTools::fromQColorTo3f( moreSaturatedColor ) );
    }

    this->updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStackablePlotCurve::isStacked() const
{
    return m_isStacked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStackablePlotCurve::isStackedWithPhaseColors() const
{
    return m_isStackedWithPhaseColors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStackablePlotCurve::setIsStacked( bool stacked )
{
    m_isStacked = stacked;

    if ( !m_isStacked() && fillStyle() != Qt::NoBrush )
    {
        // Switch off area fill when turning off stacking.
        setFillStyle( Qt::NoBrush );
    }
    stackingChanged.send( m_isStacked() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStackablePlotCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimPlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_isStacked )
    {
        if ( !m_isStacked() && fillStyle() != Qt::NoBrush )
        {
            // Switch off area fill when turning off stacking.
            setFillStyle( Qt::NoBrush );
        }
        stackingChanged.send( m_isStacked() );
    }
    else if ( changedField == &m_isStackedWithPhaseColors )
    {
        stackingColorsChanged.send( m_isStackedWithPhaseColors() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStackablePlotCurve::onFillColorChanged( const caf::SignalEmitter* emitter )
{
    m_isStackedWithPhaseColors = false;
    this->updateConnectedEditors();
    stackingColorsChanged.send( m_isStackedWithPhaseColors() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStackablePlotCurve::stackingUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_isStacked );
    if ( m_isStacked() ) uiOrdering.add( &m_isStackedWithPhaseColors );
}
