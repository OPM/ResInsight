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

#include "RiaSocketCommand.h"

#include "RiaApplication.h"
#include "RiaSocketServer.h"
#include "RiaSocketTools.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimScriptCollection.h"
#include "RimSimWellInViewCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"



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
        RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        caseIds.push_back(rimCase->caseId);
        caseNames.push_back(rimCase->caseUserDescription);
        caseTypes.push_back(caseType);
        caseGroupIds.push_back(caseGroupId);
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetCurrentCase : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCurrentCase"); }
    bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) override
    {
        qint64  caseId = server->currentCaseId();
        QString caseName;
        QString caseType;
        qint64  caseGroupId = -1;

        RimEclipseCase* rimCase = server->findReservoir(caseId);

        if (rimCase)
        {
            RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);
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



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetSelectedCases: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetSelectedCases"); }

    bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) override
    {
        {
            std::vector<RimCase*> cases;
            caf::SelectionManager::instance()->objectsByType(&cases);

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


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetCaseGroups : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCaseGroups"); }
    bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) override
    {
        RimProject* proj = RiaApplication::instance()->project();
        RimEclipseCaseCollection* analysisModels = (proj && proj->activeOilField()) ? proj->activeOilField()->analysisModels() : nullptr;
        if (analysisModels)
        {
            std::vector<QString> groupNames;
            std::vector<qint64> groupIds;

            size_t caseGroupCount = analysisModels->caseGroups().size();
            quint64 byteCount = 0;

            for (size_t i = 0; i < caseGroupCount; i++)
            {
                RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

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
class RiaGetCases : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCases"); }

    bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream) override
    {
        int argCaseGroupId = -1;

        if (args.size() == 2)
        {
            argCaseGroupId = args[1].toInt();
        }

        RimProject* proj = RiaApplication::instance()->project();
        RimEclipseCaseCollection* analysisModels = (proj && proj->activeOilField()) ? proj->activeOilField()->analysisModels() : nullptr;
        if (analysisModels)
        {

            std::vector<RimCase*> cases;
            if (argCaseGroupId == -1)
            {
                proj->allCases(cases);
            }
            else
            {
                RimIdenticalGridCaseGroup* caseGroup = nullptr;
                for (size_t i = 0; i < analysisModels->caseGroups().size(); i++)
                {
                    RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

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
