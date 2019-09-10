/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

class RimCase;
class RiaSocketServer;
class RimEclipseCase;
class QTcpSocket;

#include <QByteArray>
#include <QList>

#define PMonLog( MessageString ) RiuMainWindow::instance()->processMonitor()->addStringToLog( MessageString );

class RiaSocketTools
{
public:
    static RimEclipseCase* findCaseFromArgs( RiaSocketServer* server, const QList<QByteArray>& args );
    static void
        getCaseInfoFromCase( RimCase* rimCase, qint64& caseId, QString& caseName, QString& caseType, qint64& caseGroupId );

    static bool writeBlockData( RiaSocketServer* server, QTcpSocket* socket, const char* data, quint64 bytesToWrite );
};
