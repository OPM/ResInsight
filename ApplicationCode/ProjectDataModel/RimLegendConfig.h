/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cvfVector2.h"
#include "cvfArray.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

namespace cvf
{
    class ScalarMapperContinuousLog;
    class ScalarMapperContinuousLinear;
    class OverlayScalarMapperLegend;
    class ScalarMapperDiscreteLinear;
    class ScalarMapperDiscreteLog;
    class ScalarMapper;
}

class RimReservoirView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimLegendConfig:  public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimLegendConfig();
    virtual ~RimLegendConfig();

    void setReservoirView(RimReservoirView* ownerReservoirView) {m_reservoirView = ownerReservoirView; }

    caf::PdmField<QString>                      resultVariableName; // Used internally to describe the variable this legend setup is used for

    enum RangeModeType 
    {
        AUTOMATIC_ALLTIMESTEPS,
        AUTOMATIC_CURRENT_TIMESTEP,
        USER_DEFINED
    };

    enum ColorRangesType
    {
        NORMAL,
        OPPOSITE_NORMAL,
        WHITE_PINK,
        PINK_WHITE,
        WHITE_BLACK,
        BLACK_WHITE
    };

    typedef caf::AppEnum<ColorRangesType> ColorRangeEnum;

    enum MappingType
    {
        LINEAR_DISCRETE,
        LINEAR_CONTINUOUS,
        LOG10_CONTINUOUS,
        LOG10_DISCRETE
    };
    enum NumberFormatType { AUTO, SCIENTIFIC, FIXED};

    typedef caf::AppEnum<MappingType> MappingEnum;
    void                                        recreateLegend();
    void                                        setColorRangeMode(ColorRangesType colorMode);
    void                                        setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax);
    void                                        setPosition(cvf::Vec2ui position);

    cvf::ScalarMapper*                          scalarMapper() { return m_currentScalarMapper.p(); }
    cvf::OverlayScalarMapperLegend*                    legend() { return m_legend.p(); }
    void                                        updateLegend();

protected:
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                initAfterRead();
private:
    void                                        updateFieldVisibility();
    cvf::ref<cvf::Color3ubArray>                interpolateColorArray(const cvf::Color3ubArray& colorArray, cvf::uint targetColorCount);
    double                                      adjust(double value, double precision);
    
private:
    caf::PdmPointer<RimReservoirView>           m_reservoirView;

    cvf::ref<cvf::ScalarMapperDiscreteLinear>   m_linDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperDiscreteLog>      m_logDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLog>    m_logSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLinear> m_linSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapper>                 m_currentScalarMapper;

    cvf::ref<cvf::OverlayScalarMapperLegend>           m_legend;

    double                                      m_globalAutoMax;
    double                                      m_globalAutoMin;
    double                                      m_localAutoMax;
    double                                      m_localAutoMin;

    cvf::Vec2ui                                 m_position;

    // Fields
    caf::PdmField<int>                          m_numLevels;
    caf::PdmField<int>                          m_precision;
    caf::PdmField<caf::AppEnum<NumberFormatType> > m_tickNumberFormat;
    caf::PdmField<caf::AppEnum<RangeModeType> > m_rangeMode;
    caf::PdmField<double>                       m_userDefinedMaxValue;
    caf::PdmField<double>                       m_userDefinedMinValue;
    caf::PdmField<caf::AppEnum<ColorRangesType> > m_colorRangeMode;
    caf::PdmField<caf::AppEnum<MappingType> >    m_mappingMode;

};
