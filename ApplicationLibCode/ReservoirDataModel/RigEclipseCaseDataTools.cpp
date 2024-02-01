/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RigEclipseCaseDataTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigEclipseCaseDataTools::firstProducer( RigEclipseCaseData* eclipseCaseData )
{
    if ( !eclipseCaseData ) return {};

    auto caseCellResultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !caseCellResultsData ) return {};

    auto timeStepCount = caseCellResultsData->timeStepDates().size();
    if ( timeStepCount == 0 ) return {};

    auto simWells = eclipseCaseData->wellResults();
    for ( const auto& well : simWells )
    {
        if ( well->wellProductionType( timeStepCount - 1 ) == RiaDefines::WellProductionType::PRODUCER )
        {
            return well->m_wellName;
        }
    }

    return {};
}
