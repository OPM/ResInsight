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

#include "RimcExtractSurfaces.h"

#include "RiaLogging.h"
#include "RifSurfaceExporter.h"
#include "RimCommandRouter.h"

#include "opm/io/eclipse/EGrid.hpp"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectHandle.h"

#include <QDir>
#include <QFileInfo>

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCommandRouter, RimcCommandRouter_extractSurfaces, "ExtractSurfaces" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCommandRouter_extractSurfaces::RimcCommandRouter_extractSurfaces( caf::PdmObjectHandle* self )
    : RimCommandRouterMethod( self )
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
        int  maxI = m_maximumI() == -1 ? dims[0] - 1 : m_maximumI();
        int  minJ = m_minimumJ() == -1 ? 0 : m_minimumJ();
        int  maxJ = m_maximumJ() == -1 ? dims[1] - 1 : m_maximumJ();

        std::array<int, 4> range = { minI, maxI, minJ, maxJ };

        for ( auto layer : m_layers() )
        {
            bool bottom   = false;
            auto xyz_data = grid1.getXYZ_layer( layer, range, bottom );
            auto mapAxis  = grid1.get_mapaxes();

            // Create surface from coords

            std::vector<unsigned>   triangleIndices;
            std::vector<cvf::Vec3d> vertices;

            unsigned startOfCellCoordIndex = 0;
            while ( startOfCellCoordIndex + 4 < xyz_data.size() )
            {
                for ( size_t cornerIdx = 0; cornerIdx < 4; cornerIdx++ )
                {
                    auto coord1   = xyz_data[startOfCellCoordIndex + cornerIdx];
                    auto cvfCoord = cvf::Vec3d( coord1[0], coord1[1], -coord1[2] );

                    if ( !mapAxis.empty() )
                    {
                        cvfCoord[0] += mapAxis[2];
                        cvfCoord[1] += mapAxis[3];
                    }
                    vertices.push_back( cvfCoord );
                }

                triangleIndices.push_back( startOfCellCoordIndex );
                triangleIndices.push_back( startOfCellCoordIndex + 3 );
                triangleIndices.push_back( startOfCellCoordIndex + 2 );

                triangleIndices.push_back( startOfCellCoordIndex );
                triangleIndices.push_back( startOfCellCoordIndex + 1 );
                triangleIndices.push_back( startOfCellCoordIndex + 3 );

                // Coordinates are given for each four corners for each cell of the surface
                startOfCellCoordIndex += 4;
            }

            // Write to TS file on disk

            QFileInfo fi( m_gridModelFilename );
            QString   surfaceFilename = fi.absoluteDir().absolutePath() +
                                      QString( "/surfaceexport/layer-%1.ts" ).arg( layer );

            // TODO: Add more info in surface comment
            if ( !RifSurfaceExporter::writeGocadTSurfFile( surfaceFilename, "Surface comment", vertices, triangleIndices ) )
            {
                RiaLogging::error( "Failed to export surface data to " + surfaceFilename );
            }
            else
            {
                RiaLogging::error( "Successfully exported surface data to " + surfaceFilename );
            }
        }
    }
    catch ( ... )
    {
        RiaLogging::error( "Error during creation of surface data for model " + m_gridModelFilename() );
    }

    return nullptr;
}
