/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechView.h"
#include "RimReservoirView.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimResultSlot.h"



namespace caf {

template<>
void caf::AppEnum< RimGeoMechView::MeshModeType >::setUp()
{
    addItem(RimGeoMechView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(RimGeoMechView::FAULTS_MESH,    "FAULTS_MESH",      "Faults only");
    addItem(RimGeoMechView::NO_MESH,        "NO_MESH",        "None");
    setDefault(RimGeoMechView::FULL_MESH);
}

template<>
void caf::AppEnum< RimGeoMechView::SurfaceModeType >::setUp()
{
    addItem(RimGeoMechView::SURFACE,              "SURFACE",             "All");
    addItem(RimGeoMechView::FAULTS,               "FAULTS",              "Faults only");
    addItem(RimGeoMechView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(RimGeoMechView::SURFACE);
}

} // End namespace caf





CAF_PDM_SOURCE_INIT(RimGeoMechView, "GeoMechView");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::RimGeoMechView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);

    CAF_PDM_InitObject("Geomechanical View", ":/ReservoirView.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cellResult, "GridCellResult", "Cell Result", ":/CellResult.png", "", "");
    cellResult = new RimResultSlot();

    CAF_PDM_InitFieldNoDefault(&overlayInfoConfig,  "OverlayInfoConfig", "Info Box", "", "", "");
    overlayInfoConfig = new Rim3dOverlayInfoConfig();

    CAF_PDM_InitField(&name, "UserDescription", QString("View"), "Name", "", "", "");
    
    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ,          "GridZScale", defaultScaleFactor,         "Z Scale",          "", "Scales the scene in the Z direction", "");

    caf::AppEnum<RimGeoMechView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor,     "ViewBackgroundColor",  defBackgColor, "Background", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGeoMechView::userDescriptionField()
{
    return &name;
}

