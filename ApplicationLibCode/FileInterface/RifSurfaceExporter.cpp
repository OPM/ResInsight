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

#include "RifSurfaceExporter.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSurfaceExporter::writeGocadTSurfFile( const QString&                 fileName,
                                              const QString&                 headerText,
                                              const std::vector<cvf::Vec3d>& vertices,
                                              const std::vector<unsigned>&   triangleIndices )
{
    QFile exportFile( fileName );

    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    QTextStream out( &exportFile );

    QString headerForExport = headerForExport = "surface";
    if ( !headerText.isEmpty() ) headerForExport = headerText;

    out << "GOCAD TSurf 1 \n";
    out << "HEADER { \n";
    out << "name:" + headerForExport + " \n";
    out << "} \n";
    out << "GOCAD_ORIGINAL_COORDINATE_SYSTEM \n";
    out << "NAME Default  \n";
    out << "AXIS_NAME \"X\" \"Y\" \"Z\" \n";
    out << "AXIS_UNIT \"m\" \"m\" \"m\" \n";
    out << "ZPOSITIVE Depth \n";
    out << "END_ORIGINAL_COORDINATE_SYSTEM \n";

    out << "TFACE \n";

    size_t i = 1;
    for ( auto v : vertices )
    {
        out << "VRTX " << i << " ";
        out << v.x() << " ";
        out << v.y() << " ";
        out << -v.z() << " ";
        out << "CNXYZ\n";

        i++;
    }

    for ( size_t triIndex = 0; triIndex < triangleIndices.size(); triIndex += 3 )
    {
        out << "TRGL ";
        out << " " << 1 + triangleIndices[triIndex + 0];
        out << " " << 1 + triangleIndices[triIndex + 1];
        out << " " << 1 + triangleIndices[triIndex + 2];
        out << " \n";
    }

    out << "END\n";

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSurfaceExporter::writePetrelPtlFile( const QString&                                            fileName,
                                             const std::vector<cvf::Vec3d>&                            vertices,
                                             const std::vector<std::pair<unsigned int, unsigned int>>& columnRowIndices )
{
    if ( vertices.size() != columnRowIndices.size() ) return false;

    QFile exportFile( fileName );

    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    QTextStream out( &exportFile );

    out << "#Type: scattered data \n";
    out << "#Version: 6 \n";
    out << "#Description: No description \n";
    out << "#Format: free \n";
    out << "#Field: 1 x \n";
    out << "#Field: 2 y \n";
    out << "#Field: 3 z meters \n";
    out << "#Field: 4 column \n";
    out << "#Field: 5 row \n";
    out << "#Projection: Local Rectangular \n";
    out << "#Units: meters \n";
    out << "#End:  \n";
    out << "#Information from grid \n";
    out << "#Grid_size: Not_available \n";
    out << "#Grid_space: Not_available \n";
    out << "#Z_field: z \n";
    out << "#Vertical_faults: Not_available \n";
    out << "#History: No history \n";
    out << "#Z_units: meters \n";

    for ( size_t i = 0; i < vertices.size(); i++ )
    {
        out << vertices[i].x() << " ";
        out << vertices[i].y() << " ";
        out << vertices[i].z() << " "; // SIGN? depth or Z, Z needed on PTL?

        out << columnRowIndices[i].first << " ";
        out << columnRowIndices[i].second;

        out << "\n";
    }

    return true;
}
