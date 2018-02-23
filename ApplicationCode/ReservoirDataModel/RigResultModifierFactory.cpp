/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigResultModifierFactory.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigResultModifier.h"

#include <math.h>


//--------------------------------------------------------------------------------------------------
/// This function must be harmonized with RigResultAccessorFactory::createNativeResultAccessor()
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultModifier> RigResultModifierFactory::createResultModifier(RigEclipseCaseData* eclipseCase,
                                                                           size_t gridIndex,
                                                                           RiaDefines::PorosityModelType porosityModel,
                                                                           size_t timeStepIndex,
                                                                           QString& uiResultName)
{
    if (!eclipseCase) return nullptr;

    if (!eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return nullptr;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(uiResultName);

    return createResultModifier(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultModifier> RigResultModifierFactory::createResultModifier(RigEclipseCaseData* eclipseCase,
                                                                           size_t gridIndex,
                                                                           RiaDefines::PorosityModelType porosityModel,
                                                                           size_t timeStepIndex, size_t scalarResultIndex)
{
    if ( !eclipseCase ) return nullptr;

    if (!eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return nullptr;
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return nullptr;
    }

    RigGridBase* grid = eclipseCase->grid(gridIndex);
    if (!grid)
    {
        return nullptr;
    }

    std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(scalarResultIndex);

    if (timeStepIndex >= scalarSetResults.size())
    {
        return nullptr;
    }

    std::vector<double>* resultValues = nullptr;
    if (timeStepIndex < scalarSetResults.size())
    {
        resultValues = &(scalarSetResults[timeStepIndex]);
    }

    bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(scalarResultIndex);
    if (useGlobalActiveIndex)
    {
        cvf::ref<RigResultModifier> object = new RigActiveCellsResultModifier(grid, eclipseCase->activeCellInfo(porosityModel), resultValues);
        return object;
    }
    else
    {
        cvf::ref<RigResultModifier> object = new RigAllGridCellsResultModifier(grid, resultValues);
        return object;
    }
}
