/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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


#include "RigReservoirGridTools.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimEclipseView.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigReservoirGridTools::gridCount(RimCase* rimCase)
{
    RigMainGrid* eclipseMainGrid = RigReservoirGridTools::eclipseMainGrid(rimCase);
    RigFemPartCollection* geoMechPartCollection = RigReservoirGridTools::geoMechPartCollection(rimCase);

    if (eclipseMainGrid)
    {
        return static_cast<int>(eclipseMainGrid->gridCount());
    }
    else if (geoMechPartCollection)
    {
        return geoMechPartCollection->partCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::StructGridInterface* RigReservoirGridTools::mainGrid(RimCase* rimCase)
{
    return gridByIndex(rimCase, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::StructGridInterface* RigReservoirGridTools::gridByIndex(RimCase* rimCase, int gridIndex)
{
    RigMainGrid* eclipseMainGrid = RigReservoirGridTools::eclipseMainGrid(rimCase);
    RigFemPartCollection* geoMechPartCollection = RigReservoirGridTools::geoMechPartCollection(rimCase);

    if (eclipseMainGrid)
    {
        return eclipseMainGrid->gridByIndex(gridIndex);
    }
    else if (geoMechPartCollection)
    {
        return geoMechPartCollection->part(gridIndex)->getOrCreateStructGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigReservoirGridTools::gridName(RimCase* rimCase, int gridIndex)
{
    RigMainGrid* eclipseMainGrid = RigReservoirGridTools::eclipseMainGrid(rimCase);
    RigFemPartCollection* geoMechPartCollection = RigReservoirGridTools::geoMechPartCollection(rimCase);

    if (eclipseMainGrid)
    {
        return eclipseMainGrid->gridByIndex(gridIndex)->gridName().c_str();
    }
    else if (geoMechPartCollection)
    {
        return QString::number(gridIndex);
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigReservoirGridTools::activeCellInfo(Rim3dView* rimView)
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (eclipseView)
    {
        return eclipseView->currentActiveCellInfo();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RigReservoirGridTools::eclipseMainGrid(RimCase* rimCase)
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase && eclipseCase->eclipseCaseData())
    {
        return eclipseCase->eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RigReservoirGridTools::geoMechPartCollection(RimCase* rimCase)
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(rimCase);
    if (geoMechCase && geoMechCase->geoMechData())
    {
        return geoMechCase->geoMechData()->femParts();
    }

    return nullptr;
}
