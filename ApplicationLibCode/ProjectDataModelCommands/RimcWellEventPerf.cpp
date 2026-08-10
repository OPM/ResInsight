/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcWellEventPerf.h"

#include "RimCellFilter.h"
#include "RimWellEventPerf.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <expected>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventPerf, RimcWellEventPerf_addFilter, "AddFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventPerf_addFilter::RimcWellEventPerf_addFilter( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Add Cell Filter", "", "", "Set the cell filter associated with this perforation event (replaces any existing filter)" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filter, "Filter", "", "", "", "Cell Filter" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventPerf_addFilter::execute()
{
    auto* perfEvent = self<RimWellEventPerf>();
    perfEvent->setCellFilter( m_filter() );
    perfEvent->updateConnectedEditors();

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventPerf, RimcWellEventPerf_cellFilter, "cell_filter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventPerf_cellFilter::RimcWellEventPerf_cellFilter( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Cell Filter", "", "", "Cell filter associated with this perforation event, or null if none" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventPerf_cellFilter::execute()
{
    return self<RimWellEventPerf>()->cellFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventPerf_cellFilter::classKeywordReturnedType() const
{
    return RimCellFilter::classKeywordStatic();
}
