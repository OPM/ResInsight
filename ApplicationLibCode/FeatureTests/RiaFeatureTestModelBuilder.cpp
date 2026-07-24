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
#include "RimMockSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RifEclipseSummaryAddress.h"

#include "Well/RigWellPath.h"

#include "cvfVector3.h"

#include <QString>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Create a named well path with a simple vertical geometry and add it to the active oil field's
/// well path collection. The geometry has more than two measured depths so features that require a
/// real trajectory (laterals, targets, completions) have something to work with.
//--------------------------------------------------------------------------------------------------
RimWellPath* addWellPathWithGeometry( const QString& name )
{
    RimOilField* oilField = RimProject::current()->activeOilField();
    if ( !oilField || !oilField->wellPathCollection() ) return nullptr;

    auto* wellPath = new RimWellPath;
    wellPath->setName( name );

    // Set a unit system so features that would otherwise prompt for one (e.g. completion features via
    // RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem) run without a modal dialog.
    wellPath->setUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    cvf::ref<RigWellPath> geometry =
        new RigWellPath( { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 0, 0, -500 ), cvf::Vec3d( 0, 0, -1000 ) }, { 0.0, 500.0, 1000.0 } );
    wellPath->setWellPathGeometry( geometry.p() );

    oilField->wellPathCollection()->addWellPath( wellPath );

    return wellPath;
}
} // namespace

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

            // Many features do not read the selection but the active view (activeReservoirView() /
            // activeGridView()). Set it here so those features have a valid context. The active-view
            // pointer is a dangling-safe caf::PdmPointer, so it is cleared automatically when the
            // project (and thus the view) is closed in closeProject().
            RiaApplication::instance()->setActiveReservoirView( model.eclipseView );
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

    model.wellPath = addWellPathWithGeometry( "TestWellPath" );

    return model;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FeatureTestModel RiaFeatureTestModelBuilder::combinedModel()
{
    FeatureTestModel model = eclipseCaseWithResults();

    model.wellPath = addWellPathWithGeometry( "TestWellPath" );

    return model;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FeatureTestModel RiaFeatureTestModelBuilder::summaryCase()
{
    closeProject();

    FeatureTestModel model;

    RimOilField* oilField = RimProject::current()->activeOilField();
    if ( oilField && oilField->summaryCaseMainCollection() )
    {
        auto* mockCase = new RimMockSummaryCase;
        mockCase->setName( "TestSummaryCase" );

        // A single field vector with a few time steps, enough for plot/curve features to have data.
        const std::vector<time_t> timeSteps = { 0, 86400, 172800 };
        const std::vector<double> values    = { 1.0, 2.0, 3.0 };
        mockCase->addVector( RifEclipseSummaryAddress::fieldAddress( "FOPT" ), timeSteps, values );

        oilField->summaryCaseMainCollection()->addCase( mockCase );

        model.summaryCase = mockCase;
    }

    return model;
}
