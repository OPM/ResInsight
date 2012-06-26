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

#include <QDialog>

class QLabel;
class QPushButton;
class QTcpServer;
class QTcpSocket;
class QNetworkSession;
class QErrorMessage;
class RimReservoir;

class RiaSocketServer : public QObject
{
    Q_OBJECT

public:
    RiaSocketServer(QObject *parent = 0);
    ~RiaSocketServer();
    unsigned short  serverPort();

private slots:
    void            onNewClientConnection();
    void            slotReadCommand();
    void            slotReadPropertyData();
    void            slotCurrentClientDisconnected();

private:
    void            handleClientConnection( QTcpSocket* clientToHandle);
    RimReservoir*   findReservoir(const QString &casename);

private:
    QTcpServer*     m_tcpServer;
    QErrorMessage*  m_errorMessageDialog;

    QTcpSocket*     m_currentClient;
    qint64          m_currentCommandSize; ///< The size in bytes of the command we are currently reading.


    // Vars used for reading data from octave and adding them to the available results
    quint64         m_timeStepCountToRead;
    quint64         m_bytesPerTimeStepToRead;
    size_t          m_currentTimeStepToRead;
    std::vector< std::vector<double> >* m_scalarResultsToAdd;
    RimReservoir*   m_currentReservoir;
    size_t          m_currentScalarIndex;
    QString         m_currentPropertyName;
    bool            m_invalidActiveCellCountDetected;
};



