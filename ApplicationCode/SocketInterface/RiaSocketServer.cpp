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

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "RigCaseData.h"
#include "RigCaseCellResultsData.h"

#include "cafFactory.h"



class RiaSocketCommand
{
public:

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) = 0;
    virtual bool interpretMore(QDataStream& stream) {}

};

typedef caf::Factory<RiaSocketCommand, QString> RiaSocketCommandFactory;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void getCaseInfoFromCase(RimCase* rimCase, qint64& caseId, QString& caseName, QString& caseType, qint64& caseGroupId)
{
    CVF_ASSERT(rimCase);

    caseId = rimCase->caseId;
    caseName = rimCase->caseUserDescription;

    RimCaseCollection* caseCollection = rimCase->parentCaseCollection();
    if (caseCollection)
    {
        caseGroupId = caseCollection->parentCaseGroup()->groupId;

        if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollection))
        {
            caseType = "StatisticsCase";
        }
        else
        {
            caseType = "SourceCase";
        }
    }
    else
    {
        caseGroupId = -1;

        if (dynamic_cast<RimInputCase*>(rimCase))
        {
            caseType = "InputCase";
        }
        else
        {
            caseType = "ResultCase";
        }
    }
}

class RiaGetCurrentCase: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCurrentCase"); }
    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        qint64  caseId = -1;
        QString caseName;
        QString caseType;
        qint64  caseGroupId = -1;

        RimCase* rimCase = server->findReservoir(caseId);

        if (rimCase)
        {
            getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);
        }

        quint64 byteCount = 2*sizeof(qint64);
        byteCount += caseName.size()*sizeof(QChar);
        byteCount += caseType.size()*sizeof(QChar);

        socketStream << byteCount;

        socketStream << caseId;
        socketStream << caseName;
        socketStream << caseType;
        socketStream << caseGroupId;

        return true;

    }

};

static bool RiaGetCurrentCase_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCurrentCase>(RiaGetCurrentCase::commandName());


class RiaGetCaseGroups: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCaseGroups"); }
    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        if (RiaApplication::instance()->project())
        {
            std::vector<QString> groupNames;
            std::vector<qint64> groupIds;

            size_t caseGroupCount = RiaApplication::instance()->project()->caseGroups().size();
            quint64 byteCount = 0;

            for (size_t i = 0; i < caseGroupCount; i++)
            {
                RimIdenticalGridCaseGroup* cg = RiaApplication::instance()->project()->caseGroups()[i];

                QString caseGroupName = cg->name;
                qint64 caseGroupId = cg->groupId;

                byteCount += caseGroupName.size() * sizeof(QChar);
                byteCount += sizeof(qint64);

                groupNames.push_back(caseGroupName);
                groupIds.push_back(caseGroupId);
            }

            socketStream << (quint64)byteCount;
            socketStream << (quint64)caseGroupCount;

            for (size_t i = 0; i < caseGroupCount; i++)
            {
                socketStream << groupNames[i];
                socketStream << groupIds[i];
            }
        }
        else
        {
            // ERROR
        }

        return true;
    }

};

static bool RiaGetCaseGroups_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCaseGroups>(RiaGetCaseGroups::commandName());



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void getCaseInfoFromCases(std::vector<RimCase*>& cases, std::vector<qint64>& caseIds, std::vector<QString>& caseNames, std::vector<QString> &caseTypes, std::vector<qint64>& caseGroupIds)
{
    for (size_t i = 0; i < cases.size(); i++)
    {
        RimCase* rimCase = cases[i];

        qint64  caseId = -1;
        QString caseName;
        QString caseType;
        qint64  caseGroupId = -1;
        getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        caseIds.push_back(rimCase->caseId);
        caseNames.push_back(rimCase->caseUserDescription);
        caseTypes.push_back(caseType);
        caseGroupIds.push_back(caseGroupId);
    }
}


class RiaGetSelectedCases: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetSelectedCases"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
        if (ruiMainWindow)
        {
            std::vector<RimCase*> cases;
            ruiMainWindow->selectedCases(cases);

            std::vector<qint64>  caseIds;
            std::vector<QString> caseNames;
            std::vector<QString>  caseTypes;
            std::vector<qint64>  caseGroupIds;

            getCaseInfoFromCases(cases, caseIds, caseNames, caseTypes, caseGroupIds);

            quint64 byteCount = sizeof(quint64);
            quint64 selectionCount = caseIds.size();

            for (size_t i = 0; i < selectionCount; i++)
            {
                byteCount += 2*sizeof(qint64);
                byteCount += caseNames[i].size() * sizeof(QChar);
                byteCount += caseTypes[i].size() * sizeof(QChar);
            }

            socketStream << byteCount;
            socketStream << selectionCount;

            for (size_t i = 0; i < selectionCount; i++)
            {
                socketStream << caseIds[i];
                socketStream << caseNames[i];
                socketStream << caseTypes[i];
                socketStream << caseGroupIds[i];
            }
        }

        return true;
    }
};

static bool RiaGetSelectedCases_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetSelectedCases>(RiaGetSelectedCases::commandName());


class RiaGetCases: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCases"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        quint64 argCaseGroupId = -1;

        if (args.size() == 2)
        {
            argCaseGroupId = args[1].toInt();
        }

        if (RiaApplication::instance()->project())
        {
            RimProject* proj = RiaApplication::instance()->project();

            std::vector<RimCase*> cases;
            if (argCaseGroupId == -1)
            {
                proj->allCases(cases);
            }
            else
            {
                RimIdenticalGridCaseGroup* caseGroup = NULL;
                for (size_t i = 0; i < RiaApplication::instance()->project()->caseGroups().size(); i++)
                {
                    RimIdenticalGridCaseGroup* cg = RiaApplication::instance()->project()->caseGroups()[i];

                    if (argCaseGroupId == cg->groupId())
                    {
                        caseGroup = cg;
                    }
                }

                if (caseGroup)
                {
                    for (size_t i = 0; i < caseGroup->statisticsCaseCollection()->reservoirs.size(); i++)
                    {
                        cases.push_back(caseGroup->statisticsCaseCollection()->reservoirs[i]);
                    }

                    for (size_t i = 0; i < caseGroup->caseCollection()->reservoirs.size(); i++)
                    {
                        cases.push_back(caseGroup->caseCollection()->reservoirs[i]);
                    }
                }
            }


            std::vector<qint64>  caseIds;
            std::vector<QString> caseNames;
            std::vector<QString> caseTypes;
            std::vector<qint64>  caseGroupIds;

            getCaseInfoFromCases(cases, caseIds, caseNames, caseTypes, caseGroupIds);

            quint64 byteCount = sizeof(quint64);
            quint64 caseCount = caseIds.size();

            for (size_t i = 0; i < caseCount; i++)
            {
                byteCount += 2*sizeof(qint64);
                byteCount += caseNames[i].size() * sizeof(QChar);
                byteCount += caseTypes[i].size() * sizeof(QChar);
            }

            socketStream << byteCount;
            socketStream << caseCount;

            for (size_t i = 0; i < caseCount; i++)
            {
                socketStream << caseIds[i];
                socketStream << caseNames[i];
                socketStream << caseTypes[i];
                socketStream << caseGroupIds[i];
            }
        }

        return true;
    }
};

static bool RiaGetCases_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCases>(RiaGetCases::commandName());

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


    bool isGetProperty      = args[0] == "GetProperty";         // GetProperty [casename/index] PropertyName
    bool isSetProperty      = args[0] == "SetProperty";         // SetProperty [casename/index] PropertyName
    bool isGetCellInfo      = args[0] == "GetActiveCellInfo";   // GetActiveCellInfo [casename/index]
    bool isGetGridDim       = args[0] == "GetMainGridDimensions"; // GetMainGridDimensions [casename/index]

    if (!(isGetProperty || isSetProperty || isGetCellInfo || isGetGridDim  ))
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Unknown command: %1").arg(args[0].data()));
        terminateCurrentConnection();
        return;
    }

    int caseId = -1;
    QString propertyName;
    RimCase* rimCase = NULL;

    // Find the correct arguments

    if (isGetProperty || isSetProperty)
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
    else if (isGetCellInfo || isGetGridDim)
    {
        if (args.size() > 1)
        {
            caseId = args[1].toInt();
        }
    }

    rimCase = this->findReservoir(caseId);
    

    if (rimCase == NULL)
    {
        m_errorMessageDialog->showMessage(tr("ResInsight SocketServer: \n") + tr("Could not find the Case with CaseId : \"%1\"").arg(caseId));
        return;
    }

    if (isGetProperty || isSetProperty)
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
    else if (isGetCellInfo)
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;

        if (args.size() > 2)
        {
            QString prorosityModelString = args[2];
            if (prorosityModelString.toUpper() == "FRACTURE")
            {
                porosityModel = RifReaderInterface::FRACTURE_RESULTS;
            }
        }

        // Write data back to octave: columnCount, bytesPrTimestep, GridNr I J K ParentGridNr PI PJ PK CoarseBoxIdx

        caf::FixedArray<std::vector<qint32>, 9> activeCellInfo;
        if (!(rimCase && rimCase->reservoirData() && rimCase->reservoirData()->mainGrid()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return;
        }

        calculateMatrixModelActiveCellInfo(rimCase, porosityModel,
            activeCellInfo[0],
            activeCellInfo[1],
            activeCellInfo[2],
            activeCellInfo[3],
            activeCellInfo[4],
            activeCellInfo[5],
            activeCellInfo[6],
            activeCellInfo[7],
            activeCellInfo[8]);

        // First write column count
        quint64 columnCount = (quint64)9;
        socketStream << columnCount;

        // then the byte-size of the size of one column
        size_t  timestepResultCount = activeCellInfo[0].size();
        quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(qint32));
        socketStream << timestepByteCount;

        // Then write the data.

        for (size_t tIdx = 0; tIdx < columnCount; ++tIdx)
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

        if (rimCase && rimCase->reservoirData() && rimCase->reservoirData()->mainGrid())
        {
             iCount = rimCase->reservoirData()->mainGrid()->cellCountI();
             jCount = rimCase->reservoirData()->mainGrid()->cellCountJ();
             kCount = rimCase->reservoirData()->mainGrid()->cellCountK();
        }

        socketStream << (quint64)iCount << (quint64)jCount << (quint64)kCount;
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSocketServer::calculateMatrixModelActiveCellInfo(RimCase* reservoirCase, RifReaderInterface::PorosityModelResultType porosityModel, std::vector<qint32>& gridNumber, std::vector<qint32>& cellI, std::vector<qint32>& cellJ, std::vector<qint32>& cellK, std::vector<qint32>& parentGridNumber, std::vector<qint32>& hostCellI, std::vector<qint32>& hostCellJ, std::vector<qint32>& hostCellK, std::vector<qint32>& coarseBoxIdx)
{
    gridNumber.clear();
    cellI.clear();
    cellJ.clear();
    cellK.clear();
    parentGridNumber.clear();
    hostCellI.clear();
    hostCellJ.clear();
    hostCellK.clear();
    coarseBoxIdx.clear();

    if (!reservoirCase || !reservoirCase->reservoirData() || !reservoirCase->reservoirData()->mainGrid())
    {
        return;
    }

    RigActiveCellInfo* actCellInfo = reservoirCase->reservoirData()->activeCellInfo(porosityModel);
    size_t numMatrixModelActiveCells = actCellInfo->globalActiveCellCount();

    gridNumber.reserve(numMatrixModelActiveCells);
    cellI.reserve(numMatrixModelActiveCells);
    cellJ.reserve(numMatrixModelActiveCells);
    cellK.reserve(numMatrixModelActiveCells);
    parentGridNumber.reserve(numMatrixModelActiveCells);
    hostCellI.reserve(numMatrixModelActiveCells);
    hostCellJ.reserve(numMatrixModelActiveCells);
    hostCellK.reserve(numMatrixModelActiveCells);
    coarseBoxIdx.reserve(numMatrixModelActiveCells);

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
            cellI.push_back(static_cast<qint32>(i + 1));        // NB: 1-based index in Octave
            cellJ.push_back(static_cast<qint32>(j + 1));        // NB: 1-based index in Octave
            cellK.push_back(static_cast<qint32>(k + 1));        // NB: 1-based index in Octave
            
            parentGridNumber.push_back(static_cast<qint32>(parentGrid->gridIndex()));
            hostCellI.push_back(static_cast<qint32>(pi + 1));   // NB: 1-based index in Octave
            hostCellJ.push_back(static_cast<qint32>(pj + 1));   // NB: 1-based index in Octave
            hostCellK.push_back(static_cast<qint32>(pk + 1));   // NB: 1-based index in Octave

            // TODO: Handle coarse box concept
            coarseBoxIdx.push_back(-1);
        }
    }
}

