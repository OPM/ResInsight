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

#include "RiaStdInclude.h"
#include "RiaSocketServer.h"
#include "RiaSocketCommand.h"

#include <QtGui>
#include <QtNetwork>

#include <stdlib.h>

#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RimReservoirView.h"
#include "RimProject.h"
#include "RimCase.h"

#include "RigCaseData.h"
#include "RigCaseCellResultsData.h"

#include "cafFactory.h"
#include "RigGridBase.h"


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSocketServer::RiaSocketServer(QObject* parent)
: QObject(parent),
  m_tcpServer(NULL),
  m_currentClient(NULL),
  m_currentCommandSize(0),
  m_currentCommand(NULL)
{
    m_errorMessageDialog = new QErrorMessage(RiuMainWindow::instance());

    // TCP server setup

    m_tcpServer = new QTcpServer(this);

    if (!m_tcpServer->listen(QHostAddress::LocalHost, 40001)) 
    {
        m_errorMessageDialog->showMessage("Octave communication disabled :\n"
                                          "\n"
                                          "This instance of ResInsight could not start the Socket Server enabling octave to get and set data.\n"
                                          "This is probably because you already have a running ResInsight process.\n"
                                          "Octave can only communicate with one ResInsight process at a time, so the Octave\n"
                                          "communication in this ResInsight instance will be disabled.\n"
                                          "\n"
                                          + tr("The error from the socket system is: %1.").arg(m_tcpServer->errorString()));
        return;
    }

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewClientConnection()));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSocketServer::~RiaSocketServer()
{
    assert (m_currentCommand == NULL);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned short RiaSocketServer::serverPort()
{
    if (m_tcpServer) return m_tcpServer->serverPort();
    else return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotNewClientConnection()
{
    // If we are currently handling a connection, just ignore the new one until the queue is empty.

    if (m_currentClient != NULL)
    {
        if (m_currentClient->state() == QAbstractSocket::ConnectedState)
        {
            return;
        }
        else
        {
            if (m_currentCommand)
            {
                if (m_currentCommand->interpretMore(this, m_currentClient))
                {
                    delete m_currentCommand;
                    m_currentCommand = NULL;
                }

                CVF_ASSERT(m_currentCommand == NULL);
            }

            terminateCurrentConnection();
        }
    }


    // Get the first pending connection, and set it as the current Client to handle

    QTcpSocket *newClient = m_tcpServer->nextPendingConnection();
    this->handleClientConnection(newClient);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::handleClientConnection(QTcpSocket* clientToHandle)
{
    CVF_ASSERT(clientToHandle != NULL);
    CVF_ASSERT(m_currentClient == NULL);
    CVF_ASSERT(m_currentCommand == NULL);

    m_currentClient = clientToHandle;
    m_currentCommandSize = 0;

    connect(m_currentClient, SIGNAL(disconnected()), this, SLOT(slotCurrentClientDisconnected()));

    if (m_currentClient->bytesAvailable())
    {
        this->readCommandFromOctave();
    }

    connect(m_currentClient, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

//--------------------------------------------------------------------------------------------------
/// Find the requested reservoir by caseId
//--------------------------------------------------------------------------------------------------
RimCase* RiaSocketServer::findReservoir(int caseId)
{
    if (caseId < 0)
    {
        if (RiaApplication::instance()->activeReservoirView())
        {
            return RiaApplication::instance()->activeReservoirView()->eclipseCase();
        }
    }
    else
    {
        RimProject* project =  RiaApplication::instance()->project();
        if (!project) return NULL;

        std::vector<RimCase*> cases;
        project->allCases(cases);

        for (size_t i = 0; i < cases.size(); i++)
        {
            if (cases[i]->caseId == caseId)
            {
                return cases[i];
            }
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::readCommandFromOctave()
{
    QDataStream socketStream(m_currentClient);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // If we have not read the currentCommandSize
    // read the size of the command if all the data is available
    if (m_currentCommandSize == 0) 
    {
        if (m_currentClient->bytesAvailable() < (int)sizeof(qint64)) return;

        socketStream >> m_currentCommandSize;
    }

    // Check if the complete command is available, return and whait for readyRead() if not
    if (m_currentClient->bytesAvailable() < m_currentCommandSize) return;

    // Now we can read the command

    QByteArray command = m_currentClient->read( m_currentCommandSize);
    QTextStream commandStream(command);

    QList<QByteArray> args;
    while (!commandStream.atEnd())
    {
        QByteArray arg;
        commandStream >> arg;
        args.push_back(arg);
    }

    CVF_ASSERT(args.size() > 0); 

    std::cout << args[0].data() << std::endl;

    m_currentCommand = RiaSocketCommandFactory::instance()->create(args[0]);

    if (m_currentCommand)
    {
        bool finished = m_currentCommand->interpretCommand(this, args, socketStream);
        if (finished)
        {
            delete m_currentCommand;
            m_currentCommand = NULL;
        }
    }
    else
    {
        // Todo: When all commands are into new shape, do the "unknown command" error output here.

        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Unknown command: %1").arg(args[0].data()));
        terminateCurrentConnection();
        return;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotCurrentClientDisconnected()
{
    if (m_currentCommand)
    {
        if (m_currentCommand->interpretMore(this, m_currentClient))
        {
            delete m_currentCommand;
            m_currentCommand = NULL;
        }

        /// What do we do here ?
        CVF_ASSERT(m_currentCommand == NULL);
    }

    terminateCurrentConnection();

    QTcpSocket *newClient = m_tcpServer->nextPendingConnection();

    if (newClient)
    {
        this->handleClientConnection(newClient);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::terminateCurrentConnection()
{
    if (m_currentClient)
    {
        m_currentClient->disconnect(SIGNAL(disconnected()));
        m_currentClient->disconnect(SIGNAL(readyRead()));
        m_currentClient->deleteLater();
        m_currentClient = NULL;
    }

    // Clean up more state:

    if (m_currentCommand)
    {
        delete m_currentCommand;
        m_currentCommand = NULL;
    }

    m_currentCommandSize = 0;

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotReadyRead()
{
    if (m_currentCommand)
    {
        if (m_currentCommand->interpretMore(this, m_currentClient))
        {
            delete m_currentCommand;
            m_currentCommand = NULL;
        }
    }
    else
    {
        readCommandFromOctave();
    }
}

