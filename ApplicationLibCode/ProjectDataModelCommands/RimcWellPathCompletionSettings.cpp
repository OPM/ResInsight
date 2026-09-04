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

#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSegmentCollection, RimcSegmentCollection_addSegmentInterval, "AddSegmentInterval" );

RimcSegmentCollection_addSegmentInterval::RimcSegmentCollection_addSegmentInterval( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Segment Interval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 100.0, "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.152, "Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor, "RoughnessFactor", 1.0e-5, "Roughness Factor" );
}

std::expected<caf::PdmObjectHandle*, QString> RimcSegmentCollection_addSegmentInterval::execute()
{
    auto* segmentCollection = self<RimSegmentCollection>();
    if ( !segmentCollection ) return std::unexpected( "Segment collection is null. Cannot add segment interval." );
    if ( m_endMD() <= m_startMD() ) return std::unexpected( "End MD must be greater than Start MD." );

    segmentCollection->setDiameterRoughnessMode( RimSegmentCollection::DiameterRoughnessMode::INTERVALS );
    auto* interval = segmentCollection->createInterval( m_startMD(), m_endMD(), m_diameter(), m_roughnessFactor() );
    segmentCollection->updateAllRequiredEditors();
    return interval;
}

QString RimcSegmentCollection_addSegmentInterval::classKeywordReturnedType() const
{
    return RimSegmentInterval::classKeywordStatic();
}
