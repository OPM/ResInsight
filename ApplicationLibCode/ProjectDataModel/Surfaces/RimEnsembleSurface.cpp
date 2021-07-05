/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleSurface.h"

#include "RiaLogging.h"

#include "RimFileSurface.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleSurface, "EnsembleSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurface::RimEnsembleSurface()
{
    CAF_PDM_InitScriptableObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fileSurfaces, "FileSurfaces", "", "", "", "" );
    m_fileSurfaces.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::removeFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.removeChildObject( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.push_back( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileSurface*> RimEnsembleSurface::fileSurfaces() const
{
    return m_fileSurfaces().childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::loadDataAndUpdate()
{
    for ( auto& w : m_fileSurfaces )
    {
        if ( !w->onLoadData() )
        {
            RiaLogging::warning( QString( "Failed to load surface: %1" ).arg( w->surfaceFilePath() ) );
        }
    }
}
