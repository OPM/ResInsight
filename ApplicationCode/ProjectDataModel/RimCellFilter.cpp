/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimCellFilter.h"

#include "cafAppEnum.h"

#include <QPainter>

namespace caf
{
    template<>
    void caf::AppEnum< RimCellFilter::FilterModeType>::setUp()
    {
        addItem(RimCellFilter::INCLUDE, "INCLUDE",   "Include");
        addItem(RimCellFilter::EXCLUDE,  "EXCLUDE",   "Exclude");
        setDefault(RimCellFilter::INCLUDE);
    }
}


CAF_PDM_SOURCE_INIT(RimCellFilter, "CellFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellFilter::RimCellFilter()
{
    CAF_PDM_InitObject("Cell Filter", "", "", "");

    CAF_PDM_InitField(&name,    "UserDescription",  QString("Filter Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,  "Active",           true,                   "Active",   "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_selectedCategoryValues, "SelectedValues", "Values", "", "", "");

    CAF_PDM_InitFieldNoDefault(&filterMode, "FilterType", "Filter Type", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellFilter::~RimCellFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilter::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RimCellFilter::selectedCategoryValues() const
{
    return m_selectedCategoryValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellFilter::updateIconState()
{
    // Reset dynamic icon
    this->setUiIcon(QIcon());
    // Get static one
    QIcon icon = this->uiIcon();

    // Get a pixmap, and modify it

    QPixmap icPixmap;
    icPixmap = icon.pixmap(16, 16, QIcon::Normal);

    QPixmap sign;
    if (filterMode() == INCLUDE)
    {
        sign.load(":/Plus.png");
    }
    else
    {
        sign.load(":/Minus.png");
    }

    {
        QPainter painter(&icPixmap);
        painter.drawPixmap(0,0, sign);
    }

    if (!isActive || isActive.uiCapability()->isUiReadOnly())
    {
        QIcon temp(icPixmap);
        icPixmap = temp.pixmap(16, 16, QIcon::Disabled);
    }

    QIcon newIcon(icPixmap);
    this->setUiIcon(newIcon);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilter::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCellFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (&m_selectedCategoryValues == fieldNeedingOptions)
    {
        if (useOptionsOnly) *useOptionsOnly = true;

        if (m_categoryValues.size() == m_categoryNames.size())
        {
            for (size_t i = 0; i < m_categoryValues.size(); i++)
            {
                int categoryValue = m_categoryValues[i];
                QString categoryName = m_categoryNames[i];

                optionList.push_back(caf::PdmOptionItemInfo(categoryName, categoryValue));
            }
        }
        else
        {
            for (auto it : m_categoryValues)
            {
                QString str = QString::number(it);
                optionList.push_back(caf::PdmOptionItemInfo(str, it));
            }
        }
    }

    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setCategoryValues(const std::vector<int>& categoryValues)
{
    m_categoryValues = categoryValues;
    m_categoryNames.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setCategoryNames(const std::vector<QString>& categoryNames)
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
void RimCellFilter::clearCategories()
{
    m_categoryValues.clear();
    m_categoryNames.clear();
}
