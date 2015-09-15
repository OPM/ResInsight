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
#include "RimWellLog.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellLogFile.h"

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

    m_wellLogFile = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLasFileInfo::~RimWellLasFileInfo()
{
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
    if (!m_wellLogFile.p())
    {
        m_wellLogFile = new RigWellLogFile;
    }

    if (!m_wellLogFile->open(m_fileName))
    {
        return false;
    }

    m_wellName = m_wellLogFile->wellName();
    m_name = QFileInfo(m_fileName).fileName();

    m_lasFileLogs.deleteAllChildObjects();

    QStringList wellLogNames = m_wellLogFile->wellLogNames();
    for (int logIdx = 0; logIdx < wellLogNames.size(); logIdx++)
    {
        RimWellLog* wellLog = new RimWellLog();
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

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLasFileInfo::wellName() const
{
    return m_wellName;
}
