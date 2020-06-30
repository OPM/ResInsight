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

#include "RimWellLogExtractionCurve.h"

#include "RiuQwtSymbol.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimWellPath;
class RimWellMeasurement;
class RimFractureModel;
class RimColorLegend;

//==================================================================================================
///
//==================================================================================================
class RimElasticPropertiesCurve : public RimWellLogExtractionCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class PropertyType
    {
        YOUNGS_MODULUS,
        POISSONS_RATIO,
        K_IC,
        PROPPANT_EMBEDMENT,
        BIOT_COEFFICIENT,
        K0,
        FLUID_LOSS_COEFFICIENT,
        SPURT_LOSS
    };

    RimElasticPropertiesCurve();
    ~RimElasticPropertiesCurve() override;

    void setFractureModel( RimFractureModel* fractureModel );

    void setEclipseResultCategory( RiaDefines::ResultCatType catType );

    void setPropertyType( PropertyType propertyType );

protected:
    QString createCurveAutoName() override;

    void performDataExtraction( bool* isUsingPseudoLength ) override;

    static QString findFaciesName( const RimColorLegend& colorLegend, double value );
    static double  findFaciesValue( const RimColorLegend& colorLegend, const QString& name );

    static void addOverburden( std::vector<QString>& formationNames,
                               std::vector<double>&  formationValues,
                               std::vector<double>&  faciesValues,
                               std::vector<double>&  tvDepthValues,
                               std::vector<double>&  measuredDepthValues,
                               double                overburdenHeight,
                               double                defaultPoroValue,
                               const QString&        formationName,
                               double                faciesValue );

    static void addUnderburden( std::vector<QString>& formationNames,
                                std::vector<double>&  formationValues,
                                std::vector<double>&  faciesValues,
                                std::vector<double>&  tvDepthValues,
                                std::vector<double>&  measuredDepthValues,
                                double                overburdenHeight,
                                double                defaultPoroValue,
                                const QString&        formationName,
                                double                faciesValue );

    caf::PdmPtrField<RimFractureModel*>       m_fractureModel;
    caf::PdmField<caf::AppEnum<PropertyType>> m_propertyType;
};
