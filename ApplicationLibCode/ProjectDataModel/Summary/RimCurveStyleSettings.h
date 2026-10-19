/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <string>

class RifEclipseSummaryAddress;
class RimSummaryCase;

//==================================================================================================
/// Global definition of which data dimension controls the curve color and the curve symbol.
/// Used by summary plots that have "Use Global Curve Appearance" enabled.
//==================================================================================================
class RimCurveStyleSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class Dimension
    {
        NONE,
        CASE,
        VECTOR,
        WELL,
        GROUP,
        NETWORK,
        REGION,
        REGION_2_REGION,
        AQUIFER,
        LGR,
        BLOCK,
        CONNECTION,
        SEGMENT
    };
    using DimensionEnum = caf::AppEnum<Dimension>;

    RimCurveStyleSettings();

    Dimension colorDimension() const;
    Dimension symbolDimension() const;
    void      setColorDimension( Dimension dimension );
    void      setSymbolDimension( Dimension dimension );

    // The value of the dimension for the address, empty if the address has no value for the dimension
    static std::string dimensionValue( Dimension dimension, const RifEclipseSummaryAddress& address, const std::string& caseName );

    // The case name used by dimensionValue: ensemble name for ensemble members, otherwise the display name
    static std::string caseName( const RimSummaryCase* summaryCase );

    // Adds the color and symbol fields, used both here and when shown together with other items
    void appendFieldsToUiOrdering( caf::PdmUiOrdering& uiOrdering );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<DimensionEnum> m_colorDimension;
    caf::PdmField<DimensionEnum> m_symbolDimension;
};
