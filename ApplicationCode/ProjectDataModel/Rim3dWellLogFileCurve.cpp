/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dWellLogFileCurve.h"

#include "RigWellLogFile.h"

#include "RimWellLogCurveNameConfig.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellPath.h"

#include <QFileInfo>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogFileCurve, "Rim3dWellLogFileCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogFileCurve::Rim3dWellLogFileCurve()
{
    CAF_PDM_InitObject("3d Well Log File Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelName, "CurveWellLogChannel", "Well Log Channel", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogFile, "WellLogFile", "Well Log File", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "", "", "", "");
    m_nameConfig = new RimWellLogFileCurveNameConfig(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogFileCurve::~Rim3dWellLogFileCurve()
{
    delete m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::setDefaultFileCurveDataInfo()
{
    RimWellPath* wellPath = nullptr;
    firstAncestorOrThisOfType(wellPath);

    if (wellPath && !wellPath->wellLogFiles().empty())
    {
        m_wellLogFile = wellPath->wellLogFiles()[0];
    }

    if (m_wellLogFile)
    {
        std::vector<RimWellLogFileChannel*> fileLogs = m_wellLogFile->wellLogChannels();

        if (!fileLogs.empty())
        {
            m_wellLogChannelName = fileLogs[0]->name();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const
{
    CAF_ASSERT(values != nullptr);
    CAF_ASSERT(measuredDepthValues != nullptr);

    if (m_wellLogFile)
    {
        RigWellLogFile* wellLogFile = m_wellLogFile->wellLogFileData();
        if (wellLogFile)
        {
            *values              = wellLogFile->values(m_wellLogChannelName);
            *measuredDepthValues = wellLogFile->depthValues();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::resultPropertyString() const
{
    return m_wellLogChannelName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::name() const
{
    return m_nameConfig->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::createAutoName() const
{
    QStringList name;
    QString unit;
    bool channelNameAvailable = false;

    RimWellPath* wellPath;
    this->firstAncestorOrThisOfType(wellPath);

    if (wellPath)
    {
        name.push_back(wellPath->name());
        name.push_back("LAS");

        if (!m_wellLogChannelName().isEmpty())
        {
            name.push_back(m_wellLogChannelName);
            channelNameAvailable = true;
        }

        RigWellLogFile* wellLogFile = m_wellLogFile ? m_wellLogFile->wellLogFileData() : nullptr;

        if (wellLogFile)
        {
            if (channelNameAvailable)
            {
             /*   RimWellLogPlot* wellLogPlot;
                firstAncestorOrThisOfType(wellLogPlot);
                CVF_ASSERT(wellLogPlot);
                QString unitName = wellLogFile->wellLogChannelUnitString(m_wellLogChannelName, wellLogPlot->depthUnit());

                if (!unitName.isEmpty())
                {
                    name.back() += QString(" [%1]").arg(unitName);
                } */
            }

            QString date = wellLogFile->date();
            if (!date.isEmpty())
            {
                name.push_back(wellLogFile->date());
            }

        }

        return name.join(", ");
    }

    return "Empty curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dWellLogFileCurve::userDescriptionField()
{
    return m_nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue)
{
    if (changedField == &m_wellLogFile || changedField == &m_wellLogChannelName)
    {
        this->resetMinMaxValuesAndUpdateUI();
    }
    Rim3dWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogFileCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                           bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    options = Rim3dWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (!options.empty()) return options;

    if (fieldNeedingOptions == &m_wellLogChannelName)
    {
        if (m_wellLogFile)
        {
            std::vector<RimWellLogFileChannel*> fileLogs = m_wellLogFile->wellLogChannels();

            for (size_t i = 0; i < fileLogs.size(); i++)
            {
                QString wellLogChannelName = fileLogs[i]->name();
                options.push_back(caf::PdmOptionItemInfo(wellLogChannelName, wellLogChannelName));
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", "None"));
        }
    }

    if (fieldNeedingOptions == &m_wellLogFile)
    {
        RimWellPath* wellPath = nullptr;
        firstAncestorOrThisOfType(wellPath);

        if (wellPath && !wellPath->wellLogFiles().empty())
        {
            for (RimWellLogFile* const wellLogFile : wellPath->wellLogFiles())
            {
                QFileInfo fileInfo(wellLogFile->fileName());
                options.push_back(caf::PdmOptionItemInfo(fileInfo.baseName(), wellLogFile));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_wellLogFile);
    curveDataGroup->add(&m_wellLogChannelName);

    Rim3dWellLogCurve::configurationUiOrdering(uiOrdering);

    m_nameConfig()->createUiGroup(uiConfigName, uiOrdering);

    uiOrdering.skipRemainingFields(true);
}
