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

#pragma once

#include "RifReaderInterface.h"
#include "RigResultAccessor.h"

#include "RiaDefines.h"

class RigActiveCellInfo;
class RigGridBase;

class RimEclipseResultDefinition;

class RigResultAccessorFactory
{
public:
    static cvf::ref<RigResultAccessor>
        createFromResultDefinition(const RigEclipseCaseData* eclipseCase,
                                   size_t gridIndex,
                                   size_t timeStepIndex,
                                   RimEclipseResultDefinition* resultDefinition);

    static cvf::ref<RigResultAccessor>
        createFromUiResultName(const RigEclipseCaseData* eclipseCase,
                               size_t gridIndex,
                               RifReaderInterface::PorosityModelResultType porosityModel,
                               size_t timeStepIndex,
                               const QString& uiResultName);

    static cvf::ref<RigResultAccessor>
        createFromNameAndType(const RigEclipseCaseData* eclipseCase,
                              size_t gridIndex,
                              RifReaderInterface::PorosityModelResultType porosityModel,
                              size_t timeStepIndex,
                              const QString& uiResultName,
                              RiaDefines::ResultCatType resultType);

    static cvf::ref<RigResultAccessor>
        createFromResultIdx(const RigEclipseCaseData* eclipseCase,
                            size_t gridIndex,
                            RifReaderInterface::PorosityModelResultType porosityModel,
                            size_t timeStepIndex,
                            size_t resultIndex);

private:
    static cvf::ref<RigResultAccessor>
        createNativeFromUiResultName(const RigEclipseCaseData* eclipseCase,
                                     size_t gridIndex,
                                     RifReaderInterface::PorosityModelResultType porosityModel,
                                     size_t timeStepIndex,
                                     const QString& resultName);

};


