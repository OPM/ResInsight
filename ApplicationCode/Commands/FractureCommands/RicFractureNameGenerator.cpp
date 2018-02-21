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

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimFracture.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFractureNameGenerator::nameForNewFracture()
{
    std::vector<RimFracture*> oldFractures;
    RiaApplication::instance()->project()->activeOilField()->descendantsIncludingThisOfType(oldFractures);

    size_t fractureNum = oldFractures.size();

    bool found;
    QString name;

    do {
        found = false;
        name = QString("Fracture_%1").arg(fractureNum, 2, 10, QChar('0'));
        for (RimFracture* fracture : oldFractures)
        {
            if (fracture->name() == name)
            {
                found = true;
                break;
            }
        }

        fractureNum++;
    } while (found);

    return name;
}
