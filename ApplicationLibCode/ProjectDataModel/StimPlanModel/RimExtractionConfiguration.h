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

#pragma once

#include "RiaDefines.h"

class RimExtractionConfiguration
{
public:
    enum class EclipseCaseType
    {
        STATIC_CASE,
        DYNAMIC_CASE,
        INITIAL_PRESSURE_CASE
    };

    RimExtractionConfiguration( const QString& resultVar, RiaDefines::ResultCatType resultCat, EclipseCaseType eclipseCase );

    RiaDefines::ResultCatType resultCategory;
    QString                   resultVariable;
    EclipseCaseType           eclipseCaseType;
};
