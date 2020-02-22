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
class RigEclipseResultAddress;

class RigResultAccessorFactory
{
public:
    static cvf::ref<RigResultAccessor> createFromResultDefinition( const RigEclipseCaseData*         eclipseCase,
                                                                   size_t                            gridIndex,
                                                                   size_t                            timeStepIndex,
                                                                   const RimEclipseResultDefinition* resultDefinition );

    static cvf::ref<RigResultAccessor> createFromResultAddress( const RigEclipseCaseData*      eclipseCase,
                                                                size_t                         gridIndex,
                                                                RiaDefines::PorosityModelType  porosityModel,
                                                                size_t                         timeStepIndex,
                                                                const RigEclipseResultAddress& resVarAddr );

private:
    static cvf::ref<RigResultAccessor> createCombinedResultAccessor( const RigEclipseCaseData*      eclipseCase,
                                                                     size_t                         gridIndex,
                                                                     RiaDefines::PorosityModelType  porosityModel,
                                                                     size_t                         timeStepIndex,
                                                                     const RigEclipseResultAddress& resVarAddr );

    static cvf::ref<RigResultAccessor> createNativeFromResultAddress( const RigEclipseCaseData*      eclipseCase,
                                                                      size_t                         gridIndex,
                                                                      RiaDefines::PorosityModelType  porosityModel,
                                                                      size_t                         timeStepIndex,
                                                                      const RigEclipseResultAddress& resVarAddr );
};
