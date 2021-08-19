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

#include "opm/io/eclipse/EGrid.hpp"

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCommandRouter, RimcCommandRouter_extractSurfaces, "ExtractSurfaces" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCommandRouter_extractSurfaces::RimcCommandRouter_extractSurfaces( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Extract Layer Surface", "", "", "Extract Layer Surface" );

    CAF_PDM_InitScriptableField( &m_gridModelFilename, "GridModelFilename", QString(), "Grid Model Case Filename", "", "", "" );
    CAF_PDM_InitScriptableField( &m_layers, "Layers", std::vector<int>(), "Layers", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minimumI, "MinimumI", -1, "Minimum I", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maximumI, "MaximumI", -1, "Maximum I", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minimumJ, "MinimumJ", -1, "Minimum J", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maximumJ, "MaximumJ", -1, "Maximum J", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcCommandRouter_extractSurfaces::execute()
{
    try
    {
        std::string       filename = m_gridModelFilename().toStdString();
        Opm::EclIO::EGrid grid1( filename );

        auto dims = grid1.dimension();
        int  minI = m_minimumI() == -1 ? 0 : m_minimumI();
        int  maxI = m_maximumJ() == -1 ? dims[0] : m_maximumI();
        int  minJ = m_minimumI() == -1 ? 0 : m_minimumJ();
        int  maxJ = m_minimumI() == -1 ? dims[1] : m_maximumJ();

        std::array<int, 4> range = { minI, maxI, minJ, maxJ };

        for ( auto layer : m_layers() )
        {
            auto xyz_data = grid1.getXYZ_layer( layer, range, false );

            // Create surface from coords
            // Write to TS file on disk
        }
    }
    catch ( ... )
    {
    }

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
