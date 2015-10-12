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
#include "RimWellLogExtractionCurveImpl.h"

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
    RimWellLogPlotCurve::updatePlotConfiguration();

    if (isCurveVisibile())
    {
        if (m_wellPath)
        {
            RimWellLogFile* logFileInfo = m_wellPath->m_wellLogFile;
            if (logFileInfo)
            {
                RigWellLogFile* wellLogFile = logFileInfo->wellLogFile();
                if (wellLogFile)
                {
                    std::vector<double> values = wellLogFile->values(m_wellLogChannnelName);
                    std::vector<double> depthValues = wellLogFile->depthValues();

                    if (values.size() > 0 && depthValues.size() > 0)
                    {   
                        std::vector< std::pair<size_t, size_t> > valuesIntervals;
                        RimWellLogExtractionCurveImpl::calculateIntervalsOfValidValues(values, valuesIntervals);

                        std::vector<double> filteredValues;
                        std::vector<double> filteredDepths;
                        RimWellLogExtractionCurveImpl::addValuesFromIntervals(values, valuesIntervals, &filteredValues);
                        RimWellLogExtractionCurveImpl::addValuesFromIntervals(depthValues, valuesIntervals, &filteredDepths);

                        std::vector< std::pair<size_t, size_t> > fltrIntervals;
                        RimWellLogExtractionCurveImpl::filteredIntervals(valuesIntervals, &fltrIntervals);

                        m_plotCurve->setSamples(filteredValues.data(), filteredDepths.data(), (int)filteredDepths.size());
                        m_plotCurve->setPlotIntervals(fltrIntervals);
                    }
                    else
                    {
                        m_plotCurve->setSamples(NULL, NULL, 0);
                    }
                }

                if (m_autoName)
                {
                    m_plotCurve->setTitle(createCurveName());
                }
            }
        }
        else
        {
            m_plotCurve->setSamples(NULL, NULL, 0);
        }

        updateTrackAndPlotFromCurveData();

        if (m_plot) m_plot->replot();
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
    }
    else if (changedField == &m_wellLogChannnelName)
    {
        this->updatePlotData();
    }

    if (m_plot) m_plot->replot();
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
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_autoName);
    appearanceGroup->add(&m_curveColor);
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

        txt += m_wellPath()->name();
        txt += " : ";
        txt += m_wellLogChannnelName;

        RimWellLogFile* logFileInfo = m_wellPath->m_wellLogFile;
        RigWellLogFile* wellLogFile = logFileInfo ? logFileInfo->wellLogFile() : NULL;
        if (wellLogFile)
        {
            QString unitName = wellLogFile->wellLogChannelUnit(m_wellLogChannnelName);
            if (!unitName.isEmpty())
            {
                txt += QString(" [%1]").arg(unitName);
            }
        }

        return txt;
    }
    
    return "Empty curve";
}

