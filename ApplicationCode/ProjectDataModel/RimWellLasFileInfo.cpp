/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLasFileInfo.h"
#include "RimWellLasLog.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "well.hpp"

#include <QString>
#include <QStringList>
#include <QFileInfo>


CAF_PDM_SOURCE_INIT(RimWellLasFileInfo, "WellLasFileInfo");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLasFileInfo::RimWellLasFileInfo()
{
    CAF_PDM_InitObject("Well LAS File Info", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "",  "", "", "");
    m_wellName.uiCapability()->setUiReadOnly(true);
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellName.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_fileName, "FileName", "Filename",  "", "", "");
    m_fileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "",  "", "", "");
    m_name.uiCapability()->setUiReadOnly(true);
    m_name.uiCapability()->setUiHidden(true);
    m_name.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_lasFileLogs, "WellLASFileLogs", "",  "", "", "");
    m_lasFileLogs.uiCapability()->setUiHidden(true);
    m_lasFileLogs.xmlCapability()->setIOWritable(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLasFileInfo::~RimWellLasFileInfo()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLasFileInfo::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLasFileInfo::readFile()
{
    close();

    int wellFormat = NRLib::Well::LAS;
    NRLib::Well* well = NRLib::Well::ReadWell(m_fileName().toStdString(), wellFormat);
    if (!well)
    {
        // TODO: Error handling
        return false;
    }

    m_wellName = QString::fromStdString(well->GetWellName());
    m_name = QFileInfo(m_fileName).fileName();

    QStringList wellLogNames;

    const std::map<std::string, std::vector<double> >& contLogs = well->GetContLog();
    std::vector<std::string> contLogNames;
    std::map<std::string, std::vector<double> >::const_iterator itCL;
    for (itCL = contLogs.begin(); itCL != contLogs.end(); itCL++)
    {
        wellLogNames.append(QString::fromStdString(itCL->first));
    }

    const std::map<std::string, std::vector<int> >& discLogs = well->GetDiscLog();
    std::vector<std::string> discLogNames;
    std::map<std::string, std::vector<int> >::const_iterator itDL;
    for (itDL = discLogs.begin(); itDL != discLogs.end(); itDL++)
    {
        wellLogNames.append(QString::fromStdString(itDL->first));
    }

    for (size_t logIdx = 0; logIdx < wellLogNames.size(); logIdx++)
    {
        RimWellLasLog* wellLog = new RimWellLasLog();
        wellLog->setName(wellLogNames[logIdx]);
        m_lasFileLogs.push_back(wellLog);
    }

    RimWellPath* wellPath;
    firstAnchestorOrThisOfType(wellPath);

    if (wellPath)
    {
        if (wellPath->filepath().isEmpty())
        {
            wellPath->name = m_wellName;
        }
    }

    delete well;
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLasFileInfo::close()
{   
    m_lasFileLogs.deleteAllChildObjects();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLasFileInfo::wellName() const
{
    return m_wellName;
}
