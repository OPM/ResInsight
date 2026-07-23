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

#pragma once

class RimEclipseCase;
class RimGridView;
class RimWellPath;

//==================================================================================================
/// Builds small in-memory projects used by the command-feature tests.
///
/// The builders start from a clean project (RiaApplication::closeProject) and populate it with the
/// objects a feature scenario needs, returning pointers into the live project so the caller can set
/// the current caf::SelectionManager selection.
//==================================================================================================
struct FeatureTestModel
{
    RimEclipseCase* eclipseCase = nullptr;
    RimGridView*    eclipseView = nullptr;
    RimWellPath*    wellPath    = nullptr;
};

class RiaFeatureTestModelBuilder
{
public:
    // Close the current project, leaving an empty project in place.
    static void closeProject();

    // A mock Eclipse case with results and a 3D view (via RiaImportEclipseCaseTools::openMockModel).
    static FeatureTestModel eclipseCaseWithResults();

    // A single well path added to the active oil field's well path collection.
    static FeatureTestModel wellPath();

    // A single project containing both a mock Eclipse case (with a 3D view) and a well path, so the
    // sweep can select different object types without rebuilding the (expensive) Eclipse model.
    static FeatureTestModel combinedModel();
};
