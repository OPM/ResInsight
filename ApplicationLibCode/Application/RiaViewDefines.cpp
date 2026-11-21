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

#include "RiaViewDefines.h"

#include "cafAppEnum.h"
#include "cafPdmScriptEnumNameRegistry.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::MeshModeType>::setUp()
{
    addItem( RiaDefines::MeshModeType::FULL_MESH, "FULL_MESH", "All" );
    addItem( RiaDefines::MeshModeType::FAULTS_MESH, "FAULTS_MESH", "Faults only" );
    addItem( RiaDefines::MeshModeType::NO_MESH, "NO_MESH", "None" );
    setDefault( RiaDefines::MeshModeType::FULL_MESH );
}

template <>
void caf::AppEnum<RiaDefines::RINavigationPolicy>::setUp()
{
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CEETRON, "NAVIGATION_POLICY_CEETRON", "Ceetron" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CAD, "NAVIGATION_POLICY_CAD", "CAD" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_GEOQUEST, "NAVIGATION_POLICY_GEOQUEST", "GEOQUEST" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS, "NAVIGATION_POLICY_RMS", "RMS" );
    setDefault( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS );
}

template <>
void caf::AppEnum<RiaDefines::ViewDirection>::setUp()
{
    addItem( RiaDefines::ViewDirection::NORTH, "NORTH", "View From North" );
    addItem( RiaDefines::ViewDirection::SOUTH, "SOUTH", "View From South" );
    addItem( RiaDefines::ViewDirection::EAST, "EAST", "View From East" );
    addItem( RiaDefines::ViewDirection::WEST, "WEST", "View From West" );
    addItem( RiaDefines::ViewDirection::TOP, "TOP", "View From Above" );
    addItem( RiaDefines::ViewDirection::BOTTOM, "BOTTOM", "View From Below" );
    setDefault( RiaDefines::ViewDirection::TOP );
}
} // namespace caf

namespace
{
// Pin the Python StrEnum class name, see RiaDefines.cpp
struct RegisterScriptEnumNames
{
    RegisterScriptEnumNames() { caf::PdmScriptEnumNameRegistry::registerName<RiaDefines::ViewDirection>( "ViewDirection" ); }
};
const RegisterScriptEnumNames s_registerViewScriptEnumNames;
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaDefines::viewScaleOptions()
{
    return { 0.005, 0.01, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 10.0, 15.0, 20.0, 50.0 };
}
