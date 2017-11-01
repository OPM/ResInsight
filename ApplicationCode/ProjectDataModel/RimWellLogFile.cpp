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

#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimTools.h"
#include "RimWellPltPlot.h"
#include "RiaDateStringParser.h"

#include "RigWellLogFile.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDateEditor.h"

#include <QStringList>
#include <QFileInfo>
#include <QMessageBox>


CAF_PDM_SOURCE_INIT(RimWellLogFile, "WellLogFile");

namespace caf
{
    template<>
    void caf::AppEnum< RimWellLogFile::WellFlowCondition>::setUp()
    {
        addItem(RimWellLogFile::WELL_FLOW_COND_RESERVOIR, "RESERVOIR", "Reservoir");
        addItem(RimWellLogFile::WELL_FLOW_COND_STANDARD, "STANDARD", "Standard");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile::RimWellLogFile()
{
    CAF_PDM_InitObject("Well LAS File Info", ":/LasFile16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "",  "", "", "");
    m_wellName.uiCapability()->setUiReadOnly(true);
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellName.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_date, "Date", "Date", "", "", "");
    m_date.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_fileName, "FileName", "Filename",  "", "", "");
    m_fileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "",  "", "", "");
    m_name.uiCapability()->setUiReadOnly(true);
    m_name.uiCapability()->setUiHidden(true);
    m_name.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelNames, "WellLogFileChannels", "",  "", "", "");
    m_wellLogChannelNames.uiCapability()->setUiHidden(true);
    m_wellLogChannelNames.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_wellFlowCondition, "WellFlowCondition", "Well Flow Condition", "", "", "");

    m_wellLogDataFile = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile::~RimWellLogFile()
{
    m_wellLogChannelNames.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellLogFile::readWellLogFile(const QString& logFilePath)
{
    QFileInfo fi(logFilePath);

    RimWellLogFile* wellLogFile = NULL;

    if (fi.suffix().toUpper().compare("LAS") == 0)
    {
        QString errorMessage;
        wellLogFile = new RimWellLogFile();
        wellLogFile->setFileName(logFilePath);
        if (!wellLogFile->readFile(&errorMessage))
        {
            QString displayMessage = "Could not open the LAS file: \n" + logFilePath;

            if (!errorMessage.isEmpty())
            {
                displayMessage += "\n\n";
                displayMessage += errorMessage;
            }

            QMessageBox::warning(RiuMainWindow::instance(), 
                "File open error", 
                displayMessage);

            delete wellLogFile;
            wellLogFile = NULL;
        }
    }

    return wellLogFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogFile::readFile(QString* errorMessage)
{
    if (!m_wellLogDataFile.p())
    {
        m_wellLogDataFile = new RigWellLogFile;
    }

    m_name = QFileInfo(m_fileName).fileName();

    if (!m_wellLogDataFile->open(m_fileName, errorMessage))
    {
        m_wellLogDataFile = NULL;
        return false;
    }

    m_wellName = m_wellLogDataFile->wellName();

    QDateTime date = RiaDateStringParser::parseDateString(m_wellLogDataFile->date());
    m_lasFileHasValidDate = date.isValid();
    if (m_lasFileHasValidDate)
    {
        m_date = date;
    }

    m_wellLogChannelNames.deleteAllChildObjects();

    QStringList wellLogNames = m_wellLogDataFile->wellLogChannelNames();
    for (int logIdx = 0; logIdx < wellLogNames.size(); logIdx++)
    {
        RimWellLogFileChannel* wellLog = new RimWellLogFileChannel();
        wellLog->setName(wellLogNames[logIdx]);
        m_wellLogChannelNames.push_back(wellLog);
    }

    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->filepath().isEmpty())
        {
            wellPath->setName(m_wellName);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFile::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLogFile::date() const
{
    return m_date;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellLogFile::wellLogChannels() const
{
    std::vector<RimWellLogFileChannel*> channels;
    for (const auto& channel : m_wellLogChannelNames)
    {
        channels.push_back(channel);
    }
    return channels;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogFile::hasFlowData() const
{
    return RimWellPltPlot::hasFlowData(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::setupBeforeSave()
{
    m_wellFlowCondition.xmlCapability()->setIOWritable(hasFlowData());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_fileName);
    uiOrdering.add(&m_date);
    m_date.uiCapability()->setUiReadOnly(m_lasFileHasValidDate);

    if (hasFlowData())
    {
        uiOrdering.add(&m_wellFlowCondition);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogFile::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_date)
    {

    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiDateEditorAttribute* attrib = dynamic_cast<caf::PdmUiDateEditorAttribute*> (attribute);
    if (attrib != nullptr)
    {
        attrib->dateFormat = RimTools::dateFormatString();
    }
}
