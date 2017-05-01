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

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuWellLogTrack.h"
#include "RiuLineSegmentQwtPlotCurve.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QMessageBox>


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

    m_wellPath = NULL;
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
void RimWellLogFileCurve::onLoadDataAndUpdate()
{
    RimWellLogCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        m_curveData = new RigWellLogCurveData;

        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);

        if (wellLogPlot && wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            if (RiaApplication::instance()->preferences()->showLasCurveWithoutTvdWarning())
            {
                QString tmp = QString("Display of True Vertical Depth (TVD) for LAS curves in not yet supported, and no LAS curve will be displayed in this mode.\n\n");
                tmp += "Control display of this warning from \"Preferences->Show LAS curve without TVD warning\"";
            
                QMessageBox::warning(NULL, "LAS curve without TVD", tmp);
            }
        }
        else if (m_wellPath)
        {
            RimWellLogFile* logFileInfo = m_wellPath->m_wellLogFile;
            if (logFileInfo)
            {
                RigWellLogFile* wellLogFile = logFileInfo->wellLogFile();
                if (wellLogFile)
                {
                    std::vector<double> values = wellLogFile->values(m_wellLogChannnelName);
                    std::vector<double> depthValues = wellLogFile->depthValues();

                    if (values.size() == depthValues.size())
                    {
                        m_curveData->setValuesAndMD(values, depthValues, wellLogFile->depthUnit(), false);
                    }
                }

                if (m_isUsingAutoName)
                {
                    m_qwtPlotCurve->setTitle(createCurveAutoName());
                }
            }
        }

        RimDefines::DepthUnitType displayUnit = RimDefines::UNIT_METER;
        if (wellLogPlot)
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->measuredDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
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
void RimWellLogFileCurve::setWellLogChannelName(const QString& name)
{
    m_wellLogChannnelName = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellPath)
    {
        this->loadDataAndUpdate();
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->loadDataAndUpdate();
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
                if (wellPaths[i]->m_wellLogFile())
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
            RimWellLogFile* wellLogFile = m_wellPath->m_wellLogFile();
            if (wellLogFile)
            {
                const caf::PdmChildArrayField<RimWellLogFileChannel*>* fileLogs = wellLogFile->wellLogChannelNames();

                for (size_t i = 0; i < fileLogs->size(); i++)
                {
                    QString wellLogChannelName = (*fileLogs)[i]->name();
                    options.push_back(caf::PdmOptionItemInfo(wellLogChannelName, wellLogChannelName));
                }
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", "None"));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::createCurveAutoName()
{
    if (m_wellPath)
    {
        QString txt;

        txt += wellName();
        txt += " : ";
        txt += m_wellLogChannnelName;

        RimWellLogFile* logFileInfo = m_wellPath->m_wellLogFile;
        RigWellLogFile* wellLogFile = logFileInfo ? logFileInfo->wellLogFile() : NULL;
        if (wellLogFile)
        {
            RimWellLogPlot* wellLogPlot;
            firstAncestorOrThisOfType(wellLogPlot);
            CVF_ASSERT(wellLogPlot);

            QString unitName = wellLogFile->wellLogChannelUnitString(m_wellLogChannnelName, wellLogPlot->depthUnit());
            if (!unitName.isEmpty())
            {
                txt += QString(" [%1]").arg(unitName);
            }
        }

        return txt;
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
QString RimWellLogFileCurve::wellName() const
{
    return m_wellPath->name();
}
