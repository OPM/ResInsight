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

#include "RimCurveIntersectionBand.h"

#include "RimEnsembleSurface.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

CAF_PDM_SOURCE_INIT( RimCurveIntersectionBand, "RimCurveIntersectionBand" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveIntersectionBand::RimCurveIntersectionBand()
{
    CAF_PDM_InitObject( "CurveIntersectionBand", ":/CrossSection16x16.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_surface1, "Surface1", "Surface 1", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_surface2, "Surface2", "Surface 2", "", "", "" );

    CAF_PDM_InitField( &m_transparency, "Transparency", 0.5, "Transparency", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveIntersectionBand::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveIntersectionBand::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimCurveIntersectionBand::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_surface1 || fieldNeedingOptions == &m_surface2 )
    {
        RimSurfaceCollection* surfColl = RimProject::current()->activeOilField()->surfaceCollection();

        caf::IconProvider surfaceIcon( ":/ReservoirSurface16x16.png" );
        for ( auto surf : surfColl->surfaces() )
        {
            options.push_back( caf::PdmOptionItemInfo( surf->userDescription(), surf, false, surfaceIcon ) );
        }
        for ( auto ensambleSurf : surfColl->ensembleSurfaces() )
        {
            for ( auto surf : ensambleSurf->surfaces() )
            {
                options.push_back( caf::PdmOptionItemInfo( surf->userDescription(), surf, false, surfaceIcon ) );
            }
        }
    }

    return options;
}
