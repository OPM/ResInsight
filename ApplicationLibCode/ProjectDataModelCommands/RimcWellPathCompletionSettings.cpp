/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimcWellPathCompletionSettings.h"

#include "RimDiameterRoughnessInterval.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimWellPathCompletionSettings.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCompletionSettings,
                                   RimcWellPathCompletionSettings_addDiameterRoughnessInterval,
                                   "AddDiameterRoughnessInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCompletionSettings_addDiameterRoughnessInterval::RimcWellPathCompletionSettings_addDiameterRoughnessInterval( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Diameter Roughness Interval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 100.0, "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.152, "Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor, "RoughnessFactor", 1.0e-5, "Roughness Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCompletionSettings_addDiameterRoughnessInterval::execute()
{
    auto completionSettings = self<RimWellPathCompletionSettings>();
    if ( !completionSettings )
    {
        return std::unexpected( QString( "Completion settings is null. Cannot add diameter roughness interval." ) );
    }

    auto mswParams = completionSettings->mswCompletionParameters();
    if ( !mswParams )
    {
        return std::unexpected( QString( "MSW completion parameters is null. Cannot add diameter roughness interval." ) );
    }

    auto intervalCollection = mswParams->diameterRoughnessIntervals();
    if ( !intervalCollection )
    {
        return std::unexpected( QString( "Diameter roughness interval collection is null. Cannot add interval." ) );
    }

    // Ensure we're in interval-specific mode
    mswParams->setDiameterRoughnessMode( RimMswCompletionParameters::DiameterRoughnessMode::INTERVALS );

    // Create new interval
    auto* newInterval = intervalCollection->createInterval( m_startMD(), m_endMD(), m_diameter(), m_roughnessFactor() );

    if ( !newInterval )
    {
        return std::unexpected( QString( "Failed to create diameter roughness interval." ) );
    }

    completionSettings->updateAllRequiredEditors();

    return newInterval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCompletionSettings_addDiameterRoughnessInterval::classKeywordReturnedType() const
{
    return RimDiameterRoughnessInterval::classKeywordStatic();
}
