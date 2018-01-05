/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RiaSocketCommand.h"

#include "RiaSocketServer.h"
#include "RiaSocketDataTransfer.h"
#include "RiaSocketTools.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"

#include <QTcpSocket>
#include <QErrorMessage>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetNNCConnections: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetNNCConnections"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        // Write data back to octave: columnCount, GridNr I J K GridNr I J K
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0;
            return true;
        }

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();

        size_t connectionCount = mainGrid->nncData()->connections().size();

        socketStream << (quint64)connectionCount;

        for (const RigConnection& connection : mainGrid->nncData()->connections())
        {
            const RigCell& cell1 = mainGrid->globalCellArray()[connection.m_c1GlobIdx];
            const RigCell& cell2 = mainGrid->globalCellArray()[connection.m_c2GlobIdx];

            sendCellInfo(socketStream, cell1);
            sendCellInfo(socketStream, cell2);
        }

        return true;
    }

    static void sendCellInfo(QDataStream& socketStream, const RigCell& cell)
    {
        RigGridBase* hostGrid = cell.hostGrid();
        size_t gridLocalCellIndex = cell.gridLocalCellIndex();
        size_t i, j, k;
        hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        socketStream << (qint32)hostGrid->gridIndex();
        socketStream << (qint32)(i+1) << (qint32)(j+1) << (qint32)(k+1);
    }
};

static bool RiaGetNNCConnections_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetNNCConnections>(RiaGetNNCConnections::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetDynamicNNCValues: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetDynamicNNCValues"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        // Write data back to octave: connectionCount, timeStepCount, property values
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0;
            return true;
        }

        QString propertyName = args[2];

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();
        const std::vector< std::vector<double> >* nncValues = mainGrid->nncData()->dynamicConnectionScalarResultByName(propertyName);

        if (nncValues == nullptr)
        {
            socketStream << (quint64)0 << (quint64)0;
            return true;
        }

        std::vector<size_t> requestedTimeSteps;
        if (args.size() > 3)
        {
            bool timeStepReadError = false;
            for (int argIdx = 3; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    requestedTimeSteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetDynamicNNCValues : \n") + RiaSocketServer::tr("An error occurred while interpreting the requested time steps."));
            }
        }
        else
        {
            for (size_t timeStep = 0; timeStep < nncValues->size(); ++timeStep)
            {
                requestedTimeSteps.push_back(timeStep);
            }
        }

        // then the connection count and time step count.
        size_t connectionCount = mainGrid->nncData()->connections().size();
        size_t timeStepCount = requestedTimeSteps.size();

        socketStream << (quint64)connectionCount;
        socketStream << (quint64)timeStepCount;

        for (size_t timeStep : requestedTimeSteps)
        {
            const std::vector<double>& timeStepValues = nncValues->at(timeStep);
            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)timeStepValues.data(), sizeof(double) * timeStepValues.size());
        }

        return true;
    }
};

static bool RiaGetDynamicNNCValues_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetDynamicNNCValues>(RiaGetDynamicNNCValues::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetStaticNNCValues: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetStaticNNCValues"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        QString propertyName = args[2];

        // Write data back to octave: connectionCount, property values
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0;
            return true;
        }

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();
        const std::vector<double>* nncValues = mainGrid->nncData()->staticConnectionScalarResultByName(propertyName);

        if (nncValues == nullptr)
        {
            socketStream << (quint64)0;
            return true;
        }

        // connection count
        size_t connectionCount = mainGrid->nncData()->connections().size();
        socketStream << (quint64)connectionCount;

        RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)nncValues->data(), sizeof(double) * nncValues->size());

        return true;
    }
};

static bool RiaGetStaticNNCValues_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetStaticNNCValues>(RiaGetStaticNNCValues::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetNNCPropertyNames: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetNNCPropertyNames"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0;
            return true;
        }

        RigNNCData* nncData = rimCase->eclipseCaseData()->mainGrid()->nncData();

        std::vector<QString> propertyTypes;
        std::vector<QString> propertyNames;

        std::vector<RigNNCData::NNCResultType> resultTypes;
        std::vector<QString> resultTypeNames;

        resultTypes.push_back(RigNNCData::NNC_DYNAMIC);
        resultTypeNames.push_back("DynamicNative");
        resultTypes.push_back(RigNNCData::NNC_STATIC);
        resultTypeNames.push_back("StaticNative");
        resultTypes.push_back(RigNNCData::NNC_GENERATED);
        resultTypeNames.push_back("Generated");

        for (size_t rtIdx = 0; rtIdx < resultTypes.size(); ++rtIdx)
        {
            std::vector<QString> availableParameters = nncData->availableProperties(resultTypes[rtIdx]);

            for (const QString& parameter : availableParameters)
            {
                propertyNames.push_back(parameter);
                propertyTypes.push_back(resultTypeNames[rtIdx]);
            }
        }

        qint64 byteCount = 0;

        for (size_t ptIdx = 0; ptIdx < propertyNames.size(); ++ptIdx)
        {
            byteCount += propertyNames[ptIdx].size() * sizeof(QChar);
            byteCount += propertyTypes[ptIdx].size() * sizeof(QChar);
        }

        // Byte count
        socketStream << byteCount;

        // Parameter count
        socketStream << (quint64)propertyNames.size();

        for (size_t ptIdx = 0; ptIdx < propertyNames.size(); ++ptIdx)
        {
            socketStream << propertyNames[ptIdx];
            socketStream << propertyTypes[ptIdx];
        }

        return true;
    }
};

static bool RiaGetNNCPropertyNames_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetNNCPropertyNames>(RiaGetNNCPropertyNames::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaSetNNCProperty: public RiaSocketCommand
{
public:
    RiaSetNNCProperty() :
        m_currentReservoir(NULL),
        m_currentScalarIndex(cvf::UNDEFINED_SIZE_T),
        m_timeStepCountToRead(0),
        m_bytesPerTimeStepToRead(0),
        m_currentTimeStepNumberToRead(0),
        m_invalidConnectionCountDetected(false),
        m_porosityModelEnum(RiaDefines::MATRIX_MODEL)
    {}

    static QString commandName () { return QString("SetNNCProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString propertyName = args[2];

        // Find the requested data, or create a set if we are setting data and it is not found
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            QString caseId = args[1];
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find case with id %1").arg(caseId));
            return true;
        }

        // If we have not read the header and there are data enough: Read it.
        // Do nothing if we have not enough data
        if (m_timeStepCountToRead == 0 || m_bytesPerTimeStepToRead == 0)
        {
            if (server->currentClient()->bytesAvailable() < (int)sizeof(quint64)*2) return true;

            socketStream >> m_timeStepCountToRead;
            socketStream >> m_bytesPerTimeStepToRead;
        }

        RigNNCData* nncData = rimCase->eclipseCaseData()->mainGrid()->nncData();

        auto nncResults = nncData->generatedConnectionScalarResultByName(propertyName);

        if (nncResults == nullptr)
        {
            nncData->makeGeneratedConnectionScalarResult(propertyName, m_timeStepCountToRead);
        }

        if (rimCase && rimCase->results(m_porosityModelEnum))
        {
            bool ok = createIJKCellResults(rimCase->results(m_porosityModelEnum), propertyName);
            if (!ok)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the property named: \"%2\"").arg(propertyName));
                return true;
            }
            size_t scalarResultIndex = rimCase->results(m_porosityModelEnum)->findOrLoadScalarResult(QString("%1IJK").arg(propertyName));
            nncData->setScalarResultIndex(propertyName, scalarResultIndex);
        }

        // Create a list of all the requested time steps
        m_requestedTimesteps.clear();

        if (args.size() <= 4)
        {
            // Select all
            for (size_t tsIdx = 0; tsIdx < m_timeStepCountToRead; ++tsIdx)
            {
                m_requestedTimesteps.push_back(tsIdx);
            }
        }
        else
        {
            bool timeStepReadError = false;
            for (int argIdx = 4; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    m_requestedTimesteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riSetNNCProperty : \n") +
                                                          RiaSocketServer::tr("An error occurred while interpreting the requested time steps."));
            }

        }

        if (! m_requestedTimesteps.size())
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("No time steps specified"));

            return true;
        }

        m_currentReservoir = rimCase;
        m_currentPropertyName = propertyName;

        if (server->currentClient()->bytesAvailable())
        {
            return this->interpretMore(server, server->currentClient());
        }

        return false;
    }

    static bool createIJKCellResults(RigCaseCellResultsData* results, QString propertyName)
    {
        bool ok;
        ok = scalarResultExistsOrCreate(results, QString("%1IJK").arg(propertyName));
        if (!ok) return false;
        ok = scalarResultExistsOrCreate(results, QString("%1I").arg(propertyName));
        if (!ok) return false;
        ok = scalarResultExistsOrCreate(results, QString("%1J").arg(propertyName));
        if (!ok) return false;
        ok = scalarResultExistsOrCreate(results, QString("%1K").arg(propertyName));

        return ok;
    }

    static bool scalarResultExistsOrCreate(RigCaseCellResultsData* results, QString propertyName)
    {
        size_t scalarResultIndex = results->findOrLoadScalarResult(propertyName);
        if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
        {
            scalarResultIndex = results->findOrCreateScalarResultIndex(RiaDefines::GENERATED, propertyName, true);
        }
        
        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
        {
            std::vector< std::vector<double> >* scalarResultFrames = nullptr;
            scalarResultFrames = &(results->cellScalarResults(scalarResultIndex));
            size_t timeStepCount = results->maxTimeStepCount();
            scalarResultFrames->resize(timeStepCount);
            return true;
        }

        return false;
    }

    virtual bool interpretMore(RiaSocketServer* server, QTcpSocket* currentClient)
    {
        if (m_invalidConnectionCountDetected) return true;

        // If nothing should be read, or we already have read everything, do nothing
        if ((m_timeStepCountToRead == 0) || (m_currentTimeStepNumberToRead >= m_timeStepCountToRead) )  return true;

        if (!currentClient->bytesAvailable()) return false;

        if (m_timeStepCountToRead != m_requestedTimesteps.size())
        {
            CVF_ASSERT(false);
        }

        // Check if a complete timestep is available, return and whait for readyRead() if not
        if (currentClient->bytesAvailable() < (int)m_bytesPerTimeStepToRead) return false;

        RigNNCData* nncData = m_currentReservoir->eclipseCaseData()->mainGrid()->nncData();

        size_t connectionCountFromOctave = m_bytesPerTimeStepToRead / sizeof(double);
        size_t connectionCount = nncData->connections().size();
        std::vector< std::vector<double> >* resultsToAdd = nncData->generatedConnectionScalarResultByName(m_currentPropertyName);

        if (connectionCountFromOctave != connectionCount)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") +
                                                      RiaSocketServer::tr("The number of connections in the data coming from octave does not match the case: '%1'\n").arg(m_currentReservoir->caseUserDescription()) +
                                                      RiaSocketServer::tr("   Octave: %1\n").arg(connectionCountFromOctave) +
                                                      RiaSocketServer::tr("  %1: Connection count: %2").arg(m_currentReservoir->caseUserDescription()).arg(connectionCount));

            connectionCountFromOctave = 0;
            m_invalidConnectionCountDetected = true;
            currentClient->abort();

            return true;
        }

        for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
        {
            size_t tsId = m_requestedTimesteps[tIdx];
            resultsToAdd->at(tsId).resize(connectionCount, HUGE_VAL);
        }

        double* internalMatrixData = nullptr;

        QDataStream socketStream(currentClient);
        socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

        // Read available complete time step data
        while ((currentClient->bytesAvailable() >= (int)m_bytesPerTimeStepToRead) && (m_currentTimeStepNumberToRead < m_timeStepCountToRead))
        {
            internalMatrixData = resultsToAdd->at(m_requestedTimesteps[m_currentTimeStepNumberToRead]).data();

            QStringList errorMessages;
            if (!RiaSocketDataTransfer::readBlockDataFromSocket(currentClient, (char*)(internalMatrixData), m_bytesPerTimeStepToRead, errorMessages))
            {
                for (int i = 0; i < errorMessages.size(); i++)
                {
                    server->errorMessageDialog()->showMessage(errorMessages[i]);
                }

                currentClient->abort();
                return true;
            }
            ++m_currentTimeStepNumberToRead;
        }


        // If we have read all the data, refresh the views

        if (m_currentTimeStepNumberToRead == m_timeStepCountToRead)
        {
            if (m_currentReservoir != nullptr)
            {
                // Create a new input property if we have an input reservoir
                RimEclipseInputCase* inputRes = dynamic_cast<RimEclipseInputCase*>(m_currentReservoir);
                if (inputRes)
                {
                    RimEclipseInputProperty* inputProperty = inputRes->m_inputPropertyCollection->findInputProperty(m_currentPropertyName);
                    if (!inputProperty)
                    {
                        inputProperty = new RimEclipseInputProperty;
                        inputProperty->resultName = m_currentPropertyName;
                        inputProperty->eclipseKeyword = "";
                        inputProperty->fileName = "";
                        inputRes->m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                        inputRes->m_inputPropertyCollection()->updateConnectedEditors();
                    }
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED_NOT_SAVED;
                }

                if( m_currentScalarIndex != cvf::UNDEFINED_SIZE_T &&
                    m_currentReservoir->eclipseCaseData() &&
                    m_currentReservoir->eclipseCaseData()->results(m_porosityModelEnum) )
                {
                    m_currentReservoir->eclipseCaseData()->results(m_porosityModelEnum)->recalculateStatistics(m_currentScalarIndex);
                }

                for (size_t i = 0; i < m_currentReservoir->reservoirViews.size(); ++i)
                {
                    if (m_currentReservoir->reservoirViews[i])
                    {
                        // As new result might have been introduced, update all editors connected
                        m_currentReservoir->reservoirViews[i]->cellResult->updateConnectedEditors();

                        // It is usually not needed to create new display model, but if any derived geometry based on generated data (from Octave) 
                        // a full display model rebuild is required
                        m_currentReservoir->reservoirViews[i]->scheduleCreateDisplayModelAndRedraw();
                    }
                }
            }

            return true;
        }

        return false;

    }

private:
    RimEclipseCase*                     m_currentReservoir;
    size_t                              m_currentScalarIndex;
    QString                             m_currentPropertyName;
    std::vector<size_t>                 m_requestedTimesteps;
    RiaDefines::PorosityModelType       m_porosityModelEnum;

    quint64                             m_timeStepCountToRead;
    quint64                             m_bytesPerTimeStepToRead;
    size_t                              m_currentTimeStepNumberToRead;

    bool                                m_invalidConnectionCountDetected;
};

static bool RiaSetNNCProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaSetNNCProperty>(RiaSetNNCProperty::commandName());
