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

#include "RiaSocketServer.h"
#include "RiaSocketCommand.h"

#include "RiaApplication.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimCase.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafFactory.h"

#include <QtGui>
#include <QtNetwork>
#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QErrorMessage>
#include <QMdiSubWindow>
#endif

#include <stdlib.h>


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSocketServer::RiaSocketServer(QObject* parent)
: QObject(parent),
  m_tcpServer(NULL),
  m_currentClient(NULL),
  m_currentCommandSize(0),
  m_currentCommand(NULL),
  m_currentCaseId(-1)
{
    m_errorMessageDialog = new QErrorMessage(RiuMainWindow::instance());

    // TCP server setup

    m_tcpServer = new QTcpServer(this);

    m_nextPendingConnectionTimer = new QTimer(this);
    m_nextPendingConnectionTimer->setInterval(100);
    m_nextPendingConnectionTimer->setSingleShot(true);

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

    connect(m_nextPendingConnectionTimer, SIGNAL(timeout()), this, SLOT(slotNewClientConnection()));
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
    // If we are currently handling a connection, just ignore the new one until the current one is disconnected. 

    if (m_currentClient && (m_currentClient->state() != QAbstractSocket::UnconnectedState) )
    {
        //PMonLog("Starting Timer");
        m_nextPendingConnectionTimer->start(); // Reset and start again
        return;
    }

    // Read pending data from socket

    if (m_currentClient && m_currentCommand)
    {
        bool isFinshed = m_currentCommand->interpretMore(this, m_currentClient);

        if (!isFinshed)
        {
            m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Warning : The command did not finish up correctly at the presence of a new one."));
        }
    }

    handleNextPendingConnection();
}

//--------------------------------------------------------------------------------------------------
/// Find the requested reservoir by caseId
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RiaSocketServer::findReservoir(int caseId)
{
    int currCaseId = caseId;
    if (caseId < 0)
    {
        currCaseId = this->currentCaseId();
    }

    if (currCaseId < 0)
    {
        RimEclipseView* riv = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (riv)
        {
            return riv->eclipseCase();
        }

        // If the active mdi window is different from an Eclipse view, search through available mdi windows to find the last activated
        // Eclipse view. The sub windows are returned with the most recent activated window at the back.
        QList<QMdiSubWindow*> subWindows = RiuMainWindow::instance()->subWindowList(QMdiArea::ActivationHistoryOrder);
		for (int i = subWindows.size() - 1; i > -1; i--)
		{
			RiuViewer* viewer = subWindows[i]->widget()->findChild<RiuViewer*>();
			if (viewer)
			{
                RimEclipseView* riv = dynamic_cast<RimEclipseView*>(viewer->ownerReservoirView());
                if (riv)
                {
                    return riv->eclipseCase();
                }
			}
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
            if (cases[i]->caseId == currCaseId)
            {
                return dynamic_cast<RimEclipseCase*>(cases[i]);
            }
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// Reads the command name size, the command string and creates a new command object based on the string read.
/// Tries to interpret the command as well.
/// Returns whether the command actually was completely finished in one go.
//--------------------------------------------------------------------------------------------------
bool RiaSocketServer::readCommandFromOctave()
{
    QDataStream socketStream(m_currentClient);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // If we have not read the currentCommandSize
    // read the size of the command if all the data is available

    if (m_currentCommandSize == 0) 
    {
        if (m_currentClient->bytesAvailable() < (int)sizeof(qint64)) return false;

        socketStream >> m_currentCommandSize;
    }

    // Check if the complete command is available, return and whait for readyRead() if not
    if (m_currentClient->bytesAvailable() < m_currentCommandSize) return false;

    // Now we can read the command name

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

    // Create the actual RiaSocketCommand object that will interpret the socket data

    m_currentCommand = RiaSocketCommandFactory::instance()->create(args[0]);

    if (m_currentCommand)
    {
        bool finished = m_currentCommand->interpretCommand(this, args, socketStream);
        return finished;
    }
    else
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Unknown command: %1").arg(args[0].data()));
        return true;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotCurrentClientDisconnected()
{
    if (m_currentCommand)
    {
        // Make sure we read what can be read.
        bool isFinished = m_currentCommand->interpretMore(this, m_currentClient);

        if (!isFinished)
        {
            m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Warning : The command was interrupted and did not finish because the connection to octave disconnected."));
        }
    }

    handleNextPendingConnection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotReadyRead()
{
    if (m_currentCommand)
    {
        bool isFinished = m_currentCommand->interpretMore(this, m_currentClient);

        if (isFinished)
        {
            handleNextPendingConnection();
        }
    }
    else
    {
        bool isFinished = readCommandFromOctave();
        if (isFinished)
        {
            handleNextPendingConnection();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::setCurrentCaseId(int caseId)
{
    m_currentCaseId = caseId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiaSocketServer::currentCaseId() const
{
    return m_currentCaseId;
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
void RiaSocketServer::handleNextPendingConnection()
{
    if (m_currentClient && (m_currentClient->state() != QAbstractSocket::UnconnectedState) )
    {
        //PMonLog("Starting Timer");
        m_nextPendingConnectionTimer->start(); // Reset and start again
        return;
    }

    // Stop timer
    if (m_nextPendingConnectionTimer->isActive())
    {    
        //PMonLog("Stopping Timer"); 
        m_nextPendingConnectionTimer->stop();
    }

    terminateCurrentConnection();

    QTcpSocket* clientToHandle = m_tcpServer->nextPendingConnection();
    if (clientToHandle)
    {
        CVF_ASSERT(m_currentClient == NULL);
        CVF_ASSERT(m_currentCommand == NULL);

        m_currentClient = clientToHandle;
        m_currentCommandSize = 0;

        connect(m_currentClient, SIGNAL(disconnected()), this, SLOT(slotCurrentClientDisconnected()));
        connect(m_currentClient, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

        if (m_currentClient->bytesAvailable())
        {
            bool isFinished = this->readCommandFromOctave();
            if (isFinished)
            {
                // Call ourselves recursively until there are none left, or until it can not be processed in one go.
                this->handleNextPendingConnection();
            }
        }
    }
}

