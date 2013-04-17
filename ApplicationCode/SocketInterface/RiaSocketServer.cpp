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

#include <QtGui>
#include <QtNetwork>

#include <stdlib.h>

#include "RiaSocketServer.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RimCase.h"
#include "RigCaseData.h"
#include "RigCaseCellResultsData.h"
#include "RimInputProperty.h"
#include "RimInputCase.h"
#include "RimUiTreeModelPdm.h"

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
  m_readState(ReadingCommand)
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

    if (m_currentClient->bytesAvailable())
    {
        this->readCommandFromOctave();
    }

    connect(m_currentClient, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

//--------------------------------------------------------------------------------------------------
/// Find the requested reservoir: Current, by index or by name
//--------------------------------------------------------------------------------------------------
RimCase* RiaSocketServer::findReservoir(const QString& caseName)
{
    if (caseName.isEmpty())
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

       bool isInt = false;
       int caseIndex = caseName.toInt(&isInt); // Needs more checking. Case names could start with number and what does Qt do then ?

       if (isInt)
       {
           if ((int)(project->reservoirs.size()) > caseIndex && project->reservoirs[caseIndex])
           {
               return project->reservoirs[caseIndex];
           }
       }
       else
       {
           for (size_t cIdx = 0; cIdx < project->reservoirs.size(); ++cIdx)
           {
               if (project->reservoirs[cIdx] && project->reservoirs[cIdx]->caseUserDescription() == caseName )
               {
                   return project->reservoirs[cIdx];
               }
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
    socketStream.setVersion(QDataStream::Qt_4_0);

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


    bool isGetProperty = args[0] == "GetProperty"; // GetProperty [casename/index] PropertyName
    bool isSetProperty = args[0] == "SetProperty"; // SetProperty [casename/index] PropertyName
    bool isGetCellInfo = args[0] == "GetActiveCellInfo"; // GetActiveCellInfo [casename/index]
    bool isGetGridDim  = args[0] == "GetMainGridDimensions"; // GetMainGridDimensions [casename/index]


    if (!(isGetProperty || isSetProperty || isGetCellInfo || isGetGridDim))
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Unknown command: %1").arg(args[0].data()));
        terminateCurrentConnection();
        return;
    }

    QString caseName;
    QString propertyName;
    RimCase* reservoir = NULL;

    // Find the correct arguments

    if (isGetProperty || isSetProperty)
    {
        if (args.size() == 2)
        {
            propertyName = args[1];
        }
        else if (args.size() > 2)
        {
            caseName = args[1];
            propertyName = args[2];
        }
    }
    else if (isGetCellInfo || isGetGridDim)
    {
        if (args.size() > 1)
        {
            caseName = args[1];
        }
    }

    reservoir = this->findReservoir(caseName);

    if (reservoir == NULL)
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not find the eclipse case with name or index: \"%1\"").arg(caseName));
        return;
    }

    if (isGetProperty || isSetProperty)
    {
        // Find the requested data, Or create a set if we are setting data and it is not found

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (reservoir && reservoir->results(RifReaderInterface::MATRIX_RESULTS))
        {
            scalarResultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(propertyName);

            if (scalarResultIndex == cvf::UNDEFINED_SIZE_T && isSetProperty)
            {
                scalarResultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->addEmptyScalarResult(RimDefines::GENERATED, propertyName, true);
            }

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(reservoir->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->cellScalarResults(scalarResultIndex));
                m_currentScalarIndex = scalarResultIndex;
                m_currentPropertyName = propertyName;
            }

        }

        if (scalarResultFrames == NULL)
        {
            m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not find the property named: \"%1\"").arg(propertyName));
        }

        if (isGetProperty )
        {
            // Write data back : timeStepCount, bytesPrTimestep, dataForTimestep0 ... dataForTimestepN

            if ( scalarResultFrames == NULL)
            {
                // No data available
                socketStream << (quint64)0 << (quint64)0 ;
            }
            else
            {
                // First write timestep count
                quint64 timestepCount = (quint64)scalarResultFrames->size();
                socketStream << timestepCount;

                // then the byte-size of the result values in one timestep
                size_t  timestepResultCount = scalarResultFrames->front().size();
                quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(double));
                socketStream << timestepByteCount ;

                // Then write the data.

                for (size_t tIdx = 0; tIdx < scalarResultFrames->size(); ++tIdx)
                {
#if 1 // Write data as raw bytes, fast but does not handle byteswapping
                    m_currentClient->write((const char *)scalarResultFrames->at(tIdx).data(), timestepByteCount); // Raw print of data. Fast but no platform conversion
#else  // Write data using QDataStream, does byteswapping for us. Must use QDataStream on client as well
                    for (size_t cIdx = 0; cIdx < scalarResultFrames->at(tIdx).size(); ++cIdx)
                    {
                        socketStream << scalarResultFrames->at(tIdx)[cIdx];
                    }
#endif
                }
            }
        }
        else // Set property
        {
            m_readState = ReadingPropertyData;

            // Disconnect the socket from calling this slot again.
            m_currentReservoir = reservoir;

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
    else if (isGetCellInfo )
    {
        // Write data back to octave: columnCount, bytesPrTimestep, GridNr I J K ParentGridNr PI PJ PK

        caf::FixedArray<std::vector<qint32>, 8> activeCellInfo;
        if (!(reservoir && reservoir->reservoirData() && reservoir->reservoirData()->mainGrid()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return;
        }

        calculateMatrixModelActiveCellInfo(reservoir, 
            activeCellInfo[0],
            activeCellInfo[1],
            activeCellInfo[2],
            activeCellInfo[3],
            activeCellInfo[4],
            activeCellInfo[5],
            activeCellInfo[6],
            activeCellInfo[7]);

        // First write timestep count
        quint64 timestepCount = (quint64)8;
        socketStream << timestepCount;

        // then the byte-size of the result values in one timestep
        size_t  timestepResultCount = activeCellInfo[0].size();
        quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(qint32));
        socketStream << timestepByteCount ;

        // Then write the data.

        for (size_t tIdx = 0; tIdx < 8; ++tIdx)
        {
#if 1 // Write data as raw bytes, fast but does not handle byteswapping
            m_currentClient->write((const char *)activeCellInfo[tIdx].data(), timestepByteCount);
#else  // Write data using QDataStream, does byteswapping for us. Must use QDataStream on client as well
            for (size_t cIdx = 0; cIdx < activeCellInfo[tIdx].size(); ++cIdx)
            {
                socketStream << activeCellInfo[tIdx][cIdx];
            }
#endif
        }
    }
    else if (isGetGridDim)
    {
        // Write data back to octave: I, J, K dimensions

        size_t iCount = 0;
        size_t jCount = 0;
        size_t kCount = 0;

        if (reservoir && reservoir->reservoirData() && reservoir->reservoirData()->mainGrid())
        {
             iCount = reservoir->reservoirData()->mainGrid()->cellCountI();
             jCount = reservoir->reservoirData()->mainGrid()->cellCountJ();
             kCount = reservoir->reservoirData()->mainGrid()->cellCountK();
        }

        socketStream << (quint64)iCount << (quint64)jCount << (quint64)kCount;
    }
}

//--------------------------------------------------------------------------------------------------
/// This method reads data from octave and puts it into the resInsight Structures
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::readPropertyDataFromOctave()
{
    QDataStream socketStream(m_currentClient);
    socketStream.setVersion(QDataStream::Qt_4_0);

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
                    treeModel->rebuildUiSubTree(inputRes->m_inputPropertyCollection());
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::calculateMatrixModelActiveCellInfo(RimCase* reservoirCase, std::vector<qint32>& gridNumber, std::vector<qint32>& cellI, std::vector<qint32>& cellJ, std::vector<qint32>& cellK, std::vector<qint32>& parentGridNumber, std::vector<qint32>& hostCellI, std::vector<qint32>& hostCellJ, std::vector<qint32>& hostCellK)
{
    gridNumber.clear();
    cellI.clear();
    cellJ.clear();
    cellK.clear();
    parentGridNumber.clear();
    hostCellI.clear();
    hostCellJ.clear();
    hostCellK.clear();

    if (!reservoirCase || !reservoirCase->reservoirData() || !reservoirCase->reservoirData()->mainGrid())
    {
        return;
    }

    RigActiveCellInfo* actCellInfo = reservoirCase->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS);
    size_t numMatrixModelActiveCells = actCellInfo->globalActiveCellCount();

    gridNumber.reserve(numMatrixModelActiveCells);
    cellI.reserve(numMatrixModelActiveCells);
    cellJ.reserve(numMatrixModelActiveCells);
    cellK.reserve(numMatrixModelActiveCells);
    parentGridNumber.reserve(numMatrixModelActiveCells);
    hostCellI.reserve(numMatrixModelActiveCells);
    hostCellJ.reserve(numMatrixModelActiveCells);
    hostCellK.reserve(numMatrixModelActiveCells);

    const std::vector<RigCell>& globalCells = reservoirCase->reservoirData()->mainGrid()->cells();

    for (size_t cIdx = 0; cIdx < globalCells.size(); ++cIdx)
    {
        if (actCellInfo->isActive(cIdx))
        {
            RigGridBase* grid = globalCells[cIdx].hostGrid();
            CVF_ASSERT(grid != NULL);
            size_t cellIndex = globalCells[cIdx].cellIndex();

            size_t i, j, k;
            grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

            size_t pi, pj, pk;
            RigGridBase* parentGrid = NULL;

            if (grid->isMainGrid())
            {
                pi = i;
                pj = j;
                pk = k;
                parentGrid = grid;
            }
            else
            {
                size_t parentCellIdx = globalCells[cIdx].parentCellIndex();
                parentGrid = (static_cast<RigLocalGrid*>(grid))->parentGrid();
                CVF_ASSERT(parentGrid != NULL);
                parentGrid->ijkFromCellIndex(parentCellIdx, &pi, &pj, &pk);
            }

            gridNumber.push_back(static_cast<qint32>(grid->gridIndex()));
            cellI.push_back(static_cast<qint32>(i));
            cellJ.push_back(static_cast<qint32>(j));
            cellK.push_back(static_cast<qint32>(k));
            parentGridNumber.push_back(static_cast<qint32>(parentGrid->gridIndex()));
            hostCellI.push_back(static_cast<qint32>(pi));
            hostCellJ.push_back(static_cast<qint32>(pj));
            hostCellK.push_back(static_cast<qint32>(pk));
        }
    }
}

