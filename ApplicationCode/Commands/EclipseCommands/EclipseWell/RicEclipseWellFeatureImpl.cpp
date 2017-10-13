/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicEclipseWellFeatureImpl.h"

#include "RimEclipseWellCollection.h"
#include "RimSimWellInView.h"

#include "cafSelectionManager.h"

#include <QAction>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellFeatureImpl::isAnyWellSelected()
{
    std::vector<RimSimWellInView*> selection = selectedWells();
    if (selection.size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSimWellInView*> RicEclipseWellFeatureImpl::selectedWells()
{
    std::vector<RimSimWellInView*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection* RicEclipseWellFeatureImpl::wellCollectionFromSelection()
{
    std::vector<RimSimWellInView*> selection = selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* firstWell = selection[0];

        RimEclipseWellCollection* wellCollection = nullptr;
        firstWell->firstAncestorOrThisOfType(wellCollection);

        return wellCollection;
    }

    return nullptr;
}

