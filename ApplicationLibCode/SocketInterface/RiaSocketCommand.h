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

#pragma once

//////////////////////////////////////////////////////////////////////////
/// The base class for classes interpreting commands sent via socket.
/// Works in close connection with RiaSocketServer
//////////////////////////////////////////////////////////////////////////

class RiaSocketServer;

class QTcpSocket;
class QDataStream;
class QByteArray;

#include <QList>

class RiaSocketCommand
{
public:
    virtual ~RiaSocketCommand() {}

    /// This method is supposed to interpret the commands received from the calling socket connection and
    /// read the data currently available.
    /// If it read all the data and completed the command, it is supposed to return true. If it did not read all the
    /// data, it is supposed to return false. The socket server will then assume that the command is not completely
    /// interpreted, and will continue to call interpretMore() until that method returns true.

    virtual bool interpretCommand( RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream ) = 0;

    /// This method is supposed to read whatever more data that is available on the socket connection, and return true
    /// if it was able to read all the data. If not all the data was available, it must return false, so that the
    /// RiaSocketServer will call this method again when more data becomes available.
    virtual bool interpretMore( RiaSocketServer* server, QTcpSocket* currentClient ) { return true; }
};

#include "cafFactory.h"
typedef caf::Factory<RiaSocketCommand, QString> RiaSocketCommandFactory;
