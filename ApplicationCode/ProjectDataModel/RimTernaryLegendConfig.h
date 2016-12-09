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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimEclipseView;
class RivTernarySaturationOverlayItem;
class RivTernaryScalarMapper;

namespace cvf
{
    class OverlayItem;
}



//==================================================================================================
///  
///  
//==================================================================================================
class RimTernaryLegendConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum TernaryArrayIndex
    {
        TERNARY_SOIL_IDX = 0,
        TERNARY_SGAS_IDX,
        TERNARY_SWAT_IDX
    };

    enum RangeModeType
    {
        AUTOMATIC_ALLTIMESTEPS,
        AUTOMATIC_CURRENT_TIMESTEP,
        USER_DEFINED
    };
    typedef caf::AppEnum<RangeModeType> RangeModeEnum;

public:
    RimTernaryLegendConfig();
    virtual ~RimTernaryLegendConfig();

    void                setAutomaticRanges(TernaryArrayIndex ternaryIndex, double globalMin, double globalMax, double localMin, double localMax);
    void                ternaryRanges(double& soilLower, double& soilUpper, double& sgasLower, double& sgasUpper, double& swatLower, double& swatUpper) const;

    void                recreateLegend();
    
    RivTernarySaturationOverlayItem*    legend();
    RivTernaryScalarMapper*             scalarMapper();

protected:
    virtual void        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    virtual void        defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    void                updateLegend();
    void                updateLabelText();
    double              roundToNumSignificantDigits(double value, double precision);
    
    friend class RimViewLinker;
    void                setUiValuesFromLegendConfig(const RimTernaryLegendConfig* otherLegendConfig);

private:
    caf::PdmField<int>              precision;
    caf::PdmField<RangeModeEnum>    rangeMode;

    caf::PdmField<double>           userDefinedMaxValueSoil;
    caf::PdmField<double>           userDefinedMinValueSoil;
    caf::PdmField<double>           userDefinedMaxValueSgas;
    caf::PdmField<double>           userDefinedMinValueSgas;
    caf::PdmField<double>           userDefinedMaxValueSwat;
    caf::PdmField<double>           userDefinedMinValueSwat;

    caf::PdmField<bool>             applyLocalMinMax;
    caf::PdmField<bool>             applyGlobalMinMax;
    caf::PdmField<bool>             applyFullRangeMinMax;
    caf::PdmField<QString>          ternaryRangeSummary;

    std::vector<double>             m_globalAutoMax;
    std::vector<double>             m_globalAutoMin;
    std::vector<double>             m_localAutoMax;
    std::vector<double>             m_localAutoMin;

    cvf::ref<RivTernarySaturationOverlayItem>   m_legend;
    cvf::ref<RivTernaryScalarMapper>            m_scalarMapper;
};
