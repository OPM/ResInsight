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
#include <iomanip>

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
bool RifInpExportTools::printSectionComment( std::ostream& stream, const std::string& comment )
{
    return printComment( stream, "" ) && printComment( stream, comment ) && printComment( stream, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNodes( std::ostream& stream, const std::vector<cvf::Vec3d>& nodes )
{
    if ( !printHeading( stream, "Node" ) ) return false;

    for ( size_t i = 0; i < nodes.size(); i++ )
    {
        stream << i + 1 << ", " << std::setprecision( 10 ) << nodes[i].x() << ", " << nodes[i].y() << ", " << nodes[i].z() << std::endl;
    }

    return stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printElements( std::ostream& stream, const std::vector<std::vector<unsigned int>>& elements )
{
    std::string heading = "Element, type=C3D8P";
    if ( !printHeading( stream, heading ) ) return false;

    for ( size_t i = 0; i < elements.size(); i++ )
    {
        stream << i + 1;
        for ( size_t j = 0; j < elements[i].size(); j++ )
        {
            stream << ", " << elements[i][j] + 1;
        }
        stream << std::endl;
    }

    return stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNodeSet( std::ostream& stream, const std::string& partName, size_t start, size_t end, bool internal )
{
    // Should look like this:
    // *Nset, nset=part1, generate
    //     1,  50433,
    std::string internalStr = internal ? ", internal" : "";
    return printHeading( stream, "Nset, nset=" + partName + internalStr + ", generate" ) &&
           printLine( stream, std::to_string( start ) + ", " + std::to_string( end ) + ", " + std::to_string( 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNodeSet( std::ostream& stream, const std::string& partName, bool internal, const std::vector<unsigned int>& nodes )
{
    std::string internalStr = internal ? ", internal" : "";
    return printHeading( stream, "Nset, nset=" + partName + internalStr ) && printElements( stream, nodes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printElementSet( std::ostream& stream, const std::string& partName, size_t start, size_t end )
{
    // Should look like this:
    // *Elset, elset=part1, generate
    //     1,  50433, 1
    return printHeading( stream, "Elset, elset=" + partName + ", generate" ) &&
           printLine( stream, std::to_string( start ) + ", " + std::to_string( end ) + ", " + std::to_string( 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printElementSet( std::ostream& stream, const std::string& elementName, bool internal, const std::vector<unsigned int>& elements )
{
    std::string internalStr = internal ? ", internal" : "";
    return printHeading( stream, "Elset, elset=" + elementName + internalStr ) && printElements( stream, elements );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printElements( std::ostream& stream, const std::vector<unsigned int>& elements )
{
    int numItemsPerLine = 16;

    for ( size_t i = 0; i < elements.size(); i++ )
    {
        stream << elements[i] + 1;
        if ( i != elements.size() - 1 ) stream << ", ";

        // Break lines periodically
        bool isFirst = i == 0;
        bool isLast  = i == ( elements.size() - 1 );
        if ( !isFirst && !isLast && ( i + 1 ) % numItemsPerLine == 0 )
        {
            stream << std::endl;
        }
    }
    stream << std::endl;

    return stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printSurface( std::ostream&      stream,
                                      const std::string& surfaceName,
                                      const std::string& surfaceElementName,
                                      const std::string& sideName )
{
    // Sample surface element:
    //*Surface, type=ELEMENT, name=top
    //_top_S5, S5
    return printHeading( stream, "Surface, type=ELEMENT, name=" + surfaceName ) && printLine( stream, surfaceElementName + ", " + sideName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNumber( std::ostream& stream, double value )
{
    stream << value << "," << std::endl;
    return stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpExportTools::printNumbers( std::ostream& stream, const std::vector<double>& values )
{
    for ( size_t i = 0; i < values.size(); i++ )
    {
        stream << values[i];
        if ( i != values.size() - 1 ) stream << ", ";
    }
    stream << std::endl;

    return stream.good();
}
