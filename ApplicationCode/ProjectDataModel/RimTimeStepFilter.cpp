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

#include "RimCase.h"
#include "RimTools.h"

#include <QDateTime>

CAF_PDM_SOURCE_INIT(RimTimeStepFilter, "TimeStepFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTimeStepFilter::RimTimeStepFilter()
{
    CAF_PDM_InitObject("Time Step Filter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeStepIndices, "TimeStepIndicesToImport", "Values", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTimeStepFilter::setCustomTimeSteps(const std::vector<QDateTime>& timeSteps)
{
    m_customTimeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimTimeStepFilter::selectedTimeStepIndices() const
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
void RimTimeStepFilter::setSelectedTimeStepIndices(const std::vector<size_t>& indices)
{
    m_selectedTimeStepIndices.v().clear();

    for (auto sizetValue : indices)
    {
        m_selectedTimeStepIndices.v().push_back(static_cast<int>(sizetValue));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimTimeStepFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionItems;

    if (fieldNeedingOptions == &m_selectedTimeStepIndices)
    {
        RimCase* rimCase = nullptr;
        this->firstAncestorOrThisOfType(rimCase);
        if (rimCase)
        {
            QStringList timeSteps = rimCase->timeStepStrings();

            for (int i = 0; i < timeSteps.size(); i++)
            {
                optionItems.push_back(caf::PdmOptionItemInfo(timeSteps[i], static_cast<int>(i)));
            }
        }
        else
        {
            QString formatString = RimTools::createTimeFormatStringFromDates(m_customTimeSteps);

            for (size_t i = 0; i < m_customTimeSteps.size(); i++)
            {
                optionItems.push_back(caf::PdmOptionItemInfo(m_customTimeSteps[i].toString(formatString), static_cast<int>(i)));
            }
        }
    }

    return optionItems;
}
