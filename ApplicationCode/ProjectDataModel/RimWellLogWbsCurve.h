/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellLogExtractionCurve.h"

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogWbsCurve : public RimWellLogExtractionCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogWbsCurve();

    bool   smoothCurve() const;
    double smoothingThreshold() const;

    void setSmoothCurve( bool smooth );
    void setSmoothingThreshold( double threshold );

protected:
    void performDataExtraction( bool* isUsingPseudoLength ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    caf::PdmField<bool>   m_smoothCurve;
    caf::PdmField<double> m_smoothingThreshold;
};
