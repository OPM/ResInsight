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

#include "RimGeoMechCellColors.h"

#include "RimLegendConfig.h"
#include "RimView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"


CAF_PDM_SOURCE_INIT(RimGeoMechCellColors, "GeoMechResultSlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCellColors::RimGeoMechCellColors(void)
{
    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();
    legendConfig.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCellColors::~RimGeoMechCellColors(void)
{
    delete legendConfig;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCellColors::updateIconState()
{
    RimView* rimView = NULL;
    this->firstAncestorOrThisOfType(rimView);
    CVF_ASSERT(rimView);

    if (rimView)
    {
        RimViewController* viewController = rimView->viewController();
        if (viewController && viewController->isResultColorControlled())
        {
            updateUiIconFromState(false);
        }
        else
        {
            updateUiIconFromState(true);
        }
    }

    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCellColors::initAfterRead()
{
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCellColors::updateLegendCategorySettings()
{
    if(this->hasCategoryResult())
    {
        legendConfig->setMappingMode(RimLegendConfig::CATEGORY_INTEGER);
        legendConfig->setColorRangeMode(RimLegendConfig::CATEGORY);
    }
    else
    {
        if(legendConfig->mappingMode() == RimLegendConfig::CATEGORY_INTEGER)
        {
            legendConfig->setMappingMode(RimLegendConfig::LINEAR_CONTINUOUS);
        }

        if(legendConfig->colorRangeMode() == RimLegendConfig::CATEGORY)
        {
            legendConfig->setColorRangeMode(RimLegendConfig::NORMAL);
        }
    }
}
