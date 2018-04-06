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
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"

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
    class OverlayScalarMapperLegend;
}

class Rim3dView;

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

    caf::PdmField<QString>                      resultVariableName; // Used internally to describe the variable this legend setup is used for

    enum RangeModeType
    {
        AUTOMATIC_ALLTIMESTEPS,
        AUTOMATIC_CURRENT_TIMESTEP,
        USER_DEFINED
    };
    
    typedef caf::AppEnum<RangeModeType> RangeModeEnum;

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
        STIMPLAN
    };

    typedef caf::AppEnum<ColorRangesType> ColorRangeEnum;

    enum MappingType
    {
        LINEAR_DISCRETE,
        LINEAR_CONTINUOUS,
        LOG10_CONTINUOUS,
        LOG10_DISCRETE,
        CATEGORY_INTEGER
    };
    enum NumberFormatType { AUTO, SCIENTIFIC, FIXED};

    typedef caf::AppEnum<MappingType> MappingEnum;
    void                                        recreateLegend();

    void                                        setColorRangeMode(ColorRangesType colorMode);
    ColorRangesType                             colorRangeMode()    { return m_colorRangeMode();}
    void                                        setMappingMode(MappingType mappingType);
    MappingType                                 mappingMode()       { return m_mappingMode();}
    void                                        disableAllTimeStepsRange(bool doDisable);
        
    void                                        setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax);
    void                                        setClosestToZeroValues(double globalPosClosestToZero, double globalNegClosestToZero, double localPosClosestToZero, double localNegClosestToZero);
    
    void                                        setIntegerCategories(const std::vector<int>& categories);
    void                                        setNamedCategoriesInverse(const std::vector<QString>& categoryNames);
    void                                        setCategoryItems(const std::vector<std::tuple<QString, int, cvf::Color3ub>>& categories);
    QString                                     categoryNameFromCategoryValue(double categoryResultValue) const;

    void                                        setTitle(const QString& title);

    void                                        setUiValuesFromLegendConfig(const RimLegendConfig* otherLegendConfig);

    cvf::ScalarMapper*                          scalarMapper() { return m_currentScalarMapper.p(); }
    caf::TitledOverlayFrame*                    legend();

    caf::PdmField<bool>                         enableLegend;

protected:
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                initAfterRead();
    virtual caf::PdmFieldHandle*                objectToggleField();

    friend class RimStimPlanLegendConfig;
    virtual void                                defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    virtual QList<caf::PdmOptionItemInfo>       calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);

private:
    void                                        updateLegend();
    void                                        updateFieldVisibility();
    double                                      roundToNumSignificantDigits(double value, double precision);

    friend class RimViewLinker;
    
    static cvf::Color3ubArray                   colorArrayFromColorType(ColorRangesType colorType);

private:
    cvf::ref<cvf::ScalarMapperDiscreteLinear>   m_linDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperDiscreteLog>      m_logDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLog>    m_logSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLinear> m_linSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapper>                 m_currentScalarMapper;

    cvf::ref<caf::OverlayScalarMapperLegend>    m_scalarMapperLegend;
    
    cvf::ref<caf::CategoryMapper>               m_categoryMapper;
    cvf::ref<caf::CategoryLegend>               m_categoryLegend;

    double                                      m_globalAutoMax;
    double                                      m_globalAutoMin;
    double                                      m_localAutoMax;
    double                                      m_localAutoMin;

    double                                      m_globalAutoPosClosestToZero;
    double                                      m_globalAutoNegClosestToZero;
    double                                      m_localAutoPosClosestToZero;
    double                                      m_localAutoNegClosestToZero;

    bool                                        m_isAllTimeStepsRangeDisabled;
    
    std::vector<int>                            m_categories;
    std::vector<cvf::String>                    m_categoryNames;
    cvf::Color3ubArray                          m_categoryColors;

    // Fields
    caf::PdmField<int>                          m_numLevels;
    caf::PdmField<int>                          m_precision;
    caf::PdmField<caf::AppEnum<NumberFormatType> > m_tickNumberFormat;
    caf::PdmField<RangeModeEnum>                m_rangeMode;
    caf::PdmField<double>                       m_userDefinedMaxValue;
    caf::PdmField<double>                       m_userDefinedMinValue;
    caf::PdmField<caf::AppEnum<ColorRangesType> > m_colorRangeMode;
    caf::PdmField<caf::AppEnum<MappingType> >    m_mappingMode;
};
