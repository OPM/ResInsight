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

#include "RifcCommandFileReader.h"
#include "RicfCommandObject.h"

namespace caf {
    template<>
    void RicfCommandFileExecutor::ExportTypeEnum::setUp()
    {
        addItem(RicfCommandFileExecutor::COMPLETIONS, "COMPLETIONS", "Completions");
        addItem(RicfCommandFileExecutor::PROPERTIES,  "PROPERTIES",  "Properties");
        addItem(RicfCommandFileExecutor::SNAPSHOTS,   "SNAPSHOTS",   "Snapshots");
        addItem(RicfCommandFileExecutor::STATISTICS,  "STATISTICS",  "Statistics");
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
    std::vector<RicfCommandObject*> commands = RicfCommandFileReader::readCommands(stream, caf::PdmDefaultObjectFactory::instance(), &m_messages);
    for (auto message : m_messages.m_messages)
    {
        std::cout << message.second.toUtf8().constData();
    }
    for (RicfCommandObject* command : commands)
    {
        command->execute();
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
QString RicfCommandFileExecutor::getExportPath(ExportType type)
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
RicfCommandFileExecutor* RicfCommandFileExecutor::instance()
{
    static RicfCommandFileExecutor* commandFileExecutorInstance = new RicfCommandFileExecutor();
    return commandFileExecutorInstance;
}
