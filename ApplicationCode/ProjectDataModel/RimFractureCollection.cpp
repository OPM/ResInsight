/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimFractureCollection.h"

#include "RimFracture.h"
#include "cafPdmObject.h"




CAF_PDM_SOURCE_INIT(RimFractureCollection, "FractureCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureCollection::RimFractureCollection(void)
{
    CAF_PDM_InitObject("Fracture collection", "", "", "");

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&fractures, "Fractures", "", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureCollection::~RimFractureCollection()
{
    fractures.deleteAllChildObjects();

}


//TODO: Trenger vi en sånn for å legge til fractures???
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// void RimIntersectionCollection::appendIntersection(RimIntersection* intersection)
// {
//     m_intersections.push_back(intersection);
// 
//     updateConnectedEditors();
//     RiuMainWindow::instance()->selectAsCurrentItem(intersection);
// 
//     RimView* rimView = NULL;
//     firstAncestorOrThisOfType(rimView);
//     if (rimView)
//     {
//         rimView->scheduleCreateDisplayModelAndRedraw();
//     }
// }
