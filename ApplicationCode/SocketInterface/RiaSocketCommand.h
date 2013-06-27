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
/// Socket commands, to be moved into a separate file
//////////////////////////////////////////////////////////////////////////

class RiaSocketServer;
class QTcpSocket;

class RiaSocketCommand
{
public:
    virtual ~RiaSocketCommand() {}
    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) = 0;
    virtual bool interpretMore(RiaSocketServer* server, QTcpSocket* currentClient) { return true; }
};

#include "cafFactory.h"
typedef caf::Factory<RiaSocketCommand, QString> RiaSocketCommandFactory;


