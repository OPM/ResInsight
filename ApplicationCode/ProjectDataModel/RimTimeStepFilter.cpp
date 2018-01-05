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

#include "RimEclipseResultCase.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"

#include <QDateTime>
#include "RigCaseCellResultsData.h"

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

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeStepIndices, "TimeStepIndicesToImport", "Values", "", "", ""); 

    CAF_PDM_InitField(&m_firstTimeStep, "FirstTimeStep", 0, "First Time Step", "", "", "");
    CAF_PDM_InitField(&m_lastTimeStep, "LastTimeStep", 0, "Last Time Step", "", "", "");

    caf::AppEnum< RimTimeStepFilter::TimeStepFilterTypeEnum > filterType = TS_ALL;
    CAF_PDM_InitField(&m_filterType, "FilterType", filterType, "Filter Type", "", "", "");

    CAF_PDM_InitField(&m_interval, "Interval", 1, "Interval", "", "", "");

    CAF_PDM_InitField(&m_filteredTimeStepsText, "FilteredTimeSteps", QString(), "Filtered TimeSteps", "", "", "");
    m_filteredTimeStepsText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_filteredTimeStepsText.uiCapability()->setUiReadOnly(true);
    m_filteredTimeStepsText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_filteredTimeStepsText.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_applyReloadOfCase, "ApplyReloadOfCase", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_applyReloadOfCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setTimeStepsFromFile(const std::vector<QDateTime>& timeSteps)
{
    m_timeStepsFromFile = timeSteps;

    m_lastTimeStep = static_cast<int>(timeSteps.size()) - 1;

    updateSelectedTimeStepIndices();
    updateDerivedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::clearTimeStepsFromFile()
{
    m_timeStepsFromFile.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimTimeStepFilter::filteredNativeTimeStepIndices() const
{
    std::vector<size_t> indices;

    // Convert vector from int to size_t 
    for (auto intValue : m_selectedTimeStepIndices.v())
    {
        indices.push_back(intValue); 
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseResultCase* rimEclipseResultCase = parentEclipseResultCase();

    if (changedField == &m_applyReloadOfCase)
    {
        if (rimEclipseResultCase)
        {
            rimEclipseResultCase->reloadDataAndUpdate();
        }

        return;
    }

    updateSelectedTimeStepIndices();
    updateDerivedData();

    if (rimEclipseResultCase)
    {
        rimEclipseResultCase->updateConnectedEditors();
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
        std::vector<QDateTime> timeSteps = allTimeSteps();

        QString formatString = RimTools::createTimeFormatStringFromDates(timeSteps);

        for (size_t i = 0; i < timeSteps.size(); i++)
        {
            optionItems.push_back(caf::PdmOptionItemInfo(timeSteps[i].toString(formatString), static_cast<int>(i)));
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTimeStepFilter::filteredTimeStepsAsText() const
{
    QString text;

    std::vector<QDateTime> timeSteps = allTimeSteps();

    QString formatString = RimTools::createTimeFormatStringFromDates(timeSteps);

    for (auto selectedIndex : m_selectedTimeStepIndices.v())
    {
        size_t timeStepIndex = static_cast<size_t>(selectedIndex);

        if (timeStepIndex < timeSteps.size())
        {
            text += timeSteps[timeStepIndex].toString(formatString);
            text += "\n";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::updateDerivedData()
{
    m_filteredTimeStepsText = filteredTimeStepsAsText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::updateSelectedTimeStepIndices()
{
    m_selectedTimeStepIndices = selectedTimeStepIndicesFromUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimTimeStepFilter::allTimeSteps() const
{
    RimEclipseResultCase* rimEclipseResultCase = parentEclipseResultCase();
    if (rimEclipseResultCase && rimEclipseResultCase->results(RiaDefines::MATRIX_MODEL))
    {
        return  rimEclipseResultCase->results(RiaDefines::MATRIX_MODEL)->allTimeStepDatesFromEclipseReader();
    }

    return m_timeStepsFromFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RimTimeStepFilter::selectedTimeStepIndicesFromUi() const
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


        std::vector<QDateTime> timeSteps = allTimeSteps();

        indices.push_back(m_firstTimeStep);
        QDateTime d = timeSteps[m_firstTimeStep].addDays(daysToSkip);

        for (int i = m_firstTimeStep + 1; i <= m_lastTimeStep; i++)
        {
            if (timeSteps[i] > d)
            {
                d = d.addDays(daysToSkip);
                indices.push_back(i);
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
void RimTimeStepFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_firstTimeStep);
    uiOrdering.add(&m_lastTimeStep);
    uiOrdering.add(&m_filterType);
    uiOrdering.add(&m_interval);

    if (m_timeStepsFromFile.size() == 0)
    {
        uiOrdering.add(&m_applyReloadOfCase);
    }

    QString displayUiName = QString("Filtered Time Steps (%1)").arg(m_selectedTimeStepIndices().size());

    caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword(displayUiName, "FilteredTimeStepKeyword");
    group->add(&m_filteredTimeStepsText);

    if (m_timeStepsFromFile.size() == 0)
    {
        group->setCollapsedByDefault(true);
    }
    else
    {
        group->setCollapsedByDefault(false);
    }

    updateDerivedData();
    updateFieldVisibility();

    uiOrdering.skipRemainingFields();
}
