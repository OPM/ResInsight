/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimFontSizeField.h"

#include "RiaPreferences.h"

#include "cafAppEnum.h"
#include "cafPdmUiFieldHandle.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFontSizeField::configureCapabilities()
{
    if ( auto uiFieldHandle = uiCapability() )
    {
        auto callback = []() -> QList<caf::PdmOptionItemInfo>
        { return caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() ); };

        uiFieldHandle->setValueOptionsGenerator( callback );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFontSizeField& RimFontSizeField::operator=( const caf::FontTools::RelativeSize& value )
{
    setValue( value );
    return *this;
}
