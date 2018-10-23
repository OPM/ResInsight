/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimPropertyFilter.h"

#include "RimPropertyFilterCollection.h"

CAF_PDM_SOURCE_INIT(RimPropertyFilter, "PropertyFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPropertyFilter::RimPropertyFilter()
{
    CAF_PDM_InitObject("Property Filter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedCategoryValues, "SelectedValues", "Values", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPropertyFilter::~RimPropertyFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RimPropertyFilter::selectedCategoryValues() const
{
    return m_selectedCategoryValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPropertyFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (&m_selectedCategoryValues == fieldNeedingOptions)
    {
        if (useOptionsOnly) *useOptionsOnly = true;

        if (m_categoryValues.size() == m_categoryNames.size())
        {
            for (size_t i = 0; i < m_categoryValues.size(); i++)
            {
                int categoryValue = m_categoryValues[i];
                QString categoryName = m_categoryNames[i];

                options.push_back(caf::PdmOptionItemInfo(categoryName, categoryValue));
            }
        }
        else
        {
            for (auto it : m_categoryValues)
            {
                QString str = QString::number(it);
                options.push_back(caf::PdmOptionItemInfo(str, it));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilter::setCategoryValues(const std::vector<int>& categoryValues)
{
    m_categoryValues = categoryValues;
    m_categoryNames.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilter::setCategoryNames(const std::vector<QString>& categoryNames)
{
    m_categoryNames = categoryNames;

    for (size_t i = 0; i < m_categoryNames.size(); i++)
    {
        m_categoryValues.push_back(static_cast<int>(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilter::setCategoryNamesAndValues(const std::vector<std::pair<QString, int>>& categoryNamesAndValues)
{
    clearCategories();

    for (auto it : categoryNamesAndValues)
    {
        m_categoryNames.push_back(it.first);
        m_categoryValues.push_back(it.second);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilter::clearCategories()
{
    m_categoryValues.clear();
    m_categoryNames.clear();
}
