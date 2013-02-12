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
#include <QAbstractSocket>

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
    enum ReadState {ReadingCommand, ReadingPropertyData};

public:
    RiaSocketServer(QObject *parent = 0);
    ~RiaSocketServer();
    unsigned short  serverPort();

private slots:
    void            slotNewClientConnection();
    void            slotCurrentClientDisconnected();
    void            slotReadyRead();

private:
    void            readCommandFromOctave();
    void            readPropertyDataFromOctave();


    void            handleClientConnection( QTcpSocket* clientToHandle);
    RimReservoir*   findReservoir(const QString &casename);
    void            terminateCurrentConnection();

    void            calculateMatrixModelActiveCellInfo(std::vector<qint32>& gridNumber,
        std::vector<qint32>& cellI,
        std::vector<qint32>& cellJ,
        std::vector<qint32>& cellK,
        std::vector<qint32>& parentGridNumber,
        std::vector<qint32>& hostCellI,
        std::vector<qint32>& hostCellJ,
        std::vector<qint32>& hostCellK);


private:
    QTcpServer*     m_tcpServer;
    QErrorMessage*  m_errorMessageDialog;

    QTcpSocket*     m_currentClient;
    qint64          m_currentCommandSize; ///< The size in bytes of the command we are currently reading.


    // Vars used for reading data from octave and adding them to the available results
    ReadState       m_readState;
    quint64         m_timeStepCountToRead;
    quint64         m_bytesPerTimeStepToRead;
    size_t          m_currentTimeStepToRead;
    std::vector< std::vector<double> >* m_scalarResultsToAdd;
    RimReservoir*   m_currentReservoir;
    size_t          m_currentScalarIndex;
    QString         m_currentPropertyName;
    bool            m_invalidActiveCellCountDetected;
};



