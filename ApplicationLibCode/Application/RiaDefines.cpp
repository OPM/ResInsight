/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::ResultCatType>::setUp()
{
    addItem( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "DYNAMIC_NATIVE", "Dynamic" );
    addItem( RiaDefines::ResultCatType::STATIC_NATIVE, "STATIC_NATIVE", "Static" );
    addItem( RiaDefines::ResultCatType::SOURSIMRL, "SOURSIMRL", "SourSimRL" );
    addItem( RiaDefines::ResultCatType::GENERATED, "GENERATED", "Generated" );
    addItem( RiaDefines::ResultCatType::INPUT_PROPERTY, "INPUT_PROPERTY", "Input Property" );
    addItem( RiaDefines::ResultCatType::FORMATION_NAMES, "FORMATION_NAMES", "Formation Names" );
    addItem( RiaDefines::ResultCatType::ALLAN_DIAGRAMS, "ALLAN_DIAGRAMS", "Allan Diagrams" );
    addItem( RiaDefines::ResultCatType::FLOW_DIAGNOSTICS, "FLOW_DIAGNOSTICS", "Flow Diagnostics" );
    addItem( RiaDefines::ResultCatType::INJECTION_FLOODING, "INJECTION_FLOODING", "Injection Flooding" );
    setDefault( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
}

template <>
void caf::AppEnum<RiaDefines::DepthUnitType>::setUp()
{
    addItem( RiaDefines::DepthUnitType::UNIT_METER, "UNIT_METER", "Meter" );
    addItem( RiaDefines::DepthUnitType::UNIT_FEET, "UNIT_FEET", "Feet" );
    addItem( RiaDefines::DepthUnitType::UNIT_NONE, "UNIT_NONE", "None" );

    setDefault( RiaDefines::DepthUnitType::UNIT_METER );
}

template <>
void caf::AppEnum<RiaDefines::EclipseUnitSystem>::setUp()
{
    addItem( RiaDefines::EclipseUnitSystem::UNITS_METRIC, "UNITS_METRIC", "Metric" );
    addItem( RiaDefines::EclipseUnitSystem::UNITS_FIELD, "UNITS_FIELD", "Field" );
    addItem( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN, "UNITS_UNKNOWN", "Unknown" );

    setDefault( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
}

template <>
void caf::AppEnum<RiaDefines::DepthTypeEnum>::setUp()
{
    addItem( RiaDefines::DepthTypeEnum::MEASURED_DEPTH, "MEASURED_DEPTH", "Measured Depth" );
    addItem( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, "TRUE_VERTICAL_DEPTH", "True Vertical Depth (MSL)" );
    addItem( RiaDefines::DepthTypeEnum::PSEUDO_LENGTH, "PSEUDO_LENGTH", "Pseudo Length" );
    addItem( RiaDefines::DepthTypeEnum::CONNECTION_NUMBER, "CONNECTION_NUMBER", "Connection Number" );
    addItem( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB, "TRUE_VERTICAL_DEPTH_RKB", "True Vertical Depth (RKB)" );
    setDefault( RiaDefines::DepthTypeEnum::MEASURED_DEPTH );
}

template <>
void caf::AppEnum<RiaDefines::PlotAxis>::setUp()
{
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, "PLOT_AXIS_LEFT", "Left" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT, "PLOT_AXIS_RIGHT", "Right" );

    setDefault( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
}

template <>
void caf::AppEnum<RiaDefines::PhaseType>::setUp()
{
    addItem( RiaDefines::PhaseType::OIL_PHASE, "OIL_PHASE", "Oil" );
    addItem( RiaDefines::PhaseType::GAS_PHASE, "GAS_PHASE", "Gas" );
    addItem( RiaDefines::PhaseType::WATER_PHASE, "WATER_PHASE", "Water" );
    setDefault( RiaDefines::PhaseType::OIL_PHASE );
}

template <>
void caf::AppEnum<RiaDefines::WellPathComponentType>::setUp()
{
    addItem( RiaDefines::WellPathComponentType::WELL_PATH, "WELL_PATH", "Well Path" );
    addItem( RiaDefines::WellPathComponentType::PERFORATION_INTERVAL, "PERFORATION_INTERVAL", "Perforation Interval" );
    addItem( RiaDefines::WellPathComponentType::FISHBONES, "FISHBONES", "Fishbones" );
    addItem( RiaDefines::WellPathComponentType::FRACTURE, "FRACTURE", "Fracture" );
    addItem( RiaDefines::WellPathComponentType::ICD, "ICD", "ICD" );
    addItem( RiaDefines::WellPathComponentType::AICD, "AICD", "AICD" );
    addItem( RiaDefines::WellPathComponentType::ICV, "ICV", "ICV" );
    addItem( RiaDefines::WellPathComponentType::CASING, "CASING", "Casing" );
    addItem( RiaDefines::WellPathComponentType::LINER, "LINER", "Liner" );
    addItem( RiaDefines::WellPathComponentType::PACKER, "PACKER", "Packer" );
    addItem( RiaDefines::WellPathComponentType::UNDEFINED_COMPONENT, "UNDEFINED", "Undefined Component" );
    setDefault( RiaDefines::WellPathComponentType::WELL_PATH );
}

template <>
void caf::AppEnum<RiaDefines::MeshModeType>::setUp()
{
    addItem( RiaDefines::MeshModeType::FULL_MESH, "FULL_MESH", "All" );
    addItem( RiaDefines::MeshModeType::FAULTS_MESH, "FAULTS_MESH", "Faults only" );
    addItem( RiaDefines::MeshModeType::NO_MESH, "NO_MESH", "None" );
    setDefault( RiaDefines::MeshModeType::FULL_MESH );
}

template <>
void caf::AppEnum<RiaDefines::GridCaseAxis>::setUp()
{
    addItem( RiaDefines::GridCaseAxis::UNDEFINED_AXIS, "None", "None" );
    addItem( RiaDefines::GridCaseAxis::AXIS_I, "I", "I" );
    addItem( RiaDefines::GridCaseAxis::AXIS_J, "J", "J" );
    addItem( RiaDefines::GridCaseAxis::AXIS_K, "K", "K" );

    setDefault( RiaDefines::GridCaseAxis::AXIS_K );
}

template <>
void caf::AppEnum<RiaDefines::ThemeEnum>::setUp()
{
    addItem( RiaDefines::ThemeEnum::DEFAULT, "DEFAULT", "Default theme" );
    addItem( RiaDefines::ThemeEnum::DARK, "DARK", "Dark theme" );
    addItem( RiaDefines::ThemeEnum::LIGHT, "LIGHT", "Light theme" );
    setDefault( RiaDefines::ThemeEnum::DEFAULT );
}

template <>
void AppEnum<RiaDefines::RINavigationPolicy>::setUp()
{
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CEETRON, "NAVIGATION_POLICY_CEETRON", "Ceetron" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_CAD, "NAVIGATION_POLICY_CAD", "CAD" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_GEOQUEST, "NAVIGATION_POLICY_GEOQUEST", "GEOQUEST" );
    addItem( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS, "NAVIGATION_POLICY_RMS", "RMS" );
    setDefault( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isNativeCategoryResult( const QString& resultName )
{
    return resultName.endsWith( "NUM" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasic()
{
    return "Result Mock Debug Model Simple";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasicWithResults()
{
    return "Result Mock Debug Model With Results";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelLargeWithResults()
{
    return "Result Mock Debug Model Large With Results";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelCustomized()
{
    return "Result Mock Debug Model Customized";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasicInputCase()
{
    return "Input Mock Debug Model Simple";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RiaDefines::fromEclipseUnit( EclipseUnitSystem eclipseUnit )
{
    switch ( eclipseUnit )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            return DepthUnitType::UNIT_METER;
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            return DepthUnitType::UNIT_FEET;
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_LAB:
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN:
            break;
        default:
            break;
    }

    return DepthUnitType::UNIT_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RiaDefines::fromDepthUnit( DepthUnitType depthUnit )
{
    switch ( depthUnit )
    {
        case RiaDefines::DepthUnitType::UNIT_METER:
            return RiaDefines::EclipseUnitSystem::UNITS_METRIC;
            break;
        case RiaDefines::DepthUnitType::UNIT_FEET:
            return RiaDefines::EclipseUnitSystem::UNITS_FIELD;
            break;
        case RiaDefines::DepthUnitType::UNIT_NONE:
            break;
        default:
            break;
    }

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultValuePlot()
{
    return -10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultLogValuePlot()
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::maximumDefaultValuePlot()
{
    return 100.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ImportFileType RiaDefines::obtainFileTypeFromFileName( const QString& fileName )
{
    if ( fileName.endsWith( "EGRID", Qt::CaseInsensitive ) )
    {
        return ImportFileType::ECLIPSE_EGRID_FILE;
    }
    else if ( fileName.endsWith( "GRID", Qt::CaseInsensitive ) )
    {
        return ImportFileType::ECLIPSE_GRID_FILE;
    }
    else if ( fileName.endsWith( "GRDECL", Qt::CaseInsensitive ) )
    {
        return ImportFileType::ECLIPSE_INPUT_FILE;
    }
    else if ( fileName.endsWith( "SMSPEC", Qt::CaseInsensitive ) )
    {
        return ImportFileType::ECLIPSE_SUMMARY_FILE;
    }
    else if ( fileName.endsWith( "ODB", Qt::CaseInsensitive ) )
    {
        return ImportFileType::GEOMECH_ODB_FILE;
    }
    else if ( fileName.endsWith( ".rsp", Qt::CaseInsensitive ) || fileName.endsWith( ".rip", Qt::CaseInsensitive ) )
    {
        return ImportFileType::RESINSIGHT_PROJECT_FILE;
    }
    return ImportFileType::NOT_A_VALID_IMPORT_FILE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::defaultDirectoryLabel( RiaDefines::ImportFileType fileType )
{
    QString defaultDirLabel;

    int fileTypeAsInt = int( fileType );

    if ( fileType == ImportFileType::ANY_ECLIPSE_FILE )
    {
        defaultDirLabel = "GENERAL_DATA";
    }
    else if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_RESULT_GRID ) )
    {
        defaultDirLabel = "BINARY_GRID";
    }
    else if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_INPUT_FILE ) )
    {
        defaultDirLabel = "INPUT_FILES";
    }
    else if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_SUMMARY_FILE ) )
    {
        // TODO: Summary files used "INPUT_FILES" as last used directory.
        // Check if this is correct.
        defaultDirLabel = "INPUT_FILES";
    }
    else if ( fileTypeAsInt & int( ImportFileType::GEOMECH_ODB_FILE ) )
    {
        defaultDirLabel = "GEOMECH_MODEL";
    }

    return defaultDirLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isInjector( WellProductionType wellProductionType )
{
    if ( wellProductionType == RiaDefines::WellProductionType::GAS_INJECTOR ||
         wellProductionType == RiaDefines::WellProductionType::OIL_INJECTOR ||
         wellProductionType == RiaDefines::WellProductionType::WATER_INJECTOR )
        return true;

    return false;
}
