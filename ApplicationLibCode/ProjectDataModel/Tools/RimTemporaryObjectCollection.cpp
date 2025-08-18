/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimTemporaryObjectCollection.h"

CAF_PDM_SOURCE_INIT( RimTemporaryObjectCollection, "RimTemporaryObjectCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTemporaryObjectCollection::RimTemporaryObjectCollection()
{
    CAF_PDM_InitObject( "Temporary Object Collection" );

    CAF_PDM_InitFieldNoDefault( &m_temporaryObjects, "TemporaryObjects", "Temporary Objects" );
    m_temporaryObjects.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTemporaryObjectCollection* RimTemporaryObjectCollection::instance()
{
    static RimTemporaryObjectCollection staticInstance;

    return &staticInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTemporaryObjectCollection::addTemporaryObject( caf::PdmObject* object )
{
    m_temporaryObjects.push_back( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTemporaryObjectCollection::clearTemporaryObjects()
{
    m_temporaryObjects.deleteChildren();
}
