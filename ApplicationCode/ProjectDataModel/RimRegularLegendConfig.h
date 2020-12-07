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
#include "RimLegendConfigChangeType.h"

#include "cvfArray.h"
#include "cvfObject.h"

#include "cafPdmPtrField.h"
#include "cafSignal.h"

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
} // namespace cvf

namespace caf
{
class TitledOverlayFrame;
class CategoryLegend;
class CategoryMapper;
class OverlayScalarMapperLegend;
} // namespace caf

class Rim3dView;
class RimEnsembleCurveSet;
class RiuAbstractLegendFrame;
class RimColorLegend;

//==================================================================================================
///
///
//==================================================================================================
class RimRegularLegendConfig : public RimLegendConfig
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<RimLegendConfigChangeType> changed;

public:
    RimRegularLegendConfig();
    ~RimRegularLegendConfig() override;

    caf::PdmField<QString> resultVariableName; // Used internally to describe the variable this legend setup is used for

    enum class ColorRangesType
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
        RAINBOW,
        STIMPLAN,

        GREEN_RED,
        BLUE_MAGENTA,
        RED_LIGHT_DARK,
        GREEN_LIGHT_DARK,
        BLUE_LIGHT_DARK,

        CORRELATION,

        UNDEFINED
    };

    typedef caf::AppEnum<ColorRangesType> ColorRangeEnum;

    enum class MappingType
    {
        LINEAR_DISCRETE,
        LINEAR_CONTINUOUS,
        LOG10_CONTINUOUS,
        LOG10_DISCRETE,
        CATEGORY_INTEGER
    };
    enum class NumberFormatType
    {
        AUTO,
        SCIENTIFIC,
        FIXED
    };
    typedef caf::AppEnum<MappingType> MappingEnum;

    enum class CategoryColorModeType
    {
        INTERPOLATE,
        EXCLUSIVELY_COLORS
    };
    typedef caf::AppEnum<CategoryColorModeType> CategoryColorModeEnum;

    void onRecreateLegend() override;

    void            setColorLegend( RimColorLegend* colorLegend );
    RimColorLegend* colorLegend() const;
    void            setMappingMode( MappingType mappingType );
    MappingType     mappingMode() { return m_mappingMode(); }
    void            setTickNumberFormat( NumberFormatType numberFormat );
    void            resetUserDefinedValues();

    void disableAllTimeStepsRange( bool doDisable );

    void setAutomaticRanges( double globalMin, double globalMax, double localMin, double localMax );
    void setClosestToZeroValues( double globalPosClosestToZero,
                                 double globalNegClosestToZero,
                                 double localPosClosestToZero,
                                 double localNegClosestToZero );

    void    setIntegerCategories( const std::vector<int>& categories );
    void    setNamedCategories( const std::vector<QString>& categoryNames );
    void    setCategoryItems( const std::vector<std::tuple<QString, int, cvf::Color3ub>>& categories );
    QString categoryNameFromCategoryValue( double categoryResultValue ) const;
    double  categoryValueFromCategoryName( const QString& categoryName ) const;

    void setTitle( const QString& title );

    void setUiValuesFromLegendConfig( const RimRegularLegendConfig* otherLegendConfig );

    cvf::ScalarMapper*       scalarMapper() { return m_currentScalarMapper.p(); }
    const cvf::ScalarMapper* scalarMapper() const { return m_currentScalarMapper.p(); }

    bool showLegend() const;

    const caf::TitledOverlayFrame* titledOverlayFrame() const override;
    caf::TitledOverlayFrame*       titledOverlayFrame() override;
    RiuAbstractLegendFrame*        makeLegendFrame();

    void          setRangeMode( RangeModeType rangeMode );
    RangeModeType rangeMode() const;

    static cvf::Color3ubArray colorArrayFromColorType( ColorRangesType colorType );
    static RimColorLegend*    mapToColorLegend( ColorRangesType colorType );

    static double computeTenExponentCeil( double value );
    static double computeTenExponentFloor( double value );

    void updateFonts() override;

    QString        valueToText( double value ) const;
    static QString valueToText( double value, NumberFormatType numberFormat, int precision );

private:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 sendChangedSignal( const caf::PdmFieldHandle* changedField );
    void                 initAfterRead() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void   updateLegend();
    void   updateFieldVisibility();
    double roundToNumSignificantDigits( double value, double precision );

    void updateCategoryItems();
    void configureCategoryMapper();

    friend class RimViewLinker;

private:
    cvf::ref<cvf::ScalarMapperDiscreteLinear>   m_linDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperDiscreteLog>      m_logDiscreteScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLog>    m_logSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapperContinuousLinear> m_linSmoothScalarMapper;
    cvf::ref<cvf::ScalarMapper>                 m_currentScalarMapper;

    cvf::ref<caf::OverlayScalarMapperLegend> m_scalarMapperLegend;

    cvf::ref<caf::CategoryMapper> m_categoryMapper;
    cvf::ref<caf::CategoryLegend> m_categoryLegend;

    double m_globalAutoMax;
    double m_globalAutoMin;
    double m_localAutoMax;
    double m_localAutoMin;

    double m_globalAutoPosClosestToZero;
    double m_globalAutoNegClosestToZero;
    double m_localAutoPosClosestToZero;
    double m_localAutoNegClosestToZero;

    bool m_isAllTimeStepsRangeDisabled;

    std::vector<int>         m_categories;
    std::vector<cvf::String> m_categoryNames;
    cvf::Color3ubArray       m_categoryColors;

    // Fields
    caf::PdmField<bool>                                m_showLegend;
    caf::PdmField<int>                                 m_numLevels;
    caf::PdmField<int>                                 m_precision;
    caf::PdmField<caf::AppEnum<NumberFormatType>>      m_tickNumberFormat;
    caf::PdmField<RangeModeEnum>                       m_rangeMode;
    caf::PdmField<double>                              m_userDefinedMaxValue;
    caf::PdmField<double>                              m_userDefinedMinValue;
    caf::PdmField<caf::AppEnum<ColorRangesType>>       m_colorRangeMode_OBSOLETE;
    caf::PdmField<caf::AppEnum<MappingType>>           m_mappingMode;
    caf::PdmField<caf::AppEnum<CategoryColorModeType>> m_categoryColorMode;
    caf::PdmField<bool>                                m_resetUserDefinedValuesButton;

    caf::PdmPtrField<RimColorLegend*> m_colorLegend;
    caf::PdmField<bool>               m_selectColorLegendButton;

    bool m_resetUserDefinedValues;

    QString m_title;
    int     m_significantDigitsInData;
};
