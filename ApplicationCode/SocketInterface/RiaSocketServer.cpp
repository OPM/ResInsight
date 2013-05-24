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
#include "RimReservoirCellResultsCacher.h"
#include "RimInputCase.h"
#include "RimInputProperty.h"
#include "RimInputPropertyCollection.h"

#include "RimUiTreeModelPdm.h"

#include "RimResultSlot.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellCollection.h"
#include "RimScriptCollection.h"
#include "RimCaseCollection.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

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
  m_scalarResultsToAdd(NULL),
  m_currentTimeStepToRead(0),
  m_currentReservoir(NULL),
  m_currentScalarIndex(cvf::UNDEFINED_SIZE_T),
  m_invalidActiveCellCountDetected(false),
  m_readState(ReadingCommand),
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
            if (m_readState == ReadingPropertyData)
            {
                readPropertyDataFromOctave();
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
    m_currentClient = clientToHandle;

    // Initialize state varianbles
    m_currentCommandSize = 0;
    m_scalarResultsToAdd = NULL;

    m_timeStepCountToRead = 0;
    m_bytesPerTimeStepToRead = 0;
    m_currentTimeStepToRead = 0;
    m_currentReservoir = NULL;
    m_currentScalarIndex = cvf::UNDEFINED_SIZE_T;
    m_currentPropertyName = "";

    connect(m_currentClient, SIGNAL(disconnected()), this, SLOT(slotCurrentClientDisconnected()));
    m_readState = ReadingCommand;
    m_currentCommand = NULL;

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


    bool isSetProperty      = args[0] == "SetProperty";         // SetProperty [casename/index] PropertyName

    if (!( isSetProperty  ))
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Unknown command: %1").arg(args[0].data()));
        terminateCurrentConnection();
        return;
    }

    int caseId = -1;
    QString propertyName;
    RimCase* rimCase = NULL;

    // Find the correct arguments

    if (isSetProperty)
    {
        if (args.size() == 2)
        {
            propertyName = args[1];
        }
        else if (args.size() > 2)
        {
            caseId = args[1].toInt();
            propertyName = args[2];
        }
    }


    rimCase = this->findReservoir(caseId);
    

    if (rimCase == NULL)
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not find the Case with CaseId : \"%1\"").arg(caseId));
        return;
    }

    if (isSetProperty)
    {
        // Find the requested data, Or create a set if we are setting data and it is not found

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (rimCase && rimCase->results(RifReaderInterface::MATRIX_RESULTS))
        {
            scalarResultIndex = rimCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(propertyName);

            if (scalarResultIndex == cvf::UNDEFINED_SIZE_T && isSetProperty)
            {
                scalarResultIndex = rimCase->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->addEmptyScalarResult(RimDefines::GENERATED, propertyName, true);
            }

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(rimCase->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->cellScalarResults(scalarResultIndex));
                m_currentScalarIndex = scalarResultIndex;
                m_currentPropertyName = propertyName;
            }

        }

        if (scalarResultFrames == NULL)
        {
            m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not find the property named: \"%1\"").arg(propertyName));
        }

     // Set property
        {
            m_readState = ReadingPropertyData;

            // Disconnect the socket from calling this slot again.
            m_currentReservoir = rimCase;

            if ( scalarResultFrames != NULL)
            {
                m_scalarResultsToAdd = scalarResultFrames;
                if (m_currentClient->bytesAvailable())
                {
                    this->readPropertyDataFromOctave();
                }
            }
        }
    }


    }
}

//--------------------------------------------------------------------------------------------------
/// This method reads data from octave and puts it into the resInsight Structures
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::readPropertyDataFromOctave()
{
    QDataStream socketStream(m_currentClient);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // If we have not read the header and there are data enough: Read it.
    // Do nothing if we have not enough data

    if (m_timeStepCountToRead == 0 || m_bytesPerTimeStepToRead == 0)
    {
        if (m_currentClient->bytesAvailable() < (int)sizeof(quint64)*2) return;

        socketStream >> m_timeStepCountToRead;
        socketStream >> m_bytesPerTimeStepToRead;
    }

    // If nothing should be read, or we already have read everything, do nothing

    if ((m_timeStepCountToRead == 0) || (m_currentTimeStepToRead >= m_timeStepCountToRead) )  return;

    // Check if a complete timestep is available, return and whait for readyRead() if not
    if (m_currentClient->bytesAvailable() < (int)m_bytesPerTimeStepToRead) return;

    size_t  cellCountFromOctave = m_bytesPerTimeStepToRead / sizeof(double);

    size_t gridActiveCellCount = m_currentReservoir->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->globalActiveCellCount();
    size_t gridTotalCellCount = m_currentReservoir->reservoirData()->mainGrid()->cellCount();

    if (cellCountFromOctave != gridActiveCellCount && cellCountFromOctave != gridTotalCellCount)
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + 
            tr("The number of cells in the data coming from octave does not match the case") + ":\""  + m_currentReservoir->caseUserDescription() + "\"\n"
            "   Octave: " + QString::number(cellCountFromOctave) + "\n"
            "  " + m_currentReservoir->caseUserDescription() + ": Active cell count: " + QString::number(gridActiveCellCount) + " Total cell count: " +  QString::number(gridTotalCellCount)) ;

        cellCountFromOctave = 0;
        m_invalidActiveCellCountDetected = true;
        m_currentClient->abort();

        return;
    }

    // Make sure the size of the retreiving container is correct.
    // If it is, this is noops
    m_scalarResultsToAdd->resize(m_timeStepCountToRead);
    for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
    {
        m_scalarResultsToAdd->at(tIdx).resize(cellCountFromOctave, HUGE_VAL);
    }

    // Read available complete timestepdata
    while ((m_currentClient->bytesAvailable() >= (int)m_bytesPerTimeStepToRead) && (m_currentTimeStepToRead < m_timeStepCountToRead))
    {
        qint64 bytesRead = 0;
        double * internalMatrixData = m_scalarResultsToAdd->at(m_currentTimeStepToRead).data();

#if 1 // Use raw data transfer. Faster.
        bytesRead = m_currentClient->read((char*)(internalMatrixData), m_bytesPerTimeStepToRead);
#else
        for (size_t cIdx = 0; cIdx < cellCountFromOctave; ++cIdx)
        {
            socketStream >> internalMatrixData[cIdx];

            if (socketStream.status() == QDataStream::Ok) bytesRead += sizeof(double);
        }
#endif

        if ((int)m_bytesPerTimeStepToRead != bytesRead)
        {
            m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not read binary double data properly from socket"));
        }

        ++m_currentTimeStepToRead;
    }

    // If we have read all the data, refresh the views
    if (m_currentTimeStepToRead == m_timeStepCountToRead)
    {
        if (m_currentReservoir != NULL)
        {
            // Create a new input property if we have an input reservoir
            RimInputCase* inputRes = dynamic_cast<RimInputCase*>(m_currentReservoir);
            if (inputRes)
            {
                RimInputProperty* inputProperty = NULL;
                inputProperty = inputRes->m_inputPropertyCollection->findInputProperty(m_currentPropertyName);
                if (!inputProperty)
                {
                    inputProperty = new RimInputProperty;
                    inputProperty->resultName = m_currentPropertyName;
                    inputProperty->eclipseKeyword = "";
                    inputProperty->fileName = "";
                    inputRes->m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                    RimUiTreeModelPdm* treeModel = RiuMainWindow::instance()->uiPdmModel();
                    treeModel->updateUiSubTree(inputRes->m_inputPropertyCollection());
                }
                inputProperty->resolvedState = RimInputProperty::RESOLVED_NOT_SAVED;
            }

            if( m_currentScalarIndex != cvf::UNDEFINED_SIZE_T &&
                m_currentReservoir->reservoirData() && 
                m_currentReservoir->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS) )
            {
                m_currentReservoir->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->recalculateMinMax(m_currentScalarIndex);
            }

            for (size_t i = 0; i < m_currentReservoir->reservoirViews.size(); ++i)
            {
                if (m_currentReservoir->reservoirViews[i])
                {
                    m_currentReservoir->reservoirViews[i]->updateCurrentTimeStepAndRedraw();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotCurrentClientDisconnected()
{
    if (m_timeStepCountToRead > 0
        && m_currentTimeStepToRead < m_timeStepCountToRead
        && m_currentClient->bytesAvailable()
        && !m_invalidActiveCellCountDetected)
    {
        this->readPropertyDataFromOctave();
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

    m_currentCommandSize = 0;
    m_timeStepCountToRead = 0;
    m_bytesPerTimeStepToRead = 0;
    m_currentTimeStepToRead = 0;
    m_scalarResultsToAdd = NULL;
    m_currentReservoir = NULL;
    m_currentScalarIndex = cvf::UNDEFINED_SIZE_T;
    m_currentPropertyName = "";
    m_invalidActiveCellCountDetected = false;

    m_readState = ReadingCommand;

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::slotReadyRead()
{
    switch (m_readState)
    {
        case ReadingCommand :
        {
            readCommandFromOctave();
            break;
        }

        case ReadingPropertyData :
        {
            readPropertyDataFromOctave();
            break;
        }

        default:
            CVF_ASSERT(false);
            break;
    }
}

