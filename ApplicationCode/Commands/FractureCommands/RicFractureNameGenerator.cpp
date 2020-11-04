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

#include "RicFractureNameGenerator.h"

#include "RimFracture.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelTemplate.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicFractureNameGenerator::nameForNewFracture()
{
    return RicFractureNameGenerator::nameForNewObject<RimFracture>( "Fracture_%1" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicFractureNameGenerator::nameForNewStimPlanModel()
{
    return RicFractureNameGenerator::nameForNewObject<RimStimPlanModel>( "StimPlan Model_%1" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicFractureNameGenerator::nameForNewStimPlanModelTemplate()
{
    return RicFractureNameGenerator::nameForNewObject<RimStimPlanModelTemplate>( "StimPlan Model Template_%1" );
}
