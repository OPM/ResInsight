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
//#include "RimWellPltPlot.h"
#include "RimWellPlotTools.h"

#include "RiaDateStringParser.h"

#include "RigWellLogFile.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDateEditor.h"

#include <QStringList>
#include <QFileInfo>
#include <QMessageBox>
#include "RiaQDateTimeTools.h"


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
const QDateTime RimWellLogFile::DEFAULT_DATE_TIME = RiaQDateTimeTools::createUtcDateTime(QDate(1900, 1, 1));

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

    CAF_PDM_InitField(&m_wellFlowCondition, "WellFlowCondition", caf::AppEnum<RimWellLogFile::WellFlowCondition>(RimWellLogFile::WELL_FLOW_COND_STANDARD), "Well Flow Condition", "", "", "");

    CAF_PDM_InitField(&m_invalidDateMessage, "InvalidDateMessage", QString("Invalid or no date"), "", "", "", "");
    m_invalidDateMessage.uiCapability()->setUiReadOnly(true);
    m_invalidDateMessage.xmlCapability()->disableIO();

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
    m_lasFileHasValidDate = isDateValid(date);
    if (m_lasFileHasValidDate)
    {
        m_date = date;
    }
    else if(!isDateValid(m_date()))
    {
        QMessageBox msgBox;

        QString message = QString("The LAS-file '%1' contains no recognizable date. Please assign a date in the LAS-file property panel")
                                .arg(m_name());
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        m_date = DEFAULT_DATE_TIME;
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
    return RimWellPlotTools::hasFlowData(this);
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

    auto timespec = m_date().timeSpec();

    if (!isDateValid(m_date()))
    {
        uiOrdering.add(&m_invalidDateMessage);
    }

    if (hasFlowData())
    {
        uiOrdering.add(&m_wellFlowCondition);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_date)
    {
        // Due to a possible bug in QDateEdit/PdmUiDateEditor, convert m_date to a QDateTime having UTC timespec
        m_date = RiaQDateTimeTools::createUtcDateTime(m_date().date(), m_date().time());
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogFile::isDateValid(const QDateTime dateTime)
{
    return dateTime.isValid() && dateTime != DEFAULT_DATE_TIME;
}
