/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//  Copyright (C) 2018-     Ceetron Solutions AS
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

#include "RiaFieldHandleTools.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmXmlFieldHandle.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFieldHandleTools::disableWriteAndSetFieldHidden( caf::PdmFieldHandle* fieldHandle )
{
    CVF_ASSERT( fieldHandle );

    if ( fieldHandle->uiCapability() )
    {
        if ( dynamic_cast<caf::PdmChildFieldHandle*>( fieldHandle ) || dynamic_cast<caf::PdmChildArrayFieldHandle*>( fieldHandle ) )
            fieldHandle->uiCapability()->setUiTreeHidden( true );
        else
            fieldHandle->uiCapability()->setUiHidden( true );
    }

    if ( fieldHandle->xmlCapability() )
    {
        fieldHandle->xmlCapability()->setIOWritable( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFieldHandleTools::updateOverrideStateAndLabel( caf::PdmFieldHandle* fieldHandle, bool isOverridden, const QString& toolTip )
{
    // Get the label text as given by the init_field macro
    QString labelText = fieldHandle->uiCapability()->uiName( caf::PdmUiItem::uiConfigNameForStaticData() );

    if ( isOverridden ) labelText += " (overridden)";
    fieldHandle->uiCapability()->setUiToolTip( toolTip );
    fieldHandle->uiCapability()->setUiName( labelText );
    fieldHandle->uiCapability()->setUiReadOnly( isOverridden );
}
