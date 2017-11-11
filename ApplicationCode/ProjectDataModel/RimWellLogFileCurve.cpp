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

#include "RimWellLogFileCurve.h"

#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellRftPlot.h"
#include "RimWellPlotTools.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QMessageBox>
#include <QFileInfo>


CAF_PDM_SOURCE_INIT(RimWellLogFileCurve, "WellLogFileCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::RimWellLogFileCurve()
{
    CAF_PDM_InitObject("Well Log File Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Path", "", "", "");
    m_wellPath.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannnelName, "CurveWellLogChannel", "Well Log Channel", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogFile, "WellLogFile", "Well Log File", "", "", "");

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::~RimWellLogFileCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    this->RimPlotCurve::updateCurvePresentation(updateParentPlot);

    if (isCurveVisible())
    {
        m_curveData = new RigWellLogCurveData;

        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);

        if (m_wellPath && m_wellLogFile)
        {
            RigWellLogFile* wellLogFile = m_wellLogFile->wellLogFile();
            if (wellLogFile)
            {
                std::vector<double> values = wellLogFile->values(m_wellLogChannnelName);
                std::vector<double> measuredDepthValues = wellLogFile->depthValues();

                if (wellLogPlot && wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
                {
                    bool canUseTvd = false;
                    if (wellLogFile->hasTvdChannel())
                    {
                        std::vector<double> tvdMslValues = wellLogFile->tvdMslValues();

                        if (values.size() == measuredDepthValues.size() && values.size() == tvdMslValues.size())
                        {
                            m_curveData->setValuesWithTVD(values, measuredDepthValues, tvdMslValues, wellLogFile->depthUnit(), false);
                            canUseTvd = true;
                        }
                    }

                    if (!canUseTvd)
                    {
                        RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();
                        if (rigWellPath)
                        {
                            std::vector<double> trueVerticeldepthValues;

                            for (double measuredDepthValue : measuredDepthValues)
                            {
                                trueVerticeldepthValues.push_back(-rigWellPath->interpolatedPointAlongWellPath(measuredDepthValue).z());
                            }
                            if (values.size() == trueVerticeldepthValues.size() && values.size() == measuredDepthValues.size())
                            {
                                m_curveData->setValuesWithTVD(values, measuredDepthValues, trueVerticeldepthValues, wellLogFile->depthUnit(), false);
                                canUseTvd = true;
                            }
                        }
                    }

                    if (!canUseTvd)
                    {
                        if (RiaApplication::instance()->preferences()->showLasCurveWithoutTvdWarning())
                        {
                            QString tmp = QString("Display of True Vertical Depth (TVD) for LAS curves is not possible without a well log path, and the LAS curve will be hidden in this mode.\n\n");
                            tmp += "Control display of this warning from \"Preferences->Show LAS curve without TVD warning\"";

                            QMessageBox::warning(nullptr, "LAS curve without TVD", tmp);
                        }
                    }
                }
                else
                {
                    if (values.size() == measuredDepthValues.size())
                    {
                        m_curveData->setValuesAndMD(values, measuredDepthValues, wellLogFile->depthUnit(), false);
                    }
                }
            }

            if (m_isUsingAutoName)
            {
                m_qwtPlotCurve->setTitle(createCurveAutoName());
            }
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;
        if (wellLogPlot)
        {
            displayUnit = wellLogPlot->depthUnit();
        }
        if (wellLogPlot && wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->trueDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }
        else
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->measuredDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }
        m_qwtPlotCurve->setLineSegmentStartStopIndices(m_curveData->polylineStartStopIndices());

        updateZoomInParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellPath(RimWellPath* wellPath)
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogFileCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellLogChannelName(const QString& name)
{
    m_wellLogChannnelName = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellLogFile(RimWellLogFile* wellLogFile)
{
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellPath)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellLogFile)
    {
        this->loadDataAndUpdate(true);
    }
    if (m_parentQwtPlot) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_wellPath);
    curveDataGroup->add(&m_wellLogFile);
    curveDataGroup->add(&m_wellLogChannnelName);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogFileCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    if (options.size() > 0) return options;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = proj->activeOilField()->wellPathCollection()->wellPaths;

            for (size_t i = 0; i < wellPaths.size(); i++)
            {
                // Only include well paths coming from a well log file
                if (wellPaths[i]->wellLogFiles().size() > 0)
                {
                    options.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), wellPaths[i]));
                }
            }

            if (options.size() > 0)
            {
                options.push_front(caf::PdmOptionItemInfo("None", nullptr));
            }
        }
    }

    if (fieldNeedingOptions == &m_wellLogChannnelName)
    {
        if (m_wellPath())
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
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", "None"));
        }
    }

    if (fieldNeedingOptions == &m_wellLogFile)
    {
        
        if (m_wellPath() && m_wellPath->wellLogFiles().size() > 0)
        {
            bool isRftChild = isRftPlotChild();

            for (RimWellLogFile* const wellLogFile : m_wellPath->wellLogFiles())
            {
                if (!isRftChild || RimWellPlotTools::hasPressureData(wellLogFile))
                {
                    QFileInfo fileInfo(wellLogFile->fileName());
                    options.push_back(caf::PdmOptionItemInfo(fileInfo.baseName(), wellLogFile));
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::initAfterRead()
{
    if (m_wellPath->wellLogFiles().size() == 1)
    {
        m_wellLogFile = m_wellPath->wellLogFiles().front();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogFileCurve::isRftPlotChild() const
{
    RimWellRftPlot* rftPlot;
    firstAncestorOrThisOfType(rftPlot);
    return rftPlot != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::createCurveAutoName()
{
    QStringList name;
    QString unit;
    bool channelNameAvailable = false;

    if (m_wellPath)
    {
        name.push_back(wellName());
        name.push_back("LAS");

        if (!m_wellLogChannnelName().isEmpty())
        {
            name.push_back(m_wellLogChannnelName);
            channelNameAvailable = true;
        }

        RigWellLogFile* wellLogFile = m_wellLogFile ? m_wellLogFile->wellLogFile() : nullptr;

        if (wellLogFile)
        {
            if (channelNameAvailable)
            {
                RimWellLogPlot* wellLogPlot;
                firstAncestorOrThisOfType(wellLogPlot);
                CVF_ASSERT(wellLogPlot);
                QString unitName = wellLogFile->wellLogChannelUnitString(m_wellLogChannnelName, wellLogPlot->depthUnit());

                if (!unitName.isEmpty())
                {
                    name.back() += QString(" [%1]").arg(unitName);
                }
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
QString RimWellLogFileCurve::wellLogChannelName() const
{
    return m_wellLogChannnelName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellLogFileCurve::wellLogFile() const
{
    return m_wellLogFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::wellName() const
{
    return m_wellPath->name();
}
