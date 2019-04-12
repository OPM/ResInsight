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

#include <QString>
#include <vector>

namespace RiaDefines
{
    enum ResultCatType
    {
        DYNAMIC_NATIVE,
        STATIC_NATIVE,
        SOURSIMRL,
        GENERATED,
        INPUT_PROPERTY,
        FORMATION_NAMES,
        FLOW_DIAGNOSTICS,
        INJECTION_FLOODING,
        REMOVED,

        UNDEFINED = 999
    };

    // WARNING: DO NOT CHANGE THE ORDER WITHOUT KNOWING WHAT YOU ARE DOING!
    //          You may well change the behaviour of property filters.
    enum WellPathComponentType {
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
   
    enum MeshModeType
    {
        FULL_MESH,
        FAULTS_MESH,
        NO_MESH
    };

    bool isPerCellFaceResult(const QString& resultName);
    bool isNativeCategoryResult(const QString& resultName);

    QString undefinedResultName();
    QString undefinedGridFaultName();
    QString undefinedGridFaultWithInactiveName();
    QString combinedTransmissibilityResultName();
    QString combinedWaterFluxResultName();
    QString combinedOilFluxResultName();
    QString combinedGasFluxResultName();

    QString ternarySaturationResultName();
    QString combinedMultResultName();

    QString eqlnumResultName();

    QString riTranXResultName();
    QString riTranYResultName();
    QString riTranZResultName();
    QString combinedRiTranResultName();

    QString riMultXResultName();
    QString riMultYResultName();
    QString riMultZResultName();
    QString combinedRiMultResultName();

    QString riAreaNormTranXResultName();
    QString riAreaNormTranYResultName();
    QString riAreaNormTranZResultName();
    QString combinedRiAreaNormTranResultName();

    QString riCellVolumeResultName();
    QString riOilVolumeResultName();
    QString mobilePoreVolumeName();

    QString completionTypeResultName();

    // Mock model text identifiers
    QString mockModelBasic();
    QString mockModelBasicWithResults();
    QString mockModelLargeWithResults();
    QString mockModelCustomized();
    QString mockModelBasicInputCase();

    QString activeFormationNamesResultName();

    // Well path derived results
    QString wellPathAzimuthResultName();
    QString wellPathInclinationResultName();
    QString wellPathPPResultName();
    QString wellPathSHResultName();
    QString wellPathOBGResultName();
    QString wellPathFGResultName();
    QString wellPathSFGResultName();

    // List of well path derived results
    std::vector<QString> wellPathAngleResultNames();
    std::vector<QString> wellPathStabilityResultNames();

    //Units and conversions
    enum DepthUnitType
    {
        UNIT_METER,
        UNIT_FEET,
        UNIT_NONE
    };



    // Defines relate to plotting
    enum PlotAxis
    {
        PLOT_AXIS_LEFT,
        PLOT_AXIS_RIGHT,
        PLOT_AXIS_BOTTOM
    };

    double minimumDefaultValuePlot();
    double minimumDefaultLogValuePlot();
    double maximumDefaultValuePlot();

    enum PhaseType {
        OIL_PHASE,
        GAS_PHASE,
        WATER_PHASE
    };

    enum ImportFileType
    {
        NOT_A_VALID_IMPORT_FILE = 0x00,
        ECLIPSE_GRID_FILE       = 0x01,
        ECLIPSE_EGRID_FILE      = 0x02,
        ECLIPSE_INPUT_FILE      = 0x04,
        ECLIPSE_SUMMARY_FILE    = 0x08,
        GEOMECH_ODB_FILE        = 0x10,
        RESINSIGHT_PROJECT_FILE = 0x20,
        ECLIPSE_RESULT_GRID     = ECLIPSE_GRID_FILE | ECLIPSE_EGRID_FILE,
        ANY_ECLIPSE_FILE        = ECLIPSE_RESULT_GRID | ECLIPSE_INPUT_FILE | ECLIPSE_SUMMARY_FILE,
        ANY_IMPORT_FILE         = 0xFF
    };

    ImportFileType obtainFileTypeFromFileName(const QString& fileName);
    QString        defaultDirectoryLabel(ImportFileType fileTypes);

    enum FontSettingType
    {
        SCENE_FONT,
        ANNOTATION_FONT,
        WELL_LABEL_FONT,
        PLOT_FONT
    };

};

