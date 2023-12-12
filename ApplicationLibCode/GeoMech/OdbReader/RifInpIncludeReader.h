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

#include <fstream>
#include <map>
#include <string>
#include <vector>

//==================================================================================================
//
//==================================================================================================
class RifInpIncludeReader
{
public:
    RifInpIncludeReader();
    ~RifInpIncludeReader();

    bool openFile( const std::string& fileName );
    bool isOpen() const;

    void readData( int column, const std::map<int, std::string>& parts, std::map<int, std::vector<double>>& data );

private:
    void close();

private:
    std::ifstream m_stream;
};
