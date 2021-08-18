/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimcCommandRouter.h"
#include "RimCommandRouter.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectHandle.h"

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCommandRouter, RimcCommandRouter_extractSurfaces, "ExtractSurfaces" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCommandRouter_extractSurfaces::RimcCommandRouter_extractSurfaces( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Extract Layer Surface", "", "", "Extract Layer Surface" );

    CAF_PDM_InitScriptableField( &m_gridModelFilename, "gridModelFilename", QString(), "Grid Model Case Filename", "", "", "" );
    CAF_PDM_InitScriptableField( &m_layers, "layers", std::vector<int>(), "Layers", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minimumI, "minimumI", -1, "Minimum I", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maximumI, "maximumI", -1, "Maximum I", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minimumJ, "minimumJ", -1, "Minimum J", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maximumJ, "maximumJ", -1, "Maximum J", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcCommandRouter_extractSurfaces::execute()
{
    /*
        auto parentWellPath = self<RimModeledWellPath>();

        auto lateral = RicNewWellPathLateralAtDepthFeature::createLateralAtMeasuredDepth( parentWellPath, m_tieInDepth
       ); if ( !m_lateralName().isEmpty() )
        {
            lateral->setName( m_lateralName );
        }
        lateral->geometryDefinition()->enableTargetPointPicking( false );

        return lateral;*/

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcCommandRouter_extractSurfaces::isNullptrValidResult() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcCommandRouter_extractSurfaces::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcCommandRouter_extractSurfaces::defaultResult() const
{
    return nullptr;
}
