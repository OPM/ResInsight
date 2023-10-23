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

#pragma once

#include "cvfVector3.h"

#include <ostream>
#include <string>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RifInpExportTools
{
public:
    static bool printLine( std::ostream& stream, const std::string& line );
    static bool printHeading( std::ostream& stream, const std::string& heading );
    static bool printComment( std::ostream& stream, const std::string& comment );
    static bool printSectionComment( std::ostream& stream, const std::string& comment );
    static bool printNodes( std::ostream& stream, const std::vector<cvf::Vec3d>& nodes );
    static bool printElements( std::ostream& stream, const std::vector<std::vector<unsigned int>>& elements );
    static bool printNodeSet( std::ostream& stream, const std::string& partName, size_t start, size_t end, bool internal );
    static bool printNodeSet( std::ostream& stream, const std::string& partName, bool internal, const std::vector<unsigned int>& nodes );

    static bool printElementSet( std::ostream& stream, const std::string& partName, size_t start, size_t end );
    static bool
        printElementSet( std::ostream& stream, const std::string& elementName, bool internal, const std::vector<unsigned int>& elements );

    static bool
        printSurface( std::ostream& stream, const std::string& surfaceName, const std::string& surfaceElementName, const std::string& sideName );
    static bool printNumbers( std::ostream& stream, const std::vector<double>& values );
    static bool printNumber( std::ostream& stream, double value );

private:
    static bool printElements( std::ostream& stream, const std::vector<unsigned int>& elements );
};
