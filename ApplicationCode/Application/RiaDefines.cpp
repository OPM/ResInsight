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
    addItem( RiaDefines::DYNAMIC_NATIVE, "DYNAMIC_NATIVE", "Dynamic" );
    addItem( RiaDefines::STATIC_NATIVE, "STATIC_NATIVE", "Static" );
    addItem( RiaDefines::SOURSIMRL, "SOURSIMRL", "SourSimRL" );
    addItem( RiaDefines::GENERATED, "GENERATED", "Generated" );
    addItem( RiaDefines::INPUT_PROPERTY, "INPUT_PROPERTY", "Input Property" );
    addItem( RiaDefines::FORMATION_NAMES, "FORMATION_NAMES", "Formation Names" );
    addItem( RiaDefines::FLOW_DIAGNOSTICS, "FLOW_DIAGNOSTICS", "Flow Diagnostics" );
    addItem( RiaDefines::INJECTION_FLOODING, "INJECTION_FLOODING", "Injection Flooding" );
    setDefault( RiaDefines::DYNAMIC_NATIVE );
}

template <>
void caf::AppEnum<RiaDefines::DepthUnitType>::setUp()
{
    addItem( RiaDefines::UNIT_METER, "UNIT_METER", "Meter" );
    addItem( RiaDefines::UNIT_FEET, "UNIT_FEET", "Feet" );
    addItem( RiaDefines::UNIT_NONE, "UNIT_NONE", "None" );

    setDefault( RiaDefines::UNIT_METER );
}

template <>
void caf::AppEnum<RiaDefines::DepthTypeEnum>::setUp()
{
    addItem( RiaDefines::MEASURED_DEPTH, "MEASURED_DEPTH", "Measured Depth" );
    addItem( RiaDefines::TRUE_VERTICAL_DEPTH, "TRUE_VERTICAL_DEPTH", "True Vertical Depth (MSL)" );
    addItem( RiaDefines::PSEUDO_LENGTH, "PSEUDO_LENGTH", "Pseudo Length" );
    addItem( RiaDefines::CONNECTION_NUMBER, "CONNECTION_NUMBER", "Connection Number" );
    setDefault( RiaDefines::MEASURED_DEPTH );
}

template <>
void caf::AppEnum<RiaDefines::PlotAxis>::setUp()
{
    addItem( RiaDefines::PLOT_AXIS_LEFT, "PLOT_AXIS_LEFT", "Left" );
    addItem( RiaDefines::PLOT_AXIS_RIGHT, "PLOT_AXIS_RIGHT", "Right" );

    setDefault( RiaDefines::PLOT_AXIS_LEFT );
}

template <>
void caf::AppEnum<RiaDefines::PhaseType>::setUp()
{
    addItem( RiaDefines::OIL_PHASE,   "OIL_PHASE",   "Oil" );
    addItem( RiaDefines::GAS_PHASE,   "GAS_PHASE",   "Gas" );
    addItem( RiaDefines::WATER_PHASE, "WATER_PHASE", "Water" );
    setDefault( RiaDefines::OIL_PHASE );
}

template <>
void caf::AppEnum<RiaDefines::WellPathComponentType>::setUp()
{
    addItem( RiaDefines::WELL_PATH, "WELL_PATH", "Well Path" );
    addItem( RiaDefines::PERFORATION_INTERVAL, "PERFORATION_INTERVAL", "Perforation Interval" );
    addItem( RiaDefines::FISHBONES, "FISHBONES", "Fishbones" );
    addItem( RiaDefines::FRACTURE, "FRACTURE", "Fracture" );
    addItem( RiaDefines::ICD, "ICD", "ICD" );
    addItem( RiaDefines::AICD, "AICD", "AICD" );
    addItem( RiaDefines::ICV, "ICV", "ICV" );
    addItem( RiaDefines::CASING, "CASING", "Casing" );
    addItem( RiaDefines::LINER, "LINER", "Liner" );
    addItem( RiaDefines::PACKER, "PACKER", "Packer" );
    addItem( RiaDefines::UNDEFINED_COMPONENT, "UNDEFINED", "Undefined Component" );
    setDefault( RiaDefines::WELL_PATH );
}

template <>
void caf::AppEnum<RiaDefines::MeshModeType>::setUp()
{
    addItem( RiaDefines::FULL_MESH, "FULL_MESH", "All" );
    addItem( RiaDefines::FAULTS_MESH, "FAULTS_MESH", "Faults only" );
    addItem( RiaDefines::NO_MESH, "NO_MESH", "None" );
    setDefault( RiaDefines::FULL_MESH );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isPerCellFaceResult( const QString& resultName )
{
    if ( resultName.compare( RiaDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedMultResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::ternarySaturationResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedRiTranResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedRiMultResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedWaterFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedOilFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaDefines::combinedGasFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.endsWith( "IJK" ) )
    {
        return true;
    }

    return false;
}

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
QString RiaDefines::undefinedResultName()
{
    return "None";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::undefinedGridFaultName()
{
    return "Undefined Grid Faults";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::undefinedGridFaultWithInactiveName()
{
    return "Undefined Grid Faults With Inactive";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedTransmissibilityResultName()
{
    return "TRANXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedWaterFluxResultName()
{
    return "FLRWATIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedOilFluxResultName()
{
    return "FLROILIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedGasFluxResultName()
{
    return "FLRGASIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::ternarySaturationResultName()
{
    return "TERNARY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedMultResultName()
{
    return "MULTXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::eqlnumResultName()
{
    return "EQLNUM";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranXResultName()
{
    return "riTRANX";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranYResultName()
{
    return "riTRANY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranZResultName()
{
    return "riTRANZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiTranResultName()
{
    return "riTRANXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultXResultName()
{
    return "riMULTX";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultYResultName()
{
    return "riMULTY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultZResultName()
{
    return "riMULTZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiMultResultName()
{
    return "riMULTXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranXResultName()
{
    return "riTRANXbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranYResultName()
{
    return "riTRANYbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranZResultName()
{
    return "riTRANZbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiAreaNormTranResultName()
{
    return "riTRANXYZbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riCellVolumeResultName()
{
    return "riCELLVOLUME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riOilVolumeResultName()
{
    return "riOILVOLUME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mobilePoreVolumeName()
{
    return "MOBPORV";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::completionTypeResultName()
{
    return "Completion Type";
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
QString RiaDefines::activeFormationNamesResultName()
{
    return "Active Formation Names";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsAzimuthResult()
{
    return "Azimuth";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsInclinationResult()
{
    return "Inclination";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsPPResult()
{
    return "PP";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsSHResult()
{
    return "SH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsSHMkResult()
{
    return "SH_MK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsOBGResult()
{
    return "OBG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsFGResult()
{
    return "FG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::wbsSFGResult()
{
    return "SFG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaDefines::wbsAngleResultNames()
{
    return {wbsAzimuthResult(), wbsInclinationResult()};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaDefines::wbsDerivedResultNames()
{
    return {
        wbsFGResult(),
        wbsOBGResult(),
        wbsPPResult(),
        wbsSFGResult(),
        wbsSHResult(),
        wbsSHMkResult(),
    };
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
        return ECLIPSE_EGRID_FILE;
    }
    else if ( fileName.endsWith( "GRID", Qt::CaseInsensitive ) )
    {
        return ECLIPSE_GRID_FILE;
    }
    else if ( fileName.endsWith( "GRDECL", Qt::CaseInsensitive ) )
    {
        return ECLIPSE_INPUT_FILE;
    }
    else if ( fileName.endsWith( "SMSPEC", Qt::CaseInsensitive ) )
    {
        return ECLIPSE_SUMMARY_FILE;
    }
    else if ( fileName.endsWith( "ODB", Qt::CaseInsensitive ) )
    {
        return GEOMECH_ODB_FILE;
    }
    else if ( fileName.endsWith( ".rsp", Qt::CaseInsensitive ) || fileName.endsWith( ".rip", Qt::CaseInsensitive ) )
    {
        return RESINSIGHT_PROJECT_FILE;
    }
    return NOT_A_VALID_IMPORT_FILE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::defaultDirectoryLabel( RiaDefines::ImportFileType fileType )
{
    QString defaultDirLabel;

    if ( fileType == ANY_ECLIPSE_FILE )
    {
        defaultDirLabel = "GENERAL_DATA";
    }
    else if ( fileType & ECLIPSE_RESULT_GRID )
    {
        defaultDirLabel = "BINARY_GRID";
    }
    else if ( fileType & ECLIPSE_INPUT_FILE )
    {
        defaultDirLabel = "INPUT_FILES";
    }
    else if ( fileType & ECLIPSE_SUMMARY_FILE )
    {
        // TODO: Summary files used "INPUT_FILES" as last used directory.
        // Check if this is correct.
        defaultDirLabel = "INPUT_FILES";
    }
    else if ( fileType & GEOMECH_ODB_FILE )
    {
        defaultDirLabel = "GEOMECH_MODEL";
    }

    return defaultDirLabel;
}
