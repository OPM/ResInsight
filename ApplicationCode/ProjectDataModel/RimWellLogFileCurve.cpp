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
#include "RimWellLogFileChannel.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellLogPlot.h"

#include "RiuWellLogTrackPlot.h"
#include "RiuWellLogPlotCurve.h"

#include "RiaApplication.h"

#include "cafPdmUiTreeOrdering.h"


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
    RimWellLogPlotCurve::updatePlotConfiguration();

    if (isCurveVisibile())
    {
        if (m_wellPath)
        {
            RimWellLogFile* logFileInfo = m_wellPath->m_wellLogFile;
            if (logFileInfo)
            {
                RigWellLogFile* wellLogFile = logFileInfo->wellLogFile();

                std::vector<double> values = wellLogFile->values(m_wellLogChannnelName);
                std::vector<double> depthValues = wellLogFile->depthValues();

                if (values.size() > 0 && depthValues.size() > 0)
                {
                    m_plotCurve->setSamples(values.data(), depthValues.data(), (int)depthValues.size());
                }
                else
                {
                    m_plotCurve->setSamples(NULL, NULL, 0);
                }
            }
        }
        else
        {
            m_plotCurve->setSamples(NULL, NULL, 0);
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

    RimWellLogPlot* wellLoglot;
    firstAnchestorOrThisOfType(wellLoglot);

    RimWellLogPlotTrack* wellLoglotTrack;
    firstAnchestorOrThisOfType(wellLoglotTrack);

    if (changedField == &m_wellPath)
    {
        this->updatePlotData();

        if (wellLoglot)
        {
            wellLoglot->updateAvailableDepthRange();
            wellLoglot->setVisibleDepthRangeFromContents();
        }

        if (wellLoglotTrack)
        {
            wellLoglotTrack->updateXAxisRangeFromCurves();
        }
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->updatePlotData();

        if (wellLoglot)
        {
            if (!wellLoglot->hasAvailableDepthRange())
            {
                wellLoglot->updateAvailableDepthRange();
                wellLoglot->setVisibleDepthRangeFromContents();
            }

            if (wellLoglotTrack)
            {
                wellLoglotTrack->updateXAxisRangeFromCurves();
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
    RimWellLogPlotCurve::defineUiOrdering(uiConfigName, uiOrdering);
    
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
            RimWellLogFile* wellLogFile = m_wellPath->m_wellLogFile();
            if (wellLogFile)
            {
                const caf::PdmChildArrayField<RimWellLogFileChannel*>* fileLogs = wellLogFile->wellLogChannelNames();

                for (size_t i = 0; i < fileLogs->size(); i++)
                {
                    QString wellLogChannelName = (*fileLogs)[i]->name();
                    optionList.push_back(caf::PdmOptionItemInfo(wellLogChannelName, wellLogChannelName));
                }
            }
        }

        if (optionList.size() == 0)
        {
            optionList.push_back(caf::PdmOptionItemInfo("None", "None"));
        }
    }

    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::createCurveName()
{
    if (m_wellPath())
    {
        QString txt;

        txt += m_wellPath()->name();
        txt += " : ";
        txt += m_wellLogChannnelName;

        return txt;
    }
    
    return "Empty curve";
}

