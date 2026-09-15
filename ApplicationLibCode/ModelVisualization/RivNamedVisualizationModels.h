/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cvfModelBasicList.h"
#include "cvfObject.h"
#include "cvfString.h"

#include <map>
#include <vector>

//==================================================================================================
///
/// Named collection of cvf::ModelBasicList visualization models, created lazily by name.
///
/// The names of the well-known models shared across several 3D view types are centralized here as
/// static accessors, so the string literals identifying a given model are defined in a single place
/// instead of being repeated (and potentially mistyped) at each call site.
///
//==================================================================================================
class RivNamedVisualizationModels
{
public:
    static const char* wellPathPipeModelName() { return "WellPathPipeModel"; }
    static const char* seismicSectionModelName() { return "SeismicSectionModel"; }
    static const char* surfaceModelName() { return "SurfaceModel"; }
    static const char* polygonModelName() { return "PolygonModel"; }
    static const char* crossSectionModelName() { return "CrossSectionModel"; }
    static const char* intersectionModelName() { return "IntersectionModel"; }
    static const char* faultReactivationModelName() { return "FaultReactModel"; }
    static const char* refinementRegionsModelName() { return "RefinementRegionsModel"; }
    static const char* highlightModelName() { return "HighlightModel"; }
    static const char* screenSpaceModelName() { return "ScreenSpaceModel"; }

    cvf::ModelBasicList* findOrCreate( const cvf::String& modelName );
    cvf::ModelBasicList* findOrCreateAndClear( const cvf::String& modelName );
    cvf::ModelBasicList* find( const cvf::String& modelName ) const;

    std::vector<cvf::ModelBasicList*> allModels() const;

    void clear();

private:
    std::map<cvf::String, cvf::ref<cvf::ModelBasicList>> m_models;
};
