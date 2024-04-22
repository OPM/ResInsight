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

#pragma once

#include "enum_bitmask.hpp"

#include <QString>
#include <set>
#include <vector>

namespace RiaDefines
{
enum class EclipseUnitSystem
{
    UNITS_METRIC,
    UNITS_FIELD,
    UNITS_LAB,
    UNITS_UNKNOWN,
};

enum class ResultCatType
{
    DYNAMIC_NATIVE,
    STATIC_NATIVE,
    SOURSIMRL,
    GENERATED,
    INPUT_PROPERTY,
    FORMATION_NAMES,
    ALLAN_DIAGRAMS,
    FLOW_DIAGNOSTICS,
    INJECTION_FLOODING,
    REMOVED,

    UNDEFINED = 999
};

enum class ResultDataType
{
    UNKNOWN,
    FLOAT,
    DOUBLE,
    INTEGER
};

// WARNING: DO NOT CHANGE THE ORDER WITHOUT KNOWING WHAT YOU ARE DOING!
//          You may well change the behaviour of property filters.
enum class WellPathComponentType
{
    // Production Tube
    WELL_PATH,
    // Well path flow completions
    PERFORATION_INTERVAL,
    FISHBONES,
    FRACTURE,
    ICD,
    AICD,
    ICV,
    // Well path construction features
    CASING,
    LINER,
    PACKER,
    UNDEFINED_COMPONENT
};

enum class MeshModeType
{
    FULL_MESH,
    FAULTS_MESH,
    NO_MESH
};

// Mock model text identifiers
QString mockModelBasic();
QString mockModelBasicWithResults();
QString mockModelLargeWithResults();
QString mockModelCustomized();
QString mockModelBasicInputCase();

// Units and conversions
enum class DepthUnitType
{
    UNIT_METER,
    UNIT_FEET,
    UNIT_NONE
};

DepthUnitType     fromEclipseUnit( EclipseUnitSystem eclipseUnit );
EclipseUnitSystem fromDepthUnit( DepthUnitType depthUnit );

// Depth types used for well log plots
enum class DepthTypeEnum
{
    MEASURED_DEPTH,
    TRUE_VERTICAL_DEPTH,
    PSEUDO_LENGTH,
    CONNECTION_NUMBER,
    TRUE_VERTICAL_DEPTH_RKB
};

enum class PhaseType
{
    OIL_PHASE,
    GAS_PHASE,
    WATER_PHASE,
    PHASE_NOT_APPLICABLE
};

enum class ImportFileType
{
    NOT_A_VALID_IMPORT_FILE = 0x00,
    ECLIPSE_GRID_FILE       = 0x01,
    ECLIPSE_EGRID_FILE      = 0x02,
    ECLIPSE_INPUT_FILE      = 0x04,
    ECLIPSE_SUMMARY_FILE    = 0x08,
    GEOMECH_ODB_FILE        = 0x10,
    RESINSIGHT_PROJECT_FILE = 0x20,
    GEOMECH_INP_FILE        = 0x40,
    EM_H5GRID               = 0x80,
    ROFF_FILE               = 0x100,
    ECLIPSE_RESULT_GRID     = ECLIPSE_GRID_FILE | ECLIPSE_EGRID_FILE,
    ANY_ECLIPSE_FILE        = ECLIPSE_RESULT_GRID | ECLIPSE_INPUT_FILE | ECLIPSE_SUMMARY_FILE | ROFF_FILE | EM_H5GRID,
    ANY_GEOMECH_FILE        = GEOMECH_ODB_FILE | GEOMECH_INP_FILE,
    ANY_IMPORT_FILE         = 0xFF
};

ImportFileType obtainFileTypeFromFileName( const QString& fileName );
QString        defaultDirectoryLabel( ImportFileType fileTypes );

enum class CaseType
{
    UNDEFINED_CASE      = -1,
    ECLIPSE_RESULT_CASE = 1,
    ECLIPSE_INPUT_CASE  = 2,
    ECLIPSE_STAT_CASE   = 3,
    ECLIPSE_SOURCE_CASE = 4,
    GEOMECH_ODB_CASE    = 5
};

enum class FontSettingType
{
    SCENE_FONT,
    ANNOTATION_FONT,
    WELL_LABEL_FONT,
    PLOT_FONT
};

enum class GridCaseAxis
{
    AXIS_I,
    AXIS_J,
    AXIS_K,
    UNDEFINED_AXIS
};

enum class GridModelReader
{
    RESDATA,
    OPM_COMMON
};

enum class ThemeEnum
{
    DEFAULT,
    DARK,
    UNDEFINED
};

enum class RINavigationPolicy : short
{
    NAVIGATION_POLICY_CEETRON,
    NAVIGATION_POLICY_CAD,
    NAVIGATION_POLICY_GEOQUEST,
    NAVIGATION_POLICY_RMS
};

enum class WellProductionType : short
{
    PRODUCER,
    OIL_INJECTOR,
    GAS_INJECTOR,
    WATER_INJECTOR,
    UNDEFINED_PRODUCTION_TYPE
};

bool isInjector( WellProductionType wellProductionType );

QString stringListSeparator();

enum class ColumnCount
{
    COLUMNS_1         = 1,
    COLUMNS_2         = 2,
    COLUMNS_3         = 3,
    COLUMNS_4         = 4,
    COLUMNS_UNLIMITED = 1000,
};

enum class RowCount
{
    ROWS_1 = 1,
    ROWS_2 = 2,
    ROWS_3 = 3,
    ROWS_4 = 4,
};

enum class MultiPlotPageUpdateType : uint32_t
{
    NONE   = 0b00000000,
    LEGEND = 0b00000001,
    PLOT   = 0b00000010,
    TITLE  = 0b00000100,
    ALL    = 0b00000111
};

std::vector<double> viewScaleOptions();

enum class View3dContent
{
    NONE              = 0b00000000,
    ECLIPSE_DATA      = 0b00000001,
    GEOMECH_DATA      = 0b00000010,
    FLAT_INTERSECTION = 0b00000100,
    CONTOUR           = 0b00001000,
    SEISMIC           = 0b00010000,
    ALL               = 0b00011111
};

enum class ItemIn3dView
{
    NONE    = 0b00000000,
    SURFACE = 0b00000001,
    POLYGON = 0b00000010,
    ALL     = 0b00000011
};

}; // namespace RiaDefines

// Activate bit mask operators at global scope
ENABLE_BITMASK_OPERATORS( RiaDefines::MultiPlotPageUpdateType )
ENABLE_BITMASK_OPERATORS( RiaDefines::View3dContent )
ENABLE_BITMASK_OPERATORS( RiaDefines::ItemIn3dView )
