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
#include "RimWellLogTrack.h"
#include "RimWellLogPlot.h"

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
    m_wellPath.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannnelName, "CurveWellLogChannel", "Well Log Channel", "", "", "");

    m_wellPath = NULL;

    updateOptionSensitivity();
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
    RimWellLogCurve::updatePlotConfiguration();

    if (isCurveVisible())
    {
        m_curveData = new RigWellLogCurveData;

        RimWellLogPlot* wellLogPlot;
        firstAnchestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);

        if (wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
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

                if (m_autoName)
                {
                    m_qwtPlotCurve->setTitle(createCurveName());
                }
            }
        }

        m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->depthPlotValues().data(), static_cast<int>(m_curveData->xPlotValues().size()));
        m_qwtPlotCurve->setLineSegmentStartStopIndices(m_curveData->polylineStartStopIndices());

        zoomAllOwnerTrackAndPlot();

        if (m_ownerQwtTrack) m_ownerQwtTrack->replot();
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
        this->updatePlotData();
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->updatePlotData();
    }

    if (m_ownerQwtTrack) m_ownerQwtTrack->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_wellPath);
    curveDataGroup->add(&m_wellLogChannnelName);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_curveColor);
    appearanceGroup->add(&m_curveThickness);
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_autoName);
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

    optionList = RimWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    if (optionList.size() > 0) return optionList;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = proj->activeOilField()->wellPathCollection()->wellPaths;

            for (size_t i = 0; i < wellPaths.size(); i++)
            {
                if (wellPaths[i]->m_wellLogFile())
                {
                    optionList.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i]))));
                }
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
            QString unitName = wellLogFile->wellLogChannelUnitString(m_wellLogChannnelName);
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
