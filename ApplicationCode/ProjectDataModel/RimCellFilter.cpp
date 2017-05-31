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

