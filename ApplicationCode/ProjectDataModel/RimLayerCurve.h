/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModelPropertyCurve.h"
#include "RimWellLogExtractionCurve.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimFractureModel;
class RimColorLegend;

//==================================================================================================
///
//==================================================================================================
class RimLayerCurve : public RimWellLogExtractionCurve, public RimFractureModelPropertyCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimLayerCurve();
    ~RimLayerCurve() override;

    void setFractureModel( RimFractureModel* fractureModel );

    void setEclipseResultCategory( RiaDefines::ResultCatType catType );

    void                      setCurveProperty( RiaDefines::CurveProperty ) override;
    RiaDefines::CurveProperty curveProperty() const override;

protected:
    QString createCurveAutoName() override;

    void performDataExtraction( bool* isUsingPseudoLength ) override;

    // static QString findFaciesName( const RimColorLegend& colorLegend, double value );
    static QString findFormationNameForDepth( const std::vector<QString>&                   formationNames,
                                              const std::vector<std::pair<double, double>>& depthRanges,
                                              double                                        depth );

    caf::PdmPtrField<RimFractureModel*>                    m_fractureModel;
    caf::PdmField<caf::AppEnum<RiaDefines::CurveProperty>> m_curveProperty;
};
