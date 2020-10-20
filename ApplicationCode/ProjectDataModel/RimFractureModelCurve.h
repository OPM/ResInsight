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

#include "RimFractureModelPropertyCurve.h"
#include "RimWellLogExtractionCurve.h"

#include "RiuQwtSymbol.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimWellPath;
class RimWellMeasurement;
class RimFractureModel;
class RimEclipseInputPropertyCollection;
class RigEclipseCaseData;
class RigResultAccessor;

//==================================================================================================
///
//==================================================================================================
class RimFractureModelCurve : public RimWellLogExtractionCurve, public RimFractureModelPropertyCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class MissingValueStrategy
    {
        DEFAULT_VALUE,
        LINEAR_INTERPOLATION,
        OTHER_CURVE_PROPERTY
    };

    enum class BurdenStrategy
    {
        DEFAULT_VALUE,
        GRADIENT
    };

    RimFractureModelCurve();
    ~RimFractureModelCurve() override;

    void setFractureModel( RimFractureModel* fractureModel );

    void setEclipseResultCategory( RiaDefines::ResultCatType catType );

    void setMissingValueStrategy( MissingValueStrategy strategy );

    void setBurdenStrategy( BurdenStrategy strategy );

    void                      setCurveProperty( RiaDefines::CurveProperty ) override;
    RiaDefines::CurveProperty curveProperty() const override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void performDataExtraction( bool* isUsingPseudoLength ) override;

    static bool hasMissingValues( const std::vector<double>& values );
    static void replaceMissingValues( std::vector<double>& values, double defaultValue );
    static void replaceMissingValues( std::vector<double>& values, const std::vector<double>& replacementValues );
    cvf::ref<RigResultAccessor> findMissingValuesAccessor( RigEclipseCaseData*                caseData,
                                                           RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                           int                                gridIndex,
                                                           int                                timeStepIndex,
                                                           RimEclipseResultDefinition*        eclipseResultDefinition );

    void addOverburden( std::vector<double>& tvDepthValues,
                        std::vector<double>& measuredDepthValues,
                        std::vector<double>& values ) const;

    void addUnderburden( std::vector<double>& tvDepthValues,
                         std::vector<double>& measuredDepthValues,
                         std::vector<double>& values ) const;

    caf::PdmPtrField<RimFractureModel*>                    m_fractureModel;
    caf::PdmField<caf::AppEnum<MissingValueStrategy>>      m_missingValueStrategy;
    caf::PdmField<caf::AppEnum<BurdenStrategy>>            m_burdenStrategy;
    caf::PdmField<caf::AppEnum<RiaDefines::CurveProperty>> m_curveProperty;
};
