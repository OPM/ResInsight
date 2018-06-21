/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimTimeStepFilter.h"

#include "RifReaderEclipseOutput.h"
#include "RigCaseCellResultsData.h"

#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QDateTime>

#include <algorithm>

namespace caf {

    template<>
    void caf::AppEnum< RimTimeStepFilter::TimeStepFilterTypeEnum >::setUp()
    {
        addItem(RimTimeStepFilter::TS_ALL,                  "TS_ALL",               "All");
        addItem(RimTimeStepFilter::TS_INTERVAL_DAYS,        "TS_INTERVAL_DAYS",     "Skip by Days");
        addItem(RimTimeStepFilter::TS_INTERVAL_WEEKS,       "TS_INTERVAL_WEEKS",    "Skip by Weeks");
        addItem(RimTimeStepFilter::TS_INTERVAL_MONTHS,      "TS_INTERVAL_MONTHS",   "Skip by Months");
        addItem(RimTimeStepFilter::TS_INTERVAL_QUARTERS,    "TS_INTERVAL_QUARTERS", "Skip by Quarters");
        addItem(RimTimeStepFilter::TS_INTERVAL_YEARS,       "TS_INTERVAL_YEARS",    "Skip by Years");

        setDefault(RimTimeStepFilter::TS_ALL);
    }

} // End namespace caf


CAF_PDM_SOURCE_INIT(RimTimeStepFilter, "RimTimeStepFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTimeStepFilter::RimTimeStepFilter()
{
    CAF_PDM_InitObject("Time Step Filter", "", "", "");

    caf::AppEnum< RimTimeStepFilter::TimeStepFilterTypeEnum > filterType = TS_ALL;
    CAF_PDM_InitField(&m_filterType, "FilterType", filterType, "Filter Type", "", "", "");

    CAF_PDM_InitField(&m_firstTimeStep, "FirstTimeStep", 0, "First Time Step", "", "", "");
    CAF_PDM_InitField(&m_lastTimeStep, "LastTimeStep", 0, "Last Time Step", "", "", "");

    CAF_PDM_InitField(&m_interval, "Interval", 1, "Interval", "", "", "");
    m_interval.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());    

    CAF_PDM_InitField(&m_timeStepNamesFromFile, "TimeStepsFromFile", std::vector<QString>(), "TimeSteps From File", "", "", "");
    CAF_PDM_InitField(&m_dateFormat, "DateFormat", QString("yyyy-MM-dd"), "Date Format", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_filteredTimeSteps, "TimeStepIndicesToImport", "Select From Time Steps", "", "", "");
    m_filteredTimeSteps.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_filteredTimeStepsUi, "TimeStepIndicesUi", "Select From TimeSteps", "", "", "");
    m_filteredTimeStepsUi.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_filteredTimeStepsUi.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_filteredTimeStepsUi.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_applyReloadOfCase, "ApplyReloadOfCase", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_applyReloadOfCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setTimeStepsFromFile(const std::vector<QDateTime>& timeSteps)
{
    m_dateFormat = RimTools::createTimeFormatStringFromDates(timeSteps);
    std::vector<QString> timeStepStrings;
    for (const QDateTime& date : timeSteps)
    {
        timeStepStrings.push_back(date.toString(m_dateFormat));
    }

    m_timeStepNamesFromFile = timeStepStrings;
    m_lastTimeStep = static_cast<int>(timeSteps.size()) - 1;

    if (m_filteredTimeSteps().empty())
    {
        m_filteredTimeSteps = filteredTimeStepIndicesFromUi();        
    }
    m_filteredTimeStepsUi = m_filteredTimeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setTimeStepsFromFile(const std::vector<std::pair<QString, QDateTime>>& timeSteps)
{
    std::vector<QDateTime> validDates;
    for (auto stringDatePair : timeSteps)
    {
        if (stringDatePair.second.isValid())
        {
            validDates.push_back(stringDatePair.second);
        }
    }
    m_dateFormat = RimTools::createTimeFormatStringFromDates(validDates);
    
    std::vector<QString> timeStepStrings;
    for (auto stringDatePair : timeSteps)
    {
        QString stepString = stringDatePair.first;
        if (stringDatePair.second.isValid())
        {
            stepString = stringDatePair.second.toString(m_dateFormat);
        }
        timeStepStrings.push_back(stepString);
    }

    m_timeStepNamesFromFile = timeStepStrings;
    m_lastTimeStep = static_cast<int>(timeSteps.size()) - 1;

    if (m_filteredTimeSteps().empty())
    {
        m_filteredTimeSteps = filteredTimeStepIndicesFromUi();
    }
    m_filteredTimeStepsUi = m_filteredTimeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimTimeStepFilter::filteredTimeSteps() const
{
    std::vector<size_t> indices;

    // Convert vector from int to size_t 
    for (int index : m_filteredTimeSteps())
    {
        indices.push_back(static_cast<size_t>(index));
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTimeStepFilter::updateFilteredTimeStepsFromUi()
{
    std::vector<int> timeSteps = m_filteredTimeStepsUi;
    std::sort(timeSteps.begin(), timeSteps.end());
    if (m_filteredTimeSteps() == timeSteps)
    {
        return false;
    }
    m_filteredTimeSteps = timeSteps;    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseResultCase* rimEclipseResultCase = parentEclipseResultCase();
    RimGeoMechCase* rimGeoMechCase = parentGeoMechCase();
    if (changedField == &m_applyReloadOfCase)
    {
        if (updateFilteredTimeStepsFromUi())
        {

            if (rimEclipseResultCase)
            {
                rimEclipseResultCase->reloadDataAndUpdate();
            }
            else if (rimGeoMechCase)
            {
                rimGeoMechCase->reloadDataAndUpdate();
            }

            return;
        }
    }

    if (changedField == &m_filterType ||
        changedField == &m_firstTimeStep ||
        changedField == &m_lastTimeStep ||
        changedField == &m_interval)
    {
        m_filteredTimeStepsUi = filteredTimeStepIndicesFromUi();
    }

    if (rimEclipseResultCase)
    {
        rimEclipseResultCase->updateConnectedEditors();
    }
    else if (rimGeoMechCase)
    {
        rimGeoMechCase->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimTimeStepFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionItems;

    if (fieldNeedingOptions == &m_firstTimeStep ||
        fieldNeedingOptions == &m_lastTimeStep)
    {
        for (size_t i = 0; i < m_timeStepNamesFromFile().size(); i++)
        {
            optionItems.push_back(caf::PdmOptionItemInfo(m_timeStepNamesFromFile()[i], static_cast<int>(i)));
        }
    }

    if (fieldNeedingOptions == &m_filteredTimeStepsUi)
    {
        std::vector<int> filteredTimeSteps = filteredTimeStepIndicesFromUi();
        for (auto filteredIndex : filteredTimeSteps)
        {
            if (filteredIndex < static_cast<int>(m_timeStepNamesFromFile().size()))
            {
                optionItems.push_back(caf::PdmOptionItemInfo(m_timeStepNamesFromFile()[filteredIndex], static_cast<int>(filteredIndex)));
            }
        }
    }

    return optionItems;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_applyReloadOfCase)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Reload Case";
        }
    }
    else if (field == &m_interval)
    {
        caf::PdmUiLineEditorAttribute* attrib = dynamic_cast<caf::PdmUiLineEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->avoidSendingEnterEventToParentWidget = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QDateTime>> RimTimeStepFilter::allTimeSteps() const
{
    std::vector<std::pair<QString, QDateTime>> timeSteps;
    for (const QString& dateString : m_timeStepNamesFromFile())
    {
        timeSteps.push_back(std::make_pair(dateString, QDateTime::fromString(dateString, m_dateFormat)));
    }
    return  timeSteps;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RimTimeStepFilter::filteredTimeStepIndicesFromUi() const
{
    std::vector<int> indices;

    if (m_filterType == TS_ALL)
    {
        for (int i = m_firstTimeStep; i <= m_lastTimeStep; i++)
        {
            indices.push_back(i);
        }
    }
    else
    {
        int intervalFactor = 1;

        if (m_filterType == TS_INTERVAL_WEEKS)
        {
            intervalFactor = 7;
        }
        else if (m_filterType == TS_INTERVAL_MONTHS)
        {
            intervalFactor = 30;
        }
        else if (m_filterType == TS_INTERVAL_QUARTERS)
        {
            intervalFactor = 90;
        }
        else if (m_filterType == TS_INTERVAL_YEARS)
        {
            intervalFactor = 365;
        }

        int daysToSkip = m_interval * intervalFactor;

        std::vector<std::pair<QString, QDateTime>> timeSteps = allTimeSteps();

        QDateTime d;
        for (int i = m_firstTimeStep; i <= m_lastTimeStep; i++)
        {
            if (!timeSteps[i].second.isValid())
            {
                indices.push_back(i);
            }
            else
            {
                if (d.isValid())
                {
                    if (timeSteps[i].second > d)
                    {
                        d = d.addDays(daysToSkip);
                        indices.push_back(i);
                    }                    
                }
                else
                {
                    d = timeSteps[i].second.addDays(daysToSkip);
                    indices.push_back(i);
                }
            }
        }      
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::updateFieldVisibility()
{
    if (m_filterType == TS_ALL)
    {
        m_interval.uiCapability()->setUiHidden(true);
    }
    else
    {
        m_interval.uiCapability()->setUiHidden(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimTimeStepFilter::parentEclipseResultCase() const
{
    RimEclipseResultCase* rimEclipseResultCase = nullptr;
    this->firstAncestorOrThisOfType(rimEclipseResultCase);

    return rimEclipseResultCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimTimeStepFilter::parentGeoMechCase() const
{
    RimGeoMechCase* rimGeoMechCase = nullptr;
    this->firstAncestorOrThisOfType(rimGeoMechCase);
    return rimGeoMechCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_filterType);
    uiOrdering.add(&m_firstTimeStep);
    uiOrdering.add(&m_lastTimeStep);
    uiOrdering.add(&m_interval);
    uiOrdering.add(&m_filteredTimeStepsUi);
    size_t numberOfFilterOptions = filteredTimeStepIndicesFromUi().size();
    QString displayUiName = QString("Select From %1 Time Steps:").arg(numberOfFilterOptions);
    m_filteredTimeStepsUi.uiCapability()->setUiName(displayUiName);

    bool caseLoaded = false;

    RimEclipseResultCase* eclipseCase = parentEclipseResultCase();
    RimGeoMechCase* geoMechCase = parentGeoMechCase();

    if (eclipseCase)
    {
        caseLoaded = eclipseCase->eclipseCaseData() != nullptr;
    }
    else if (geoMechCase)
    {
        caseLoaded = geoMechCase->geoMechData() != nullptr;
    }

    if (caseLoaded)
    {
        uiOrdering.add(&m_applyReloadOfCase);
    }

    updateFieldVisibility();

    uiOrdering.skipRemainingFields();
}
