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

#pragma once

#include "cafFontTools.h"
#include "cafPdmField.h"
#include "cafPdmFieldScriptingCapability.h"

//==================================================================================================
///
///
//==================================================================================================
class RimFontSizeField : public caf::PdmField<caf::FontTools::RelativeSizeEnum>
{
public:
    void configureCapabilities() override;

    RimFontSizeField& operator=( const caf::FontTools::RelativeSize& value );
};

namespace caf
{
template <>
class PdmFieldScriptingCapability<RimFontSizeField> : public PdmFieldScriptingCapability<PdmField<caf::FontTools::RelativeSizeEnum>>
{
public:
    PdmFieldScriptingCapability( RimFontSizeField* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptingCapability<PdmField<caf::FontTools::RelativeSizeEnum>>( field, fieldName, giveOwnership )
    {
    }

    QString dataType() const override { return "str"; }
};

}; // namespace caf
