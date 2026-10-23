/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimEclipseResultCase.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
/// A main case without any results must not be indexed for its first result.
//--------------------------------------------------------------------------------------------------
TEST( RigCaseCellResultsData, CopyResultsMetaDataFromMainCaseWithoutResults )
{
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    const auto poroModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
    ASSERT_TRUE( eclipseCase->results( poroModel )->existingResults().empty() );

    RigCaseCellResultsData::copyResultsMetaDataFromMainCase( eclipseCase.p(), poroModel, {} );
}
