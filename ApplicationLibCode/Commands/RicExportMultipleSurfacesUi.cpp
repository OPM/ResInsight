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

#include "RicExportMultipleSurfacesUi.h"

#include "RiaApplication.h"

#include "cafPdmObject.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RicExportMultipleSurfacesUi, "RicExportMultipleSurfacesUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportMultipleSurfacesUi::RicExportMultipleSurfacesUi()
{
    CAF_PDM_InitObject( "Export Multiple Surfaces", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_layers, "Layers", "Layers", "", "", "" );
    CAF_PDM_InitField( &m_autoCreateEnsembleSurfaces,
                       "AutoCreateEnsembleSurfaces",
                       false,
                       "Create Ensemble Surfaces From Exported Files",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_minLayer, "MinLayer", "MinLayer", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_maxLayer, "MaxLayer", "MaxLayer", "", "", "" );

    m_tabNames << "Layers"
               << "Ensemble Surfaces";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportMultipleSurfacesUi::~RicExportMultipleSurfacesUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RicExportMultipleSurfacesUi::tabNames() const
{
    return m_tabNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesUi::setLayersMinMax( int minLayer, int maxLayer )
{
    m_minLayer = minLayer;
    m_maxLayer = maxLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_layers )
    {
        caf::PdmUiListEditorAttribute* myAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_heightHint = 280;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSurfacesUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == m_tabNames[0] )
    {
        uiOrdering.add( &m_layers );
    }
    else if ( uiConfigName == m_tabNames[1] )
    {
        uiOrdering.add( &m_autoCreateEnsembleSurfaces );
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicExportMultipleSurfacesUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_layers )
    {
        for ( int layer = m_minLayer; layer < m_maxLayer; layer++ )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( layer ), layer ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RicExportMultipleSurfacesUi::layers() const
{
    return m_layers();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportMultipleSurfacesUi::autoCreateEnsembleSurfaces() const
{
    return m_autoCreateEnsembleSurfaces;
}
