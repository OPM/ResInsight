/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimEquilibriumAxisAnnotation.h"

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RimEclipseCase.h"
#include "RimPlot.h"
#include "RimTools.h"
#include "RimViewWindow.h"

#include <cmath>

namespace caf
{
template <>
void RimEquilibriumAxisAnnotation::ExportKeywordEnum::setUp()
{
    addItem( RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_USER_DEFINED, "User Defined", "User Defined" );
    addItem( RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT,
             "PL_EQUIL_WATER_OIL_CONTACT",
             "PL_EQUIL_WATER_OIL_CONTACT" );
    addItem( RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT,
             "PL_EQUIL_GAS_OIL_CONTACT",
             "PL_EQUIL_GAS_OIL_CONTACT" );
    setDefault( RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_USER_DEFINED );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimEquilibriumAxisAnnotation, "RimEquilibriumAxisAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEquilibriumAxisAnnotation::RimEquilibriumAxisAnnotation()
    : RimPlotAxisAnnotation()
{
    CAF_PDM_InitObject( "Equilibrium Annotation", ":/LeftAxis16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_annotationType, "AnnotationType", "AnnotationType", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_sourceCase, "Associated3DCase", "Eclipse Case", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_equilNum, "m_equilNum", "equil Num", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEquilibriumAxisAnnotation::setEquilibriumData( RimEclipseCase* eclipseCase,
                                                       int             zeroBasedEquilRegionIndex,
                                                       RimEquilibriumAxisAnnotation::PlotAxisAnnotationType annotationType )
{
    m_sourceCase     = eclipseCase;
    m_equilNum       = zeroBasedEquilRegionIndex + 1;
    m_annotationType = annotationType;

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEquilibriumAxisAnnotation::value() const
{
    if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT )
    {
        return selectedItem().waterOilContactDepth();
    }
    else if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT )
    {
        return selectedItem().gasOilContactDepth();
    }

    return m_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimEquilibriumAxisAnnotation::color() const
{
    if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT )
    {
        return QColor( 0, 0, 0 );
    }
    else if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT )
    {
        return QColor( 220, 0, 0 );
    }

    return RimPlotAxisAnnotation::color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEquilibriumAxisAnnotation::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_sourceCase )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_equilNum )
    {
        for ( int i = 0; i < static_cast<int>( equilItems().size() ); i++ )
        {
            QString uiText = QString( "%1" ).arg( i + 1 );
            options.push_back( caf::PdmOptionItemInfo( uiText, i ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEquilibriumAxisAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_annotationType );

    if ( m_annotationType() == PlotAxisAnnotationType::PL_USER_DEFINED )
    {
        uiOrdering.add( &m_name );
        uiOrdering.add( &m_value );
    }
    else
    {
        uiOrdering.add( &m_sourceCase );
        uiOrdering.add( &m_equilNum );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEquil RimEquilibriumAxisAnnotation::selectedItem() const
{
    int index = m_equilNum() - 1;

    if ( index < static_cast<int>( equilItems().size() ) )
    {
        return equilItems()[index];
    }

    return RigEquil::defaultObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEquil> RimEquilibriumAxisAnnotation::equilItems() const
{
    if ( m_sourceCase && m_sourceCase->eclipseCaseData() )
    {
        m_sourceCase->ensureDeckIsParsedForEquilData();

        return m_sourceCase->eclipseCaseData()->equilData();
    }

    return std::vector<RigEquil>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEquilibriumAxisAnnotation::updateName()
{
    QString text;

    if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT ||
         m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT )
    {
        double diffBetweenTwoContactDepths =
            std::fabs( selectedItem().gasOilContactDepth() - selectedItem().waterOilContactDepth() );

        if ( diffBetweenTwoContactDepths < 0.1 )
        {
            text = QString( "GWC %1" ).arg( selectedItem().gasOilContactDepth() );
        }
        else if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT )
        {
            text = QString( "WOC %1" ).arg( value() );
        }
        else if ( m_annotationType() == PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT )
        {
            text = QString( "GOC %1" ).arg( value() );
        }

        m_name = text;
    }
}
