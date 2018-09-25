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

#include "RicfCommandFileExecutor.h"

#include "RiaLogging.h"

#include "RicfCloseProject.h"
#include "RicfCommandObject.h"
#include "RicfOpenProject.h"
#include "RicfReplaceCase.h"
#include "RifcCommandFileReader.h"

namespace caf {
    template<>
    void RicfCommandFileExecutor::ExportTypeEnum::setUp()
    {
        addItem(RicfCommandFileExecutor::COMPLETIONS, "COMPLETIONS", "Completions");
        addItem(RicfCommandFileExecutor::PROPERTIES,  "PROPERTIES",  "Properties");
        addItem(RicfCommandFileExecutor::SNAPSHOTS,   "SNAPSHOTS",   "Snapshots");
        addItem(RicfCommandFileExecutor::STATISTICS,  "STATISTICS",  "Statistics");
        addItem(RicfCommandFileExecutor::WELLPATHS,   "WELLPATHS",   "Well Path");
        setDefault(RicfCommandFileExecutor::COMPLETIONS);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfCommandFileExecutor::RicfCommandFileExecutor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfCommandFileExecutor::~RicfCommandFileExecutor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCommandFileExecutor::executeCommands(QTextStream& stream)
{
    RicfMessages messages;
    std::vector<RicfCommandObject*> executableCommands;
    {
        std::vector<RicfCommandObject*> fileCommands = RicfCommandFileReader::readCommands(stream, caf::PdmDefaultObjectFactory::instance(), &messages);
        for (auto message : messages.m_messages)
        {
            if (message.first == RicfMessages::MESSAGE_WARNING)
            {
                RiaLogging::warning(QString("Command file parsing warning: %1").arg(message.second));
            }
            else
            {
                RiaLogging::error(QString("Command file parsing error: %1").arg(message.second));

                for (auto& command : fileCommands)
                {
                    delete command;
                    command = nullptr;
                }

                return;
            }
        }

        for (auto fileCommand : fileCommands)
        {
            fileCommand->initAfterReadRecursively();
        }

        executableCommands = RicfCommandFileExecutor::prepareFileCommandsForExecution(fileCommands);
    }

    for (auto& command : executableCommands)
    {
        command->execute();

        delete command;
        command = nullptr;
    }

    // All command objects should be deleted and grounded at this point
    for (auto c : executableCommands)
    {
        CAF_ASSERT(c == nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCommandFileExecutor::setExportPath(ExportType type, QString path)
{
    m_exportPaths[type] = path;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicfCommandFileExecutor::getExportPath(ExportType type) const
{
    auto it = m_exportPaths.find(type);
    QString path;
    if (it != m_exportPaths.end())
    {
        path = it->second;
    }
    return path;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCommandFileExecutor::setLastProjectPath(const QString& path)
{
    m_lastProjectPath = path;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicfCommandFileExecutor::getLastProjectPath() const
{
    return m_lastProjectPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfCommandFileExecutor* RicfCommandFileExecutor::instance()
{
    static RicfCommandFileExecutor* commandFileExecutorInstance = new RicfCommandFileExecutor();
    return commandFileExecutorInstance;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RicfCommandObject*> RicfCommandFileExecutor::prepareFileCommandsForExecution(const std::vector<RicfCommandObject*>& commandsReadFromFile)
{
    // This function will merge multiple RicfSingleCaseReplace object by a single RicfMultiCaseReplace object
    // A command file version for multi case replace was rejected by @hhgs 2018-02-02
    //
    // The reason for this is based on two requirements
    //   1. Ability to aggregate info from multiple replaceCase() statements in a command file
    //   2. Improve performance, as a replace case is implemented by reading a project file from XML and replace file paths
    //      during project loading

    std::vector<RicfCommandObject*> executableCommands;
    {
        std::vector<RicfSingleCaseReplace*> objectsToBeDeleted;
        std::vector<RicfSingleCaseReplace*> batchOfReplaceCaseFileObjects;
        
        std::map<int, QString> aggregatedCasePathPairs;

        for (RicfCommandObject* command : commandsReadFromFile)
        {
            RicfSingleCaseReplace* fileReplaceCase = dynamic_cast<RicfSingleCaseReplace*>(command);
            if (fileReplaceCase)
            {
                aggregatedCasePathPairs[fileReplaceCase->caseId()] = fileReplaceCase->filePath();

                batchOfReplaceCaseFileObjects.push_back(fileReplaceCase);
                objectsToBeDeleted.push_back(fileReplaceCase);
            }
            else
            {
                if (!batchOfReplaceCaseFileObjects.empty())
                {
                    RicfMultiCaseReplace* multiCaseReplace = new RicfMultiCaseReplace;
                    multiCaseReplace->setCaseReplacePairs(aggregatedCasePathPairs);

                    executableCommands.push_back(multiCaseReplace);

                    batchOfReplaceCaseFileObjects.clear();
                }

                if (dynamic_cast<RicfOpenProject*>(command) || dynamic_cast<RicfCloseProject*>(command))
                {
                    // Reset aggregation when openProject or closeProject is issued
                    aggregatedCasePathPairs.clear();
                }

                executableCommands.push_back(command);
            }
        }

        // Delete RicfSingleCaseReplace objects, as they are replaced by RicfMultiCaseReplace
        for (auto objToDelete : objectsToBeDeleted)
        {
            delete objToDelete;
            objToDelete = nullptr;
        }
    }

    return executableCommands;
}
