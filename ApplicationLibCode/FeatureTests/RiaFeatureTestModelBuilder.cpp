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

#include "RiaFeatureTestModelBuilder.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaImportEclipseCaseTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFeatureTestModelBuilder::closeProject()
{
    RiaApplication::instance()->closeProject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FeatureTestModel RiaFeatureTestModelBuilder::eclipseCaseWithResults()
{
    closeProject();

    FeatureTestModel model;

    // Note: openMockModel returns the new case id as a bool, so a successfully created first case
    // (id 0) reports as "false". Do not gate on the return value; query the project for the case.
    RiaImportEclipseCaseTools::openMockModel( RiaDefines::mockModelBasicWithResults() );

    std::vector<RimEclipseCase*> cases = RimProject::current()->eclipseCases();
    if ( !cases.empty() )
    {
        model.eclipseCase = cases.front();

        std::vector<RimEclipseView*> views = cases.front()->reservoirViews();
        if ( !views.empty() )
        {
            model.eclipseView = views.front();
        }
    }

    return model;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FeatureTestModel RiaFeatureTestModelBuilder::wellPath()
{
    closeProject();

    FeatureTestModel model;

    RimOilField* oilField = RimProject::current()->activeOilField();
    if ( oilField && oilField->wellPathCollection() )
    {
        auto* wellPath = new RimWellPath;
        wellPath->setName( "TestWellPath" );
        oilField->wellPathCollection()->addWellPath( wellPath );

        model.wellPath = wellPath;
    }

    return model;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FeatureTestModel RiaFeatureTestModelBuilder::combinedModel()
{
    FeatureTestModel model = eclipseCaseWithResults();

    RimOilField* oilField = RimProject::current()->activeOilField();
    if ( oilField && oilField->wellPathCollection() )
    {
        auto* wellPath = new RimWellPath;
        wellPath->setName( "TestWellPath" );
        oilField->wellPathCollection()->addWellPath( wellPath );

        model.wellPath = wellPath;
    }

    return model;
}
