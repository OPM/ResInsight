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

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannnelName, "CurveWellLogChannel", "Well Log Channel", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogFile, "WellLogFile", "Well Log File", "", "", "");

    m_name = "3D Well Log LAS Curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogFileCurve::~Rim3dWellLogFileCurve() {}

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
            m_wellLogChannnelName = fileLogs[0]->name();
            m_name                = "LAS: " + m_wellLogChannnelName;
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
            *values              = wellLogFile->values(m_wellLogChannnelName);
            *measuredDepthValues = wellLogFile->depthValues();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue)
{
    if (changedField == &m_wellLogFile || changedField == &m_wellLogChannnelName)
    {
        this->resetMinMaxValuesAndUpdateUI();
    }
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

    if (fieldNeedingOptions == &m_wellLogChannnelName)
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
    curveDataGroup->add(&m_wellLogChannnelName);

    Rim3dWellLogCurve::configurationUiOrdering(uiOrdering);

    uiOrdering.skipRemainingFields(true);
}
