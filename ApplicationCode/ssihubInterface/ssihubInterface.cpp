/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "ssihubInterface.h"
#include "ssihubDialog.h"
#include "ssihubWebServiceInterface.h"

#include <math.h>

namespace ssihub {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Interface::Interface()
{
    m_north = HUGE_VAL;
    m_south = HUGE_VAL;
    m_east = HUGE_VAL;
    m_west = HUGE_VAL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Interface::setWebServiceAddress(const QString wsAdress)
{
    m_webServiceAddress = wsAdress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Interface::setJsonDestinationFolder(const QString folder)
{
    m_jsonDestinationFolder = folder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Interface::setRegion(int north, int south, int east, int west)
{
    m_east = east;
    m_west = west;
    m_north = north;
    m_south = south;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList Interface::jsonWellPaths()
{
    FetchWellPathsDialog fetchWellPaths;
    fetchWellPaths.setSsiHubUrl(m_webServiceAddress);
    fetchWellPaths.setDestinationFolder(m_jsonDestinationFolder);
    fetchWellPaths.setRegion(m_north, m_south, m_east, m_west);

    QStringList importedWellPathFiles;
    if (fetchWellPaths.exec() == QDialog::Accepted)
    {
        importedWellPathFiles = fetchWellPaths.downloadedJsonWellPathFiles();
    }
    
    return importedWellPathFiles;
}

}; // namespace ssihub