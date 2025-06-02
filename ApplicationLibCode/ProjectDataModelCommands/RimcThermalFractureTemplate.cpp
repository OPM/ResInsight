/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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
#include "RimcThermalFractureTemplate.h"

#include "RifThermalFractureTemplateSurfaceExporter.h"

#include "RimThermalFractureTemplate.h"

#include "RimcDataContainerString.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimThermalFractureTemplate, RimcThermalFractureTemplate_exportToFile, "ExportToFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcThermalFractureTemplate_exportToFile::RimcThermalFractureTemplate_exportToFile( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Thermal Fracture Template", "", "", "Export Thermal Fracture Template to File" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_filePath, "FilePath", "", "", "", "File Path" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcThermalFractureTemplate_exportToFile::execute()
{
    RimThermalFractureTemplate* thermalFracture = self<RimThermalFractureTemplate>();
    if ( thermalFracture && thermalFracture->fractureDefinition() )
        RifThermalFractureTemplateSurfaceExporter::writeToFile( thermalFracture, m_timeStep(), m_filePath() );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcThermalFractureTemplate_exportToFile::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcThermalFractureTemplate_exportToFile::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimThermalFractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcThermalFractureTemplate_exportToFile::isNullptrValidResult_obsolete() const
{
    return true;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimThermalFractureTemplate, RimcThermalFractureTemplate_timeSteps, "TimeSteps" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcThermalFractureTemplate_timeSteps::RimcThermalFractureTemplate_timeSteps( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Get Thermal Fracture Template Time Steps", "", "", "Get Thermal Fracture Template Time Steps" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcThermalFractureTemplate_timeSteps::execute()
{
    RimThermalFractureTemplate* thermalFracture = self<RimThermalFractureTemplate>();
    auto                        timeSteps       = thermalFracture->timeStepsStrings();

    auto dataObject            = new RimcDataContainerString;
    dataObject->m_stringValues = timeSteps;

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcThermalFractureTemplate_timeSteps::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcThermalFractureTemplate_timeSteps::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcThermalFractureTemplate_timeSteps::isNullptrValidResult_obsolete() const
{
    return true;
}
