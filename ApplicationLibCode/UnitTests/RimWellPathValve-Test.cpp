/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "RimValveCollection.h"
#include "RimWellPath.h"
#include "RimWellPathValve.h"

//--------------------------------------------------------------------------------------------------
/// A stand-alone valve has no perforation interval ancestor, and must not crash on template update
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathValveTest, TemplateUpdatedWithoutPerforationInterval )
{
    RimWellPath wellPath;

    RimValveCollection* valveCollection = wellPath.valveCollection();
    ASSERT_TRUE( valveCollection != nullptr );

    RimWellPathValve* valve = valveCollection->addIcvValve( 100.0 );
    ASSERT_TRUE( valve != nullptr );

    valve->templateUpdated();
}
