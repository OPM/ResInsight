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
#include "RiaVersionInfo.h"

#include "RifInpExportTools.h"

#include <fstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::exportToStream( std::ostream& stream, const RimFaultReactivationModel& rimModel )
{
    std::string applicationNameAndVersion = std::string( RI_APPLICATION_NAME ) + " " + std::string( STRPRODUCTVER );

    using PartBorderSurface                                        = RigGriddedPart3d::BorderSurface;
    std::vector<std::pair<PartBorderSurface, std::string>> borders = { { PartBorderSurface::UpperSurface, "TOP" },
                                                                       { PartBorderSurface::FaultSurface, "FAULT" },
                                                                       { PartBorderSurface::LowerSurface, "BASE" } };

    // The two parts are "mirrored", so face number 4 should of the two parts should face eachother.
    using FaultGridPart                                              = RigFaultReactivationModel::GridPart;
    std::map<std::pair<FaultGridPart, PartBorderSurface>, int> faces = { { { FaultGridPart::PART1, PartBorderSurface::FaultSurface }, 4 },
                                                                         { { FaultGridPart::PART1, PartBorderSurface::UpperSurface }, 4 },
                                                                         { { FaultGridPart::PART1, PartBorderSurface::LowerSurface }, 4 },
                                                                         { { FaultGridPart::PART2, PartBorderSurface::FaultSurface }, 4 },
                                                                         { { FaultGridPart::PART2, PartBorderSurface::UpperSurface }, 4 },
                                                                         { { FaultGridPart::PART2, PartBorderSurface::LowerSurface }, 4 } };

    std::map<FaultGridPart, std::string> partNames = {
        { FaultGridPart::PART1, "LEFT_PART" },
        { FaultGridPart::PART2, "RIGHT_PART" },
    };

    std::map<RigGriddedPart3d::Boundary, std::string> boundaries = {
        { RigGriddedPart3d::Boundary::Bottom, "BOTTOM" },
        { RigGriddedPart3d::Boundary::Back, "BACK" },
        { RigGriddedPart3d::Boundary::Front, "FRONT" },
        { RigGriddedPart3d::Boundary::FarSide, "FARSIDE" },
    };

    double faultFriction = 0.0;

    auto model = rimModel.model();
    CAF_ASSERT( !model.isNull() );
    printHeading( stream, applicationNameAndVersion );
    printParts( stream, *model, partNames, borders, faces, boundaries, rimModel.localCoordSysNormalsXY() );
    printAssembly( stream, *model, partNames );
    printMaterials( stream );
    printInteractionProperties( stream, faultFriction );
    printBoundaryConditions( stream, *model, partNames, boundaries );
    printPredefinedFields( stream, partNames );
    printInteractions( stream, partNames, borders );
    printSteps( stream, partNames );

    // TODO: improve error handling
    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::exportToFile( const std::string&               filePath,
                                                                              const RimFaultReactivationModel& model )
{
    std::ofstream stream( filePath );
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
    std::ostream&                                                                                         stream,
    const RigFaultReactivationModel&                                                                      model,
    const std::map<RigFaultReactivationModel::GridPart, std::string>&                                     partNames,
    const std::vector<std::pair<RigGriddedPart3d::BorderSurface, std::string>>&                           borders,
    const std::map<std::pair<RigFaultReactivationModel::GridPart, RigGriddedPart3d::BorderSurface>, int>& faces,
    const std::map<RigGriddedPart3d::Boundary, std::string>&                                              boundaries,
    const std::pair<cvf::Vec3d, cvf::Vec3d>&                                                              orientation )
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

        RifInpExportTools::printNodeSet( stream, partName, 1, nodes.size(), false );
        RifInpExportTools::printElementSet( stream, partName, 1, elements.size() );

        RifInpExportTools::printNodeSet( stream, "PORE_PRESSURE", 1, nodes.size(), true );

        const std::map<RigGriddedPart3d::BorderSurface, std::vector<unsigned int>>& borderSurfaceElements = grid->borderSurfaceElements();

        for ( auto [boundary, boundaryName] : boundaries )
        {
            // Create boundary condition sets for each side of the parts (except top).
            auto boundaryNodes    = grid->boundaryNodes();
            auto boundaryElements = grid->boundaryElements();

            const std::vector<unsigned int>& nodes = boundaryNodes[boundary];
            RifInpExportTools::printNodeSet( stream, boundaryName, false, nodes );

            const std::vector<unsigned int>& elements = boundaryElements[boundary];
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

        // Print local orientation
        std::string orientationName = "ori";
        RifInpExportTools::printHeading( stream, "Orientation, name=" + orientationName );
        auto [dir1, dir2] = orientation;
        RifInpExportTools::printNumbers( stream, { dir1.x(), dir1.y(), dir1.z(), dir2.x(), dir2.y(), dir2.z() } );
        RifInpExportTools::printLine( stream, "3, 0.0" );

        RifInpExportTools::printComment( stream, "Section: sand" );
        RifInpExportTools::printHeading( stream, "Solid Section, elset=" + partName + ", orientation=" + orientationName + ", material=sand" );

        RifInpExportTools::printLine( stream, "," );
        RifInpExportTools::printHeading( stream, "End Part" );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printAssembly( std::ostream&                                                     stream,
                                                      const RigFaultReactivationModel&                                  model,
                                                      const std::map<RigFaultReactivationModel::GridPart, std::string>& partNames )
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

    RifInpExportTools::printHeading( stream, "End Assembly" );

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifFaultReactivationModelExporter::printMaterials( std::ostream& stream )
{
    // MATERIALS PART
    struct Material
    {
        std::string name;
        double      density;
        double      elastic1;
        double      elastic2;
        double      permeability1;
        double      permeability2;
    };

    RifInpExportTools::printSectionComment( stream, "MATERIALS" );

    std::vector<Material> materials = {
        Material{ .name = "sand", .density = 2000.0, .elastic1 = 5e+09, .elastic2 = 0.2, .permeability1 = 1e-09, .permeability2 = 0.3 } };
    for ( Material mat : materials )
    {
        RifInpExportTools::printHeading( stream, "Material, name=" + mat.name );
        RifInpExportTools::printHeading( stream, "Density" );
        RifInpExportTools::printNumber( stream, mat.density );
        RifInpExportTools::printHeading( stream, "Elastic" );
        RifInpExportTools::printNumbers( stream, { mat.elastic1, mat.elastic2 } );

        RifInpExportTools::printHeading( stream, "Permeability, specific=1." );
        RifInpExportTools::printNumbers( stream, { mat.permeability1, mat.permeability2 } );
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
    RifFaultReactivationModelExporter::printBoundaryConditions( std::ostream&                                                     stream,
                                                                const RigFaultReactivationModel&                                  model,
                                                                const std::map<RigFaultReactivationModel::GridPart, std::string>& partNames,
                                                                const std::map<RigGriddedPart3d::Boundary, std::string>& boundaries )
{
    auto printBoundaryCondition = []( std::ostream& stream, const std::string& boundarySetName, const std::string& symmetryType )
    {
        RifInpExportTools::printHeading( stream, "Boundary" );
        RifInpExportTools::printLine( stream, boundarySetName + ", " + symmetryType );
    };

    std::map<RigGriddedPart3d::Boundary, std::string> symmetryTypes = {
        { RigGriddedPart3d::Boundary::Bottom, "ZSYMM" },
        { RigGriddedPart3d::Boundary::Back, "YSYMM" },
        { RigGriddedPart3d::Boundary::Front, "YSYMM" },
        { RigGriddedPart3d::Boundary::FarSide, "XSYMM" },

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

    std::string partSymmetry = "XSYMM";
    for ( const auto& part : parts )
    {
        auto partNameIt = partNames.find( part );
        CAF_ASSERT( partNameIt != partNames.end() );
        std::string partName = partNameIt->second;
        printBoundaryCondition( stream, partName + "." + partName, partSymmetry );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printPredefinedFields( std::ostream&                                                     stream,
                                                              const std::map<RigFaultReactivationModel::GridPart, std::string>& partNames )
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
        std::string name = partName + "." + partName;
        fields.push_back( PredefinedField{ .initialConditionType = "RATIO", .partName = name, .value = 0.3 } );
        fields.push_back( PredefinedField{ .initialConditionType = "PORE PRESSURE", .partName = name, .value = 0.0 } );
    }

    RifInpExportTools::printSectionComment( stream, "PREDEFINED FIELDS" );

    for ( auto field : fields )
    {
        RifInpExportTools::printHeading( stream, "Initial Conditions, TYPE=" + field.initialConditionType );
        RifInpExportTools::printLine( stream, field.partName + ", " + std::to_string( field.value ) );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printSteps( std::ostream&                                                     stream,
                                                   const std::map<RigFaultReactivationModel::GridPart, std::string>& partNames )
{
    int numSteps = 2;

    for ( int i = 0; i < numSteps; i++ )
    {
        std::string stepNum  = std::to_string( i + 1 );
        std::string stepName = "Step-" + stepNum;
        RifInpExportTools::printComment( stream, "----------------------------------------------------------------" );
        RifInpExportTools::printSectionComment( stream, "STEP: " + stepName );

        RifInpExportTools::printHeading( stream, "Step, name=" + stepName + ", nlgeom=NO" );

        std::string stepType = i == 0 ? "Geostatic, utol" : "Soils, utol=1.0";
        RifInpExportTools::printHeading( stream, stepType );
        RifInpExportTools::printNumbers( stream, { 1.0, 1.0, 1e-05, 1.0 } );

        RifInpExportTools::printSectionComment( stream, "BOUNDARY CONDITIONS" );
        RifInpExportTools::printHeading( stream, "Boundary" );

        std::string part1Name = partNames.find( RigFaultReactivationModel::GridPart::PART1 )->second;
        std::string part2Name = partNames.find( RigFaultReactivationModel::GridPart::PART2 )->second;

        std::string ppName = "PORE_PRESSURE";
        RifInpExportTools::printLine( stream, part1Name + "." + ppName + ", 8, 8" );
        RifInpExportTools::printHeading( stream, "Boundary" );
        std::string extra = i != 0 ? ", 1e+07" : "";
        RifInpExportTools::printLine( stream, part2Name + "." + ppName + ", 8, 8" + extra );

        RifInpExportTools::printSectionComment( stream, "OUTPUT REQUESTS" );
        RifInpExportTools::printHeading( stream, "Restart, write, frequency=0" );

        RifInpExportTools::printSectionComment( stream, "FIELD OUTPUT: F-Output-" + stepNum );
        RifInpExportTools::printHeading( stream, "Output, field, variable=PRESELECT" );

        RifInpExportTools::printSectionComment( stream, "HISTORY OUTPUT: H-Output-" + stepNum );
        RifInpExportTools::printHeading( stream, "Output, history, variable=PRESELECT" );

        RifInpExportTools::printHeading( stream, "End Step" );
    }

    return { true, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string>
    RifFaultReactivationModelExporter::printInteractions( std::ostream&                                                     stream,
                                                          const std::map<RigFaultReactivationModel::GridPart, std::string>& partNames,
                                                          const std::vector<std::pair<RigGriddedPart3d::BorderSurface, std::string>>& borders )
{
    RifInpExportTools::printSectionComment( stream, "INTERACTIONS" );
    for ( const auto& [border, borderName] : borders )
    {
        RifInpExportTools::printComment( stream, "Interaction: " + borderName );

        std::string interactionName = "NON-FAULT";
        std::string extra;
        if ( border == RigGriddedPart3d::BorderSurface::FaultSurface )
        {
            interactionName = "FAULT";
            extra           = ", adjust=0.0";
        }

        RifInpExportTools::printHeading( stream,
                                         "Contact Pair, interaction=" + interactionName + ", small sliding, type=SURFACE TO SURFACE" + extra );

        std::string part1Name = partNames.find( RigFaultReactivationModel::GridPart::PART1 )->second;
        std::string part2Name = partNames.find( RigFaultReactivationModel::GridPart::PART2 )->second;
        RifInpExportTools::printLine( stream, part1Name + "." + borderName + ", " + part2Name + "." + borderName );
    }

    return { true, "" };
}
