/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include <QStringList>
#include <QTcpSocket>

//==================================================================================================
///  Utility class used for transfer of data using QTcpSocket
///
///  As the compile configuration for octave plugins is quite complex,
//   the octave plugins includes the cpp-file to be able to compile only one file per plugin
//==================================================================================================
class RiaSocketDataTransfer
{
public:
    static size_t maximumValueCountInBlock();

public:
    static bool
                writeBlockDataToSocket( QTcpSocket* socket, const char* data, quint64 bytesToWrite, QStringList& errorMessages );
    static bool readBlockDataFromSocket( QTcpSocket* socket, char* data, quint64 bytesToRead, QStringList& errorMessages );
};
