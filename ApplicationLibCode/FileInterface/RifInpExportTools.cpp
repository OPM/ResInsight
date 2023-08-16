/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RifInpExportTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printLine( std::ostream& stream, const std::string& heading )
{
    stream << heading << std::endl;
    return stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printHeading( std::ostream& stream, const std::string& heading )
{
    return printLine( stream, "*" + heading );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printComment( std::ostream& stream, const std::string& comment )
{
    return printLine( stream, "** " + comment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNodes( std::ostream& stream, const std::vector<cvf::Vec3d>& nodes )
{
    if ( !printHeading( stream, "Node" ) ) return false;

    for ( size_t i = 0; i < nodes.size(); i++ )
    {
        stream << i + 1 << ", " << nodes[i].x() << ", " << nodes[i].y() << ", " << nodes[i].z() << std::endl;
    }

    return stream.good();
}
