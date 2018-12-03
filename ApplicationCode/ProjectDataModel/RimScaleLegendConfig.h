/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimLegendConfig.h" 

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"

#include <tuple>

namespace cvf
{
    class ScalarMapperContinuousLog;
    class ScalarMapperContinuousLinear;
    class OverlayItem;
    class ScalarMapperDiscreteLinear;
    class ScalarMapperDiscreteLog;
    class ScalarMapper;
    class String;
}

namespace caf
{
    class TitledOverlayFrame;
    class CategoryLegend;
    class CategoryMapper;
    class OverlayScaleLegend;
}

class Rim3dView;
class RimEnsembleCurveSet;

//==================================================================================================
///  
///  
//==================================================================================================
class RimScaleLegendConfig : public RimLegendConfig
{
    CAF_PDM_HEADER_INIT;
public:
    RimScaleLegendConfig();
    ~RimScaleLegendConfig() override;

    caf::PdmField<QString>                      resultVariableName; // Used internally to describe the variable this legend setup is used for

    enum ColorRangesType
    {
        NORMAL,
        OPPOSITE_NORMAL,
        WHITE_PINK,
        PINK_WHITE,
        WHITE_BLACK,
        BLACK_WHITE,
        BLUE_WHITE_RED,
        RED_WHITE_BLUE,
        CATEGORY,
        ANGULAR,
        STIMPLAN,

        GREEN_RED,
        BLUE_MAGENTA,
        RED_LIGHT_DARK,
        GREEN_LIGHT_DARK,
        BLUE_LIGHT_DARK
    };

    typedef caf::AppEnum<ColorRangesType> ColorRangeEnum;

    void                                        recreateLegend();

    void                                        setColorRange(ColorRangesType colorMode);
    ColorRangesType                             colorRange()    { return m_colorRangeMode();}
    void                                        disableAllTimeStepsRange(bool doDisable);
        
    void                                        setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax);
    
    void                                        setTitle(const QString& title);

    void                                        setUiValuesFromLegendConfig(const RimScaleLegendConfig* otherLegendConfig);

    bool                                        showLegend() const;
    void                                        setShowLegend(bool show);

    const caf::TitledOverlayFrame* titledOverlayFrame() const override;
    caf::TitledOverlayFrame* titledOverlayFrame() override;

    const cvf::OverlayItem*                     overlayItem() const;
    cvf::OverlayItem*                           overlayItem();

    RangeModeType                               rangeMode() const;

    void                                        setCurrentScale(double scale);

private:
    void                                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                        initAfterRead() override;
    caf::PdmFieldHandle*                        objectToggleField() override;

    void                                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo>               calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

    void                                        updateLegend();
    void                                        updateFieldVisibility();
    double                                      roundToNumSignificantDigits(double value, double precision);

    friend class RimViewLinker;
    
    static cvf::Color3ubArray                   colorArrayFromColorType(ColorRangesType colorType);

private:
    cvf::ref<caf::OverlayScaleLegend>           m_scaleLegend;
    
    double                                      m_globalAutoMax;
    double                                      m_globalAutoMin;
    double                                      m_localAutoMax;
    double                                      m_localAutoMin;

    bool                                        m_isAllTimeStepsRangeDisabled;
    
    // Fields
    caf::PdmField<bool>                         m_showLegend;
    caf::PdmField<int>                          m_numLevels;
    caf::PdmField<int>                          m_precision;
    caf::PdmField<RangeModeEnum>                m_rangeMode;
    caf::PdmField<double>                       m_userDefinedMaxValue;
    caf::PdmField<double>                       m_userDefinedMinValue;
    caf::PdmField<caf::AppEnum<ColorRangesType> > m_colorRangeMode;

    double                                      m_currentScale; // [meters/pixel]
};
