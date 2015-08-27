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

#include "RimManagedViewCollection.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimManagedViewConfig.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "RimGeoMechView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechCellColors.h"



CAF_PDM_SOURCE_INIT(RimManagedViewCollection, "RimManagedViewCollection");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimManagedViewCollection::RimManagedViewCollection(void)
{
    CAF_PDM_InitObject("Managed Views", ":/chain.png", "", "");

    CAF_PDM_InitFieldNoDefault(&managedViews, "ManagedViews", "Managed Views", "", "", "");
    managedViews.push_back(new RimManagedViewConfig);
    managedViews.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimManagedViewCollection::~RimManagedViewCollection(void)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewCollection::updateViewers(RiuViewer* masterViewer)
{
    for (size_t i = 0; i < managedViews.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = managedViews[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncCamera() && managedViewConfig->managedView()->viewer())
            {
                const cvf::Mat4d mat = masterViewer->mainCamera()->viewMatrix();

                managedViewConfig->managedView()->viewer()->mainCamera()->setViewMatrix(mat);
                managedViewConfig->managedView()->viewer()->update();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewCollection::updateTimeStep(int timeStep)
{
    for (size_t i = 0; i < managedViews.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = managedViews[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncTimeStep() && managedViewConfig->managedView()->viewer())
            {
                managedViewConfig->managedView()->viewer()->slotSetCurrentFrame(timeStep);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewCollection::updateCellResult()
{
    RimView* masterView = NULL;
    firstAnchestorOrThisOfType(masterView);

    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(masterView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (size_t i = 0; i < managedViews.size(); i++)
        {
            RimManagedViewConfig* managedViewConfig = managedViews[i];
            if (managedViewConfig->managedView())
            {
                if (managedViewConfig->syncCellResult())
                {
                    RimView* rimView = managedViewConfig->managedView();
                    RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                    if (eclipeView)
                    {
                        eclipeView->cellResult()->setPorosityModel(eclipseCellResultDefinition->porosityModel());
                        eclipeView->cellResult()->setResultType(eclipseCellResultDefinition->resultType());
                        eclipeView->cellResult()->setResultVariable(eclipseCellResultDefinition->resultVariable());
                    }
                }
            }
        }
    }

    RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>(masterView);
    if (masterGeoView && masterGeoView->cellResult())
    {
        RimGeoMechResultDefinition* geoMechResultDefinition = masterGeoView->cellResult();

        for (size_t i = 0; i < managedViews.size(); i++)
        {
            RimManagedViewConfig* managedViewConfig = managedViews[i];
            if (managedViewConfig->managedView())
            {
                if (managedViewConfig->syncCellResult())
                {
                    RimView* rimView = managedViewConfig->managedView();
                    RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                    if (geoView)
                    {
                        geoView->cellResult()->setResultAddress(geoMechResultDefinition->resultAddress());
                        geoView->scheduleCreateDisplayModelAndRedraw();
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewCollection::updateRangeFilters()
{
    for (size_t i = 0; i < managedViews.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = managedViews[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncRangeFilters())
            {
                RimView* rimView = managedViewConfig->managedView();
                RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                if (eclipeView)
                {
                    eclipeView->scheduleGeometryRegen(RANGE_FILTERED);
                    eclipeView->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

                    eclipeView->scheduleCreateDisplayModelAndRedraw();
                }

                RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                if (geoView)
                {
                    geoView->scheduleGeometryRegen(RANGE_FILTERED);
                    geoView->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

                    geoView->scheduleCreateDisplayModelAndRedraw();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewCollection::updatePropertyFilters()
{
    for (size_t i = 0; i < managedViews.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = managedViews[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncPropertyFilters())
            {
                RimView* rimView = managedViewConfig->managedView();
                RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                if (eclipeView)
                {
                    eclipeView->scheduleGeometryRegen(PROPERTY_FILTERED);
                    eclipeView->scheduleCreateDisplayModelAndRedraw();
                }

/*
                RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                if (geoView)
                {
                    geoView->scheduleGeometryRegen(RANGE_FILTERED);
                    geoView->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

                    geoView->scheduleCreateDisplayModelAndRedraw();
                }
*/
            }
        }
    }
}
