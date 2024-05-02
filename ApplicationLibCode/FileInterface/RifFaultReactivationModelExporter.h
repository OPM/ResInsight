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

#pragma once

#include "RigFaultReactivationModel.h"
#include "RigGriddedPart3d.h"

#include "RimFaultReactivationDataAccess.h"
#include "RimFaultReactivationModel.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RifFaultReactivationModelExporter
{
public:
    static std::pair<bool, std::string> exportToStream( std::ostream& stream, const RimFaultReactivationModel& model );
    static std::pair<bool, std::string> exportToFile( const RimFaultReactivationModel& model );

private:
    static std::pair<bool, std::string> printHeading( std::ostream& stream, const std::string& applicationNameAndVersion );
    static std::pair<bool, std::string>
        printParts( std::ostream&                                                                                        stream,
                    const RigFaultReactivationModel&                                                                     model,
                    const std::map<RimFaultReactivation::GridPart, std::string>&                                         partNames,
                    const std::vector<std::pair<RimFaultReactivation::BorderSurface, std::string>>&                      borders,
                    const std::map<std::pair<RimFaultReactivation::GridPart, RimFaultReactivation::BorderSurface>, int>& faces,
                    const std::map<RimFaultReactivation::Boundary, std::string>&                                         boundaries,
                    const std::map<RimFaultReactivation::ElementSets, std::string>&                                      materialNames );

    static std::pair<bool, std::string> printAssembly( std::ostream&                                                stream,
                                                       const RigFaultReactivationModel&                             model,
                                                       const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                       bool                                                         includeTransform,
                                                       const std::pair<cvf::Vec3d, cvf::Vec3d>&                     transform );

    static std::pair<bool, std::string> printMaterials( std::ostream&                                                   stream,
                                                        const RimFaultReactivationModel&                                rimModel,
                                                        const std::map<RimFaultReactivation::ElementSets, std::string>& materialNames,
                                                        const RimFaultReactivationDataAccess&                           dataAccess,
                                                        const std::string&                                              exportDirectory,
                                                        const std::map<RimFaultReactivation::GridPart, std::string>&    partNames,
                                                        bool                                                            densityFromGrid,
                                                        bool elasticPropertiesFromGrid );

    static std::pair<bool, std::string> printInteractionProperties( std::ostream& stream, double faultFriction );
    static std::pair<bool, std::string> printBoundaryConditions( std::ostream&                                                stream,
                                                                 const RigFaultReactivationModel&                             model,
                                                                 const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                                 const std::map<RimFaultReactivation::Boundary, std::string>& boundaries );
    static std::pair<bool, std::string> printPredefinedFields( std::ostream&                                                stream,
                                                               const RigFaultReactivationModel&                             model,
                                                               const RimFaultReactivationDataAccess&                        dataAccess,
                                                               const std::string&                                           exportDirectory,
                                                               const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                               bool useGridVoidRatio,
                                                               bool useGridTemperature );
    static std::pair<bool, std::string> printSteps( std::ostream&                                                stream,
                                                    const RigFaultReactivationModel&                             model,
                                                    const RimFaultReactivationDataAccess&                        dataAccess,
                                                    const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                                    const std::vector<QDateTime>&                                timeSteps,
                                                    const std::string&                                           exportDirectory,
                                                    bool                                                         useGridPorePressure,
                                                    bool                                                         useGridTemperature,
                                                    double                                                       seaWaterLoad );

    static std::pair<bool, std::string>
        printInteractions( std::ostream&                                                                   stream,
                           const std::map<RimFaultReactivation::GridPart, std::string>&                    partNames,
                           const std::vector<std::pair<RimFaultReactivation::BorderSurface, std::string>>& borders );

    static bool writePropertyToFile( const RigFaultReactivationModel&                             model,
                                     const RimFaultReactivationDataAccess&                        dataAccess,
                                     RimFaultReactivation::Property                               property,
                                     size_t                                                       outputTimeStep,
                                     const std::string&                                           filePath,
                                     const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                     const std::string&                                           additionalData );

    static bool writePropertiesToFile( const RigFaultReactivationModel&                             model,
                                       const RimFaultReactivationDataAccess&                        dataAccess,
                                       const std::vector<RimFaultReactivation::Property>&           properties,
                                       const std::vector<std::string>&                              propertyNames,
                                       size_t                                                       outputTimeStep,
                                       const std::string&                                           filePath,
                                       const std::map<RimFaultReactivation::GridPart, std::string>& partNames,
                                       const std::string&                                           tableName,
                                       const std::string&                                           heading );

    static std::string createFilePostfix( const std::string& title, const std::string& stepName );

    static bool                                            exportModelSettings( const RimFaultReactivationModel& model );
    static std::shared_ptr<RimFaultReactivationDataAccess> extractAndExportModelData( const RimFaultReactivationModel& model );
};
