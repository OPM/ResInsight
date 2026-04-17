/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "cvfObject.h"

namespace cvf
{
class ModelBasicList;
class Part;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class RimEclipseCase;
class RimRefinementRegion;
class RimRefinementRegionCollection;

//==================================================================================================
///
/// Builds per-refined-cell wireframe parts for preview of refinement regions in the 3D view.
///
//==================================================================================================
class RivRefinementRegionPartMgr
{
public:
    // Append parts for all active regions in the collection to the given scene model.
    // No-op if the collection, case, or transform is null, or if the collection is disabled.
    static void appendPartsToModel( cvf::ModelBasicList*                 model,
                                    const RimRefinementRegionCollection* collection,
                                    RimEclipseCase*                      eclipseCase,
                                    const caf::DisplayCoordTransform*    coordTransform );

private:
    static cvf::ref<cvf::Part>
        createRegionPart( const RimRefinementRegion* region, RimEclipseCase* eclipseCase, const caf::DisplayCoordTransform* coordTransform );

    static cvf::ref<cvf::Part>
        createOuterBoxPart( const RimRefinementRegion* region, RimEclipseCase* eclipseCase, const caf::DisplayCoordTransform* coordTransform );
};
