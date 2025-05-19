/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RifFaultReactivationModelExporter.h"

#include "RigFaultReactivationModel.h"
#include "RigGriddedPart3d.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFilePathTools.h"
#include "RiaVersionInfo.h"
#include "RiaWellLogUnitTools.h"

#include "RifInpExportTools.h"
#include "RifJsonEncodeDecode.h"

#include "RimFaultReactivationDataAccess.h"
#include "RimFaultReactivationEnums.h"
#include "RimFaultReactivationModel.h"
#include "RimFaultReactivationTools.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <numbers>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::exportToStream( std::ostream& stream, const RimFaultReactivationModel& rimModel )
{
    auto [modelOk, errorMsg] = rimModel.validateModel();
    if ( !modelOk ) return { false, errorMsg };

    auto dataAccess = extractAndExportModelData( rimModel );
    if ( !dataAccess ) return { false, "Unable to get necessary data from the input case." };

    std::string applicationNameAndVersion = std::string( RI_APPLICATION_NAME ) + " " + std::string( STRPRODUCTVER );

    using PartBorderSurface                                        = RimFaultReactivation::BorderSurface;
    std::vector<std::pair<PartBorderSurface, std::string>> borders = { { PartBorderSurface::UpperSurface, "TOP" },
                                                                       { PartBorderSurface::FaultSurface, "FAULT" },
                                                                       { PartBorderSurface::LowerSurface, "BASE" },
                                                                       { PartBorderSurface::Seabed, "SEABED" } };

    // The two parts are "mirrored", so face number 4 of the two parts should face eachother.
    using FaultGridPart                                              = RimFaultReactivation::GridPart;
    std::map<std::pair<FaultGridPart, PartBorderSurface>, int> faces = { { { FaultGridPart::FW, PartBorderSurface::FaultSurface }, 5 },
                                                                         { { FaultGridPart::FW, PartBorderSurface::UpperSurface }, 5 },
                                                                         { { FaultGridPart::FW, PartBorderSurface::LowerSurface }, 5 },
                                                                         { { FaultGridPart::FW, PartBorderSurface::Seabed }, 2 },
                                                                         { { FaultGridPart::HW, PartBorderSurface::FaultSurface }, 5 },
                                                                         { { FaultGridPart::HW, PartBorderSurface::UpperSurface }, 5 },
                                                                         { { FaultGridPart::HW, PartBorderSurface::LowerSurface }, 5 },
                                                                         { { FaultGridPart::HW, PartBorderSurface::Seabed }, 2 } };

    std::map<FaultGridPart, std::string> partNames = {
        { FaultGridPart::FW, "FW" },
        { FaultGridPart::HW, "HW" },
    };

    std::map<RimFaultReactivation::Boundary, std::string> boundaries = {
        { RimFaultReactivation::Boundary::Bottom, "BOTTOM" },
        { RimFaultReactivation::Boundary::FarSide, "FARSIDE" },
    };

    std::map<RimFaultReactivation::ElementSets, std::string> materialNames = {
        { RimFaultReactivation::ElementSets::OverBurden, "OVERBURDEN" },
        { RimFaultReactivation::ElementSets::IntraReservoir, "INTRA_RESERVOIR" },
        { RimFaultReactivation::ElementSets::Reservoir, "RESERVOIR" },
        { RimFaultReactivation::ElementSets::UnderBurden, "UNDERBURDEN" },
        { RimFaultReactivation::ElementSets::FaultZone, "FAULT_ZONE" },
    };

    bool useGridVoidRatio         = rimModel.useGridVoidRatio();
    bool useGridPorePressure      = rimModel.useGridPorePressure();
    bool useGridTemperature       = rimModel.useGridTemperature();
    bool useGridDensity           = rimModel.useGridDensity();
    bool useGridElasticProperties = rimModel.useGridElasticProperties();

    double seaBedDepth   = rimModel.seaBedDepth();
    double waterDensity  = rimModel.waterDensity();
    double seaWaterLoad  = RiaWellLogUnitTools<double>::gravityAcceleration() * seaBedDepth * waterDensity;
    double frictionValue = std::tan( ( rimModel.frictionAngleDeg() / 180.0 ) * std::numbers::pi );

    auto model = rimModel.model();
    CAF_ASSERT( !model.isNull() );

    const std::string basePath = rimModel.baseFilePath();

    std::vector<std::function<std::pair<bool, std::string>()>> methods = {
        [&]() { return printHeading( stream, applicationNameAndVersion ); },
        [&]() { return printParts( stream, *model, partNames, borders, faces, boundaries, materialNames ); },
        [&]() { return printAssembly( stream, *model, partNames, !rimModel.useLocalCoordinates(), model->modelLocalNormalsXY() ); },
        [&]()
        {
            return printMaterials( stream, rimModel, materialNames, *dataAccess, basePath, partNames, useGridDensity, useGridElasticProperties );
        },
        [&]() { return printInteractionProperties( stream, frictionValue ); },
        [&]() { return printBoundaryConditions( stream, *model, partNames, boundaries ); },
        [&]() { return printPredefinedFields( stream, *model, *dataAccess, basePath, partNames, useGridVoidRatio, useGridTemperature ); },
        [&]() { return printInteractions( stream, partNames, borders ); },
        [&]()
        {
            return printSteps( stream,
                               *model,
                               *dataAccess,
                               partNames,
                               rimModel.selectedTimeSteps(),
                               basePath,
                               useGridPorePressure,
                               useGridTemperature,
                               seaWaterLoad );
        },
    };

    for ( auto method : methods )
    {
        auto [isOk, errorMessage] = method();
        if ( !isOk ) return { false, errorMessage };
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::exportToFile( const RimFaultReactivationModel& model )
{
    std::ofstream stream( model.inputFilename() );
    return exportToStream( stream, model );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::printHeading( std::ostream& stream, const std::string& applicationNameAndVersion )
{
    if ( RifInpExportTools::printHeading( stream, "Heading" ) &&
         RifInpExportTools::printComment( stream, std::string( "Generated by: " ).append( applicationNameAndVersion ) ) &&
         RifInpExportTools::printHeading( stream, "Preprint, echo=NO, model=NO, history=NO, contact=NO" ) )
    {
        return { true, "" };
    }

    return { false, "Failed to write header to fault reactivation INP." };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::printParts(
    std::ostream&                                                                                        stream,
    const RigFaultReactivationModel&                                                                     model,
    const std::map<RimFaultReactivation::GridPart, std::string>&                                         partNames,
    const std::vector<std::pair<RimFaultReactivation::BorderSurface, std::string>>&                      borders,
    const std::map<std::pair<RimFaultReactivation::GridPart, RimFaultReactivation::BorderSurface>, int>& faces,
    const std::map<RimFaultReactivation::Boundary, std::string>&                                         boundaries,
    const std::map<RimFaultReactivation::ElementSets, std::string>&                                      materialNames )
{
    RifInpExportTools::printSectionComment( stream, "PARTS" );

    auto parts = model.allGridParts();
    for ( const auto& part : parts )
    {
        auto partNameIt = partNames.find( part );
        CAF_ASSERT( partNameIt != partNames.end() );
        std::string partName = partNameIt->second;

        RifInpExportTools::printHeading( stream, "Part, name=" + partName );

        auto grid = model.grid( part );

        const std::vector<cvf::Vec3d>& nodes = grid->nodes();
        RifInpExportTools::printNodes( stream, nodes );

        const std::vector<std::vector<unsigned int>>& elements = grid->elementIndices();
        RifInpExportTools::printElements( stream, elements );

        RifInpExportTools::printNodeSet( stream, "ALL", 1, nodes.size(), false );
        RifInpExportTools::printElementSet( stream, "ALL", 1, elements.size() );

        RifInpExportTools::printNodeSet( stream, "PORE_PRESSURE", 1, nodes.size(), true );

        const std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>>& borderSurfaceElements = grid->borderSurfaceElements();

        for ( auto [boundary, boundaryName] : boundaries )
        {
            // Create boundary condition sets for each side of the parts (except top).
            const auto& boundaryNodes    = grid->boundaryNodes();
            const auto& boundaryElements = grid->boundaryElements();

            const std::vector<unsigned int>& nodes = boundaryNodes.at( boundary );
            RifInpExportTools::printNodeSet( stream, boundaryName, false, nodes );

            const std::vector<unsigned int>& elements = boundaryElements.at( boundary );
            RifInpExportTools::printElementSet( stream, boundaryName, false, elements );
        }

        for ( auto [border, borderName] : borders )
        {
            auto elementIt = faces.find( { part, border } );
            CAF_ASSERT( elementIt != faces.end() );
            int elementSide = elementIt->second;

            std::string sideName        = "S" + std::to_string( elementSide );
            auto        surfaceElements = borderSurfaceElements.find( border );
            if ( surfaceElements != borderSurfaceElements.end() )
            {
                std::string borderElementName = "_" + borderName + "_" + sideName;
                RifInpExportTools::printElementSet( stream, borderElementName, true, surfaceElements->second );
                RifInpExportTools::printSurface( stream, borderName, borderElementName, sideName );
            }
        }

        std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>> elementSets = grid->elementSets();

        for ( auto [setType, elements] : elementSets )
        {
            auto materialNameIt = materialNames.find( setType );
            CAF_ASSERT( materialNameIt != materialNames.end() );
            std::string materialName    = materialNameIt->second;
            std::string materialSetName = materialName;
            if ( elements.empty() )
            {
                RifInpExportTools::printComment( stream, "Section: " + materialName + " (skipped: no elements)" );
            }
            else
            {
                RifInpExportTools::printComment( stream, "Section: " + materialName );
                RifInpExportTools::printElementSet( stream, materialSetName, true, elements );
                RifInpExportTools::printHeading( stream, "Solid Section, elset=" + materialSetName + ", material=" + materialName );
            }
        }

        RifInpExportTools::printLine( stream, "," );
        RifInpExportTools::printHeading( stream, "End Part" );

        if ( !stream.good() ) return { false, "Failed to write part " + partName + " to fault reactivation INP." };
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printAssembly( std::ostream&                                                stream,
                                                      const RigFaultReactivationModel&                             model,
                                                      const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                      bool                                                         includeTransform,
                                                      const std::pair<cvf::Vec3d, cvf::Vec3d>&                     transform )
{
    // ASSEMBLY part
    RifInpExportTools::printSectionComment( stream, "ASSEMBLY" );
    RifInpExportTools::printHeading( stream, "Assembly, name=Assembly" );

    auto parts = model.allGridParts();

    for ( const auto& part : parts )
    {
        auto partNameIt = partNames.find( part );
        CAF_ASSERT( partNameIt != partNames.end() );
        std::string partName = partNameIt->second;

        std::string instanceName = partName;
        RifInpExportTools::printHeading( stream, "Instance, name=" + instanceName + ", part=" + partName );
        RifInpExportTools::printHeading( stream, "End Instance" );
    }

    if ( includeTransform )
    {
        for ( const auto& part : parts )
        {
            auto partNameIt = partNames.find( part );
            CAF_ASSERT( partNameIt != partNames.end() );
            std::string partName = partNameIt->second;

            RifInpExportTools::printHeading( stream, "Transform, nset=" + partName + ".ALL" );
            auto [dir1, dir2] = transform;
            RifInpExportTools::printNumbers( stream, { dir1.x(), dir1.y(), dir1.z(), dir2.x(), dir2.y(), dir2.z() } );
        }
    }

    RifInpExportTools::printHeading( stream, "End Assembly" );

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printMaterials( std::ostream&                                                   stream,
                                                       const RimFaultReactivationModel&                                rimModel,
                                                       const std::map<RimFaultReactivation::ElementSets, std::string>& materialNames,
                                                       const RimFaultReactivationDataAccess&                           dataAccess,
                                                       const std::string&                                              exportBasePath,
                                                       const std::map<RimFaultReactivation::GridPart, std::string>&    partNames,
                                                       bool                                                            densityFromGrid,
                                                       bool elasticPropertiesFromGrid )
{
    // MATERIALS PART
    struct Material
    {
        std::string name;
        double      density;
        double      youngsModulus;
        double      poissonNumber;
        double      permeability1;
        double      permeability2;
        double      expansion;
    };

    RifInpExportTools::printSectionComment( stream, "MATERIALS" );
    std::vector<Material> materials;

    for ( auto [element, materialName] : materialNames )
    {
        std::array<double, 4> parameters = rimModel.materialParameters( element );

        // Incoming unit for Young's Modulus is GPa: convert to Pa.
        double youngsModulus = RiaEclipseUnitTools::gigaPascalToPascal( parameters[0] );

        // Poisson Number has no unit.
        double poissonNumber = parameters[1];

        // Unit is already kg/m^3
        double density = parameters[2];

        double expansion = parameters[3];

        materials.push_back( Material{ .name          = materialName,
                                       .density       = density,
                                       .youngsModulus = youngsModulus,
                                       .poissonNumber = poissonNumber,
                                       .permeability1 = 1e-09,
                                       .permeability2 = 0.3,
                                       .expansion     = expansion } );
    }

    for ( Material mat : materials )
    {
        RifInpExportTools::printHeading( stream, "Material, name=" + mat.name );
        RifInpExportTools::printHeading( stream, "Density" );
        if ( densityFromGrid )
        {
            RifInpExportTools::printLine( stream, "DENSITY" );
        }
        else
        {
            RifInpExportTools::printNumber( stream, mat.density );
        }

        RifInpExportTools::printHeading( stream, "Elastic" );
        if ( elasticPropertiesFromGrid )
        {
            RifInpExportTools::printLine( stream, "ELASTICS" );
        }
        else
        {
            RifInpExportTools::printNumbers( stream, { mat.youngsModulus, mat.poissonNumber } );
        }

        RifInpExportTools::printHeading( stream, "Permeability, specific=1." );
        RifInpExportTools::printNumbers( stream, { mat.permeability1, mat.permeability2 } );
        RifInpExportTools::printHeading( stream, "Expansion" );
        RifInpExportTools::printNumbers( stream, { mat.expansion } );
    }

    if ( densityFromGrid )
    {
        // Export the density to a separate inp file
        std::string tableName     = "DENSITY";
        std::string fullPath      = exportBasePath + "_" + tableName + ".inp";
        auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

        auto model = rimModel.model();
        bool isOk  = writePropertiesToFile( *model,
                                           dataAccess,
                                            { RimFaultReactivation::Property::Density },
                                            { "DENSITY" },
                                           0,
                                           fullPath,
                                           partNames,
                                           tableName,
                                           ", 1." );
        if ( !isOk ) return { false, "Failed to create density file." };

        RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
    }

    if ( elasticPropertiesFromGrid )
    {
        // Export the elastic properties to a separate inp file
        std::string tableName     = "ELASTICS";
        std::string fullPath      = exportBasePath + "_" + tableName + ".inp";
        auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

        auto model = rimModel.model();
        bool isOk  = writePropertiesToFile( *model,
                                           dataAccess,
                                            { RimFaultReactivation::Property::YoungsModulus, RimFaultReactivation::Property::PoissonsRatio },
                                            { "MODULUS", "RATIO" },
                                           0,
                                           fullPath,
                                           partNames,
                                           tableName,
                                           ", 2." );
        if ( !isOk ) return { false, "Failed to create elastic properties file." };

        RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::printInteractionProperties( std::ostream& stream, double faultFriction )
{
    RifInpExportTools::printSectionComment( stream, "INTERACTION PROPERTIES" );

    // Fault interaction
    RifInpExportTools::printHeading( stream, "Surface Interaction, name=FAULT" );
    RifInpExportTools::printNumber( stream, 1.0 );
    RifInpExportTools::printHeading( stream, "Friction, slip tolerance=0.005" );
    RifInpExportTools::printNumber( stream, faultFriction );
    RifInpExportTools::printHeading( stream, "Surface Behavior, no separation, pressure-overclosure=HARD" );
    // Non-fault interaction
    RifInpExportTools::printHeading( stream, "Surface Interaction, name=NON-FAULT" );
    RifInpExportTools::printNumber( stream, 1.0 );
    RifInpExportTools::printHeading( stream, "Cohesive Behavior" );

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printBoundaryConditions( std::ostream&                                                stream,
                                                                const RigFaultReactivationModel&                             model,
                                                                const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                                const std::map<RimFaultReactivation::Boundary, std::string>& boundaries )
{
    auto printBoundaryCondition = []( std::ostream& stream, const std::string& boundarySetName, const std::string& symmetryType )
    {
        RifInpExportTools::printHeading( stream, "Boundary" );
        RifInpExportTools::printLine( stream, boundarySetName + ", " + symmetryType );
    };

    std::map<RimFaultReactivation::Boundary, std::string> symmetryTypes = {
        { RimFaultReactivation::Boundary::Bottom, "ZSYMM" },
        { RimFaultReactivation::Boundary::FarSide, "XSYMM" },

    };

    RifInpExportTools::printSectionComment( stream, "BOUNDARY CONDITIONS" );

    auto parts = model.allGridParts();

    for ( const auto& part : parts )
    {
        auto partNameIt = partNames.find( part );
        CAF_ASSERT( partNameIt != partNames.end() );
        std::string partName = partNameIt->second;

        for ( auto [boundary, boundaryName] : boundaries )
        {
            std::string boundarySetName = partName + "." + boundaryName;
            std::string symmetryType    = symmetryTypes[boundary];
            printBoundaryCondition( stream, boundarySetName, symmetryType );
        }
    }

    std::string partSymmetry = "YSYMM";
    for ( const auto& part : parts )
    {
        auto partNameIt = partNames.find( part );
        CAF_ASSERT( partNameIt != partNames.end() );
        std::string partName = partNameIt->second;
        printBoundaryCondition( stream, partName + ".ALL", partSymmetry );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printPredefinedFields( std::ostream&                                                stream,
                                                              const RigFaultReactivationModel&                             model,
                                                              const RimFaultReactivationDataAccess&                        dataAccess,
                                                              const std::string&                                           exportBasePath,
                                                              const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                              bool voidRatioFromEclipse,
                                                              bool useGridTemperature )
{
    // PREDEFINED FIELDS
    struct PredefinedField
    {
        std::string initialConditionType;
        std::string partName;
        double      value;
    };

    std::vector<PredefinedField> fields;
    for ( auto [part, partName] : partNames )
    {
        std::string name = partName + ".ALL";
        if ( !voidRatioFromEclipse )
        {
            fields.push_back( PredefinedField{ .initialConditionType = "RATIO", .partName = name, .value = 0.3 } );
        }
    }

    RifInpExportTools::printSectionComment( stream, "INITIAL CONDITIONS" );

    for ( auto field : fields )
    {
        RifInpExportTools::printHeading( stream, "Initial Conditions, TYPE=" + field.initialConditionType );
        RifInpExportTools::printLine( stream, field.partName + ", " + std::to_string( field.value ) );
    }

    if ( voidRatioFromEclipse )
    {
        // Export the ratio to a separate inp file for each step
        std::string ratioName     = "RATIO";
        std::string fullPath      = exportBasePath + "_" + ratioName + ".inp";
        auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

        // Use void ratio from first time step
        size_t timeStep = 0;
        bool isOk = writePropertyToFile( model, dataAccess, RimFaultReactivation::Property::VoidRatio, timeStep, fullPath, partNames, "" );
        if ( !isOk ) return { false, "Failed to create " + ratioName + " file." };

        RifInpExportTools::printHeading( stream, "Initial Conditions, TYPE=" + ratioName );
        RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
    }

    if ( useGridTemperature )
    {
        // Export the temperature to a separate inp file for each step
        std::string propertyName  = "TEMPERATURE";
        std::string fullPath      = exportBasePath + "_" + propertyName + ".inp";
        auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

        // Use temperature from first time step
        size_t timeStep = 0;
        bool isOk = writePropertyToFile( model, dataAccess, RimFaultReactivation::Property::Temperature, timeStep, fullPath, partNames, "" );
        if ( !isOk ) return { false, "Failed to create " + propertyName + " file." };

        RifInpExportTools::printHeading( stream, "Initial Conditions, TYPE=" + propertyName );
        RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
    }

    // stress export
    {
        // Export the stress to a separate inp file
        std::string stressName    = "STRESS";
        std::string fullPath      = exportBasePath + "_" + stressName + ".inp";
        auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

        // Use stress from first time step
        size_t timeStep = 0;
        bool   isOk     = writePropertiesToFile( model,
                                           dataAccess,
                                                 { RimFaultReactivation::Property::StressTop,
                                                   RimFaultReactivation::Property::DepthTop,
                                                   RimFaultReactivation::Property::StressBottom,
                                                   RimFaultReactivation::Property::DepthBottom,
                                                   RimFaultReactivation::Property::LateralStressComponentX,
                                                   RimFaultReactivation::Property::LateralStressComponentY },
                                                 {},
                                           timeStep,
                                           fullPath,
                                           partNames,
                                           "",
                                           "" );

        if ( !isOk ) return { false, "Failed to create " + stressName + " file." };

        RifInpExportTools::printHeading( stream, "Initial Conditions, TYPE=" + stressName + ", geostatic" );
        RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::printSteps( std::ostream&                         stream,
                                                                            const RigFaultReactivationModel&      model,
                                                                            const RimFaultReactivationDataAccess& dataAccess,
                                                                            const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                                            const std::vector<QDateTime>& timeSteps,
                                                                            const std::string&            exportBasePath,
                                                                            bool                          useGridPorePressure,
                                                                            bool                          useGridTemperature,
                                                                            double                        seaWaterLoad )
{
    // First time step has to be selected in order to export currently
    if ( timeSteps.size() < 2 ) return { false, "Failed to export fault reactivation INP: needs at least two time steps." };

    for ( int i = 0; i < static_cast<int>( timeSteps.size() ); i++ )
    {
        std::string stepNum  = std::to_string( i + 1 );
        std::string stepName = timeSteps[i].toString( "yyyy-MM-dd" ).toStdString();
        if ( i == 0 ) stepName = "Geostatic_" + stepName;
        std::string stepType = i == 0 ? "Geostatic, utol" : "Soils, utol=1.0";

        RifInpExportTools::printComment( stream, "----------------------------------------------------------------" );
        RifInpExportTools::printSectionComment( stream, "STEP: " + stepName );

        RifInpExportTools::printHeading( stream, "Step, name=" + stepName + ", nlgeom=NO" );

        RifInpExportTools::printHeading( stream, stepType );
        RifInpExportTools::printNumbers( stream, { 1.0, 1.0, 1e-05, 1.0 } );

        if ( i == 0 )
        {
            RifInpExportTools::printComment( stream, "GRAVITY LOADS FROM ROCK AND SEAWATER" );

            RifInpExportTools::printHeading( stream, "Dload" );
            RifInpExportTools::printLine( stream, ",GRAV, 9.80665, 0., 0., -1." );

            RifInpExportTools::printComment( stream, "SEAWATER LOAD" );

            for ( auto [part, partName] : partNames )
            {
                RifInpExportTools::printHeading( stream, "Dsload" );
                std::string seaBedName = partName + "." + "SEABED";
                RifInpExportTools::printLine( stream, seaBedName + ", P, " + std::to_string( seaWaterLoad ) );
            }
        }

        if ( useGridPorePressure )
        {
            RifInpExportTools::printSectionComment( stream, "BOUNDARY CONDITIONS" );

            // Export the pore pressure to a separate inp file for each step
            std::string postfix       = createFilePostfix( "PORE_PRESSURE", stepName );
            std::string fullPath      = exportBasePath + postfix;
            auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

            bool isOk =
                writePropertyToFile( model, dataAccess, RimFaultReactivation::Property::PorePressure, i, fullPath, partNames, "8, 8, " );
            if ( !isOk ) return { false, "Failed to create pore pressure file." };

            RifInpExportTools::printHeading( stream, "Boundary, type=displacement" );
            RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
        }

        if ( useGridTemperature )
        {
            RifInpExportTools::printSectionComment( stream, "TEMPERATURE" );

            // Export the temperature to a separate inp file for each step
            std::string postfix       = createFilePostfix( "TEMPERATURE", stepName );
            std::string fullPath      = exportBasePath + postfix;
            auto [filePath, fileName] = RiaFilePathTools::toFolderAndFileName( QString::fromStdString( fullPath ) );

            bool isOk = writePropertyToFile( model, dataAccess, RimFaultReactivation::Property::Temperature, i, fullPath, partNames, "" );
            if ( !isOk ) return { false, "Failed to create temperature file." };

            RifInpExportTools::printHeading( stream, "Temperature" );
            RifInpExportTools::printHeading( stream, "INCLUDE, input=" + fileName.toStdString() );
        }

        RifInpExportTools::printSectionComment( stream, "OUTPUT" );
        RifInpExportTools::printHeading( stream, "Output, field" );
        RifInpExportTools::printHeading( stream, "Node Output" );
        RifInpExportTools::printLine( stream, "COORD, POR, U" );
        RifInpExportTools::printHeading( stream, "Element Output" );
        RifInpExportTools::printLine( stream, "COORD, VOIDR, S, E, TEMP" );

        RifInpExportTools::printComment( stream, "" );
        RifInpExportTools::printHeading( stream, "Output, history, variable=PRESELECT" );

        RifInpExportTools::printHeading( stream, "End Step" );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultReactivationModelExporter::writePropertyToFile( const RigFaultReactivationModel&                             model,
                                                             const RimFaultReactivationDataAccess&                        dataAccess,
                                                             RimFaultReactivation::Property                               property,
                                                             size_t                                                       outputTimeStep,
                                                             const std::string&                                           filePath,
                                                             const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                             const std::string&                                           additionalData )
{
    std::ofstream stream( filePath );
    if ( !stream.good() ) return false;

    for ( auto [part, partName] : partNames )
    {
        auto grid = model.grid( part );

        const std::vector<cvf::Vec3d>& nodes  = grid->globalNodes();
        const std::vector<double>      values = dataAccess.propertyValues( part, property, outputTimeStep );
        if ( values.size() != nodes.size() ) return false;

        for ( size_t i = 0; i < nodes.size(); i++ )
        {
            std::string line = partName + ".ALL." + std::to_string( i + 1 ) + ", " + additionalData + std::to_string( values[i] );
            RifInpExportTools::printLine( stream, line );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultReactivationModelExporter::writePropertiesToFile( const RigFaultReactivationModel&                             model,
                                                               const RimFaultReactivationDataAccess&                        dataAccess,
                                                               const std::vector<RimFaultReactivation::Property>&           properties,
                                                               const std::vector<std::string>&                              propertyNames,
                                                               size_t                                                       outputTimeStep,
                                                               const std::string&                                           filePath,
                                                               const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                               const std::string&                                           tableName,
                                                               const std::string&                                           heading )
{
    std::ofstream stream( filePath );
    if ( !stream.good() ) return false;

    bool includeHeader = !propertyNames.empty();
    if ( includeHeader )
    {
        RifInpExportTools::printHeading( stream, "Distribution Table, name=" + tableName + "_Table" );
        std::string propertyNamesLine;
        for ( size_t i = 0; i < propertyNames.size(); i++ )
        {
            propertyNamesLine += propertyNames[i];
            if ( i != propertyNames.size() - 1 ) propertyNamesLine += ", ";
        }
        RifInpExportTools::printLine( stream, propertyNamesLine );

        RifInpExportTools::printHeading( stream, "Distribution, name=" + tableName + ", location=ELEMENT, Table=" + tableName + "_Table" );

        RifInpExportTools::printLine( stream, heading );
    }

    for ( auto [part, partName] : partNames )
    {
        auto grid = model.grid( part );

        const std::vector<std::vector<unsigned int>>& elementIndices = grid->elementIndices();
        for ( size_t i = 0; i < elementIndices.size(); i++ )
        {
            std::string line = partName + ".ALL." + std::to_string( i + 1 );
            for ( auto property : properties )
            {
                const std::vector<double> values = dataAccess.propertyValues( part, property, outputTimeStep );
                if ( values.size() != elementIndices.size() ) return false;

                line += ", " + std::to_string( values[i] );
            }
            RifInpExportTools::printLine( stream, line );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printInteractions( std::ostream&                                                stream,
                                                          const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                          const std::vector<std::pair<RimFaultReactivation::BorderSurface, std::string>>& borders )
{
    RifInpExportTools::printSectionComment( stream, "INTERACTIONS" );
    for ( const auto& [border, borderName] : borders )
    {
        if ( border != RimFaultReactivation::BorderSurface::Seabed )
        {
            RifInpExportTools::printComment( stream, "Interaction: " + borderName );

            std::string interactionName = "NON-FAULT";
            std::string extra;
            if ( border == RimFaultReactivation::BorderSurface::FaultSurface )
            {
                interactionName = "FAULT";
                extra           = ", adjust=0.0";
            }

            RifInpExportTools::printHeading( stream,
                                             "Contact Pair, interaction=" + interactionName + ", small sliding, type=SURFACE TO SURFACE" +
                                                 extra );

            std::string part1Name = partNames.find( RimFaultReactivation::GridPart::FW )->second;
            std::string part2Name = partNames.find( RimFaultReactivation::GridPart::HW )->second;
            RifInpExportTools::printLine( stream, part1Name + "." + borderName + ", " + part2Name + "." + borderName );
        }
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifFaultReactivationModelExporter::createFilePostfix( const std::string& title, const std::string& stepName )
{
    return QString( "_%1_%2.inp" ).arg( QString::fromStdString( title ) ).arg( QString::fromStdString( stepName ) ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultReactivationModelExporter::exportModelSettings( const RimFaultReactivationModel& rimModel )
{
    auto model = rimModel.model();

    if ( model.isNull() ) return false;
    if ( !model->isValid() ) return false;

    QMap<QString, QVariant> settings;

    auto [topPosition, bottomPosition] = model->faultTopBottom();
    auto faultNormal                   = model->modelNormal();

    // make sure we export in local coordinates, if that is used
    topPosition    = model->transformPointIfNeeded( topPosition );
    bottomPosition = model->transformPointIfNeeded( bottomPosition );

    // make sure we move horizontally, and along the 2D model
    faultNormal.z() = 0.0;
    faultNormal.normalize();
    faultNormal = faultNormal ^ cvf::Vec3d::Z_AXIS;

    RimFaultReactivationTools::addSettingsToMap( settings, faultNormal, topPosition, bottomPosition );
    return ResInsightInternalJson::JsonWriter::encodeFile( QString::fromStdString( rimModel.settingsFilename() ), settings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RimFaultReactivationDataAccess>
    RifFaultReactivationModelExporter::extractAndExportModelData( const RimFaultReactivationModel& rimModel )
{
    if ( !exportModelSettings( rimModel ) ) return nullptr;

    auto eCase = rimModel.eclipseCase();
    if ( eCase == nullptr ) return nullptr;

    // extract data for each timestep
    auto dataAccess = std::make_shared<RimFaultReactivationDataAccess>( rimModel,
                                                                        eCase,
                                                                        rimModel.geoMechCase(),
                                                                        rimModel.selectedTimeStepIndexes(),
                                                                        rimModel.stressSource() );
    dataAccess->extractModelData( *rimModel.model() );
    return dataAccess;
}
