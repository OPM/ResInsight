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

#include "RimProject.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimWellLog.h"
#include "RimWellLasFileInfo.h"
#include "RimWellLogPlotTrace.h"
#include "RimWellLogPlot.h"

#include "RiuWellLogTracePlot.h"

#include "RiaApplication.h"

#include "cafPdmUiTreeOrdering.h"

#include "qwt_plot_curve.h"


CAF_PDM_SOURCE_INIT(RimWellLogFileCurve, "WellLogFileCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::RimWellLogFileCurve()
{
    CAF_PDM_InitObject("Well Log File Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Path", "", "", "");
    m_wellPath.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannnelName, "CurveWellLogChannel", "Well Log Channel", "", "", "");

    m_userName.uiCapability()->setUiHidden(true);

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
void RimWellLogFileCurve::updatePlotData()
{
    RimWellLogPlotCurve::updatePlotData();

    if (m_showCurve)
    {
        if (m_wellPath)
        {
            RimWellLasFileInfo* logFileInfo = m_wellPath->m_lasFileInfo;
            if (logFileInfo)
            {
                RigWellLogFile* wellLogFile = logFileInfo->wellLogFile();
                m_plotCurve->setSamples(wellLogFile->values(m_wellLogChannnelName).data(), wellLogFile->depthValues().data(), (int) wellLogFile->depthValues().size());
                m_plotCurve->setTitle(m_wellLogChannnelName);
            }
        }
        else
        {
            m_plotCurve->setSamples(NULL, NULL, 0);
            m_plotCurve->setTitle("None");
        }
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
    RimWellLogPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellPath)
    {
        this->updatePlotData();

        RimWellLogPlot* wellLoglot;
        firstAnchestorOrThisOfType(wellLoglot);
        if (wellLoglot)
        {
            wellLoglot->updateAvailableDepthRange();
            wellLoglot->setVisibleDepthRangeFromContents();
        }
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->updatePlotData();

        RimWellLogPlot* wellLoglot;
        firstAnchestorOrThisOfType(wellLoglot);
        if (wellLoglot)
        {
            if (!wellLoglot->hasAvailableDepthRange())
            {
                wellLoglot->updateAvailableDepthRange();
                wellLoglot->setVisibleDepthRangeFromContents();
            }
        }
    }

    m_plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_wellPath);
    uiOrdering.add(&m_wellLogChannnelName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogFileCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = proj->activeOilField()->wellPathCollection()->wellPaths;

            for (size_t i = 0; i < wellPaths.size(); i++)
            {
                optionList.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i]))));
            }

            if (optionList.size() > 0)
            {
                optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
            }
        }
    }

    if (fieldNeedingOptions == &m_wellLogChannnelName)
    {
        if (m_wellPath())
        {
            RimWellLasFileInfo* lasFileInfo = m_wellPath->m_lasFileInfo();
            if (lasFileInfo)
            {
                const caf::PdmChildArrayField<RimWellLog*>* fileLogs = lasFileInfo->lasFileLogs();

                for (size_t i = 0; i < fileLogs->size(); i++)
                {
                    QString wellLogChannelName = (*fileLogs)[i]->name();
                    optionList.push_back(caf::PdmOptionItemInfo(wellLogChannelName, wellLogChannelName));

                }
            }
        }
    }

    return optionList;
}

