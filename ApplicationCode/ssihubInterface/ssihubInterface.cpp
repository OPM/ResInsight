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


namespace ssihub {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Interface::Interface()
{
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
void Interface::setRegion(int east, int west, int north, int south)
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
    ssihub::WebServiceInterface ws;


    ws.setUrl(m_webServiceAddress);

    QMap<QString, QVariant> params;
    ws.fetchData("", params);

//         BasicAboutDialog dialog(NULL);
//     dialog.exec();

    return m_importedWellPathFiles;
}

}; // namespace ssihub