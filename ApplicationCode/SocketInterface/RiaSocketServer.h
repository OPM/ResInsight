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

#include <QObject>
#include <vector>
#include <QDataStream>

class QLabel;
class QPushButton;
class QTcpServer;
class QTcpSocket;
class QNetworkSession;
class QErrorMessage;
class QTimer;
class RimEclipseCase;
class RiaSocketCommand;

namespace riOctavePlugin
{
    const int qtDataStreamVersion = QDataStream::Qt_4_0;
}

class RiaSocketServer : public QObject
{
    Q_OBJECT

public:
    enum ReadState {ReadingCommand, ReadingPropertyData};

public:
    explicit RiaSocketServer(QObject *parent = 0);
    ~RiaSocketServer();

    unsigned short      serverPort();
    RimEclipseCase*            findReservoir(int caseId);
    QErrorMessage*      errorMessageDialog() { return m_errorMessageDialog; }
    QTcpSocket*         currentClient() { return m_currentClient; }

    void                setCurrentCaseId(int caseId);
    int                 currentCaseId() const;

private slots:
    void                slotNewClientConnection();
    void                slotCurrentClientDisconnected();
    void                slotReadyRead();

private:
    void                handleNextPendingConnection();
    void                terminateCurrentConnection();
    bool                readCommandFromOctave();

private:
    QTcpServer*         m_tcpServer;
    QErrorMessage*      m_errorMessageDialog;

    QTcpSocket*         m_currentClient;
    qint64              m_currentCommandSize; ///< The size in bytes of the command we are currently reading.

    RiaSocketCommand*   m_currentCommand;

    QTimer*             m_nextPendingConnectionTimer;

    int                 m_currentCaseId;    // Set to -1 to use default server behavior
};
