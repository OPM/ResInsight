/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimExtractionConfiguration.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimExtractionConfiguration::EclipseCaseType>::setUp()
{
    addItem( RimExtractionConfiguration::EclipseCaseType::STATIC_CASE, "STATIC_CASE", "Static Case" );
    addItem( RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE, "DYNAMIC_CASE", "Dynamic Case" );
    addItem( RimExtractionConfiguration::EclipseCaseType::INITIAL_PRESSURE_CASE,
             "INITIAL_PRESSURE_CASE",
             "Initial Pressure Case" );

    setDefault( RimExtractionConfiguration::EclipseCaseType::STATIC_CASE );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtractionConfiguration::RimExtractionConfiguration( const QString&            resultVar,
                                                        RiaDefines::ResultCatType resultCat,
                                                        EclipseCaseType           eclipseCase )
{
    resultVariable  = resultVar;
    resultCategory  = resultCat;
    eclipseCaseType = eclipseCase;
}
