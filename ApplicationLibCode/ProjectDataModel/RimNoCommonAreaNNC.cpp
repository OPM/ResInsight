/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimNoCommonAreaNNC.h"

CAF_PDM_SOURCE_INIT( RimNoCommonAreaNNC, "NoCommonAreaNNC" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNoCommonAreaNNC::RimNoCommonAreaNNC()
{
    CAF_PDM_InitObject( "RimNoCommonAreaNNC", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &name, "Name", "Name", "", "", "" );
    name.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimNoCommonAreaNNC::userDescriptionField()
{
    return &name;
}
