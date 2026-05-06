/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimcDataFilterCollection.h"

#include "RimCellFilterCollection.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimDataFilterCollection, RimcDataFilterCollection_addCombinedFilter, "AddCombinedFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcDataFilterCollection_addCombinedFilter::RimcDataFilterCollection_addCombinedFilter( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Combined Filter", "", "", "Add a new combined filter to the data filter collection" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_combineMode, "CombineMode", "Combine Mode (AND/OR)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcDataFilterCollection_addCombinedFilter::execute()
{
    auto* coll = self<RimDataFilterCollection>();
    if ( !coll ) return std::unexpected( QString( "Self is not a RimDataFilterCollection" ) );

    RimCombinedFilter* combined = coll->addNewCombinedFilter();
    if ( !combined ) return std::unexpected( QString( "Failed to create combined filter" ) );

    if ( !m_name().isEmpty() )
    {
        combined->setName( m_name() );
    }

    const QString modeStr = m_combineMode().trimmed().toUpper();
    if ( modeStr == "AND" )
    {
        combined->setCombineMode( RimCellFilterCollection::AND );
    }
    else if ( modeStr == "OR" )
    {
        combined->setCombineMode( RimCellFilterCollection::OR );
    }

    coll->updateConnectedEditors();

    return combined;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcDataFilterCollection_addCombinedFilter::classKeywordReturnedType() const
{
    return RimCombinedFilter::classKeywordStatic();
}
