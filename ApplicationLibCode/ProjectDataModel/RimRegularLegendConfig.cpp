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

#include "RimRegularLegendConfig.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaNumberFormat.h"
#include "RiaNumericalTools.h"
#include "RiaPreferences.h"

#include "RimCellEdgeColors.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimTools.h"
#include "RimViewLinker.h"
#include "RimWellMeasurementInView.h"
#include "RimWellRftEnsembleCurveSet.h"
#include "RimWellRftPlot.h"

#include "Riu3DMainWindowTools.h"
#include "RiuCategoryLegendFrame.h"
#include "RiuScalarMapperLegendFrame.h"

#include "cafCategoryLegend.h"
#include "cafCategoryMapper.h"
#include "cafOverlayScalarMapperLegend.h"
#include "cafTitledOverlayFrame.h"

#include "cafFactory.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"

#include "cvfMath.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfScalarMapperContinuousLog.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfScalarMapperDiscreteLog.h"
#include "cvfqtUtils.h"

#include <algorithm>
#include <cmath>

using ColorManager = RimEnsembleCurveSetColorManager;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT( RimRegularLegendConfig, "Legend" );

namespace caf
{
template <>
void RimRegularLegendConfig::ColorRangeEnum::setUp()
{
    addItem( RimRegularLegendConfig::ColorRangesType::NORMAL, "NORMAL", "Full color, Red on top" );
    addItem( RimRegularLegendConfig::ColorRangesType::OPPOSITE_NORMAL, "OPPOSITE_NORMAL", "Full color, Blue on top" );
    addItem( RimRegularLegendConfig::ColorRangesType::WHITE_PINK, "WHITE_PIMK", "White to pink" );
    addItem( RimRegularLegendConfig::ColorRangesType::PINK_WHITE, "PINK_WHITE", "Pink to white" );
    addItem( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED, "BLUE_WHITE_RED", "Blue, white, red" );
    addItem( RimRegularLegendConfig::ColorRangesType::RED_WHITE_BLUE, "RED_WHITE_BLUE", "Red, white, blue" );
    addItem( RimRegularLegendConfig::ColorRangesType::WHITE_BLACK, "WHITE_BLACK", "White to black" );
    addItem( RimRegularLegendConfig::ColorRangesType::BLACK_WHITE, "BLACK_WHITE", "Black to white" );
    addItem( RimRegularLegendConfig::ColorRangesType::CATEGORY, "CATEGORY", "Category colors" );
    addItem( RimRegularLegendConfig::ColorRangesType::ANGULAR, "ANGULAR", "Full color cyclic" );
    addItem( RimRegularLegendConfig::ColorRangesType::RAINBOW, "RAINBOW", "Rainbow Palette" );
    addItem( RimRegularLegendConfig::ColorRangesType::STIMPLAN, "STIMPLAN", "StimPlan colors" );
    addItem( RimRegularLegendConfig::ColorRangesType::RED_LIGHT_DARK, "RED_DARK_LIGHT", "Red Light to Dark" );
    addItem( RimRegularLegendConfig::ColorRangesType::GREEN_LIGHT_DARK, "GREEN_DARK_LIGHT", "Green Light to Dark" );
    addItem( RimRegularLegendConfig::ColorRangesType::BLUE_LIGHT_DARK, "BLUE_DARK_LIGHT", "Blue Light to Dark" );
    addItem( RimRegularLegendConfig::ColorRangesType::GREEN_RED, "GREEN_RED", "Green to Red" );
    addItem( RimRegularLegendConfig::ColorRangesType::BLUE_MAGENTA, "BLUE_MAGENTA", "Blue to Magenta" );
    addItem( RimRegularLegendConfig::ColorRangesType::CORRELATION, "CORRELATION", "Correlation colors" );
    addItem( RimRegularLegendConfig::ColorRangesType::HEAT_MAP, "HEAT_MAP", "Heat map colors" );
    addItem( RimRegularLegendConfig::ColorRangesType::UNDEFINED, "UNDEFINED", "Undefined" );
    setDefault( RimRegularLegendConfig::ColorRangesType::UNDEFINED );
}

template <>
void RimRegularLegendConfig::MappingEnum::setUp()
{
    addItem( RimRegularLegendConfig::MappingType::LINEAR_DISCRETE, "LinearDiscrete", "Discrete Linear" );
    addItem( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS, "LinearContinuous", "Continuous Linear" );
    addItem( RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS, "Log10Continuous", "Continuous Logarithmic" );
    addItem( RimRegularLegendConfig::MappingType::LOG10_DISCRETE, "Log10Discrete", "Discrete Logarithmic" );
    addItem( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER, "Category", "Category" );
    setDefault( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
}

template <>
void AppEnum<RimRegularLegendConfig::CategoryColorModeType>::setUp()
{
    addItem( RimRegularLegendConfig::CategoryColorModeType::INTERPOLATE, "INTERPOLATE", "Interpolate" );
    addItem( RimRegularLegendConfig::CategoryColorModeType::EXCLUSIVELY_COLORS, "COLOR_LEGEND_VALUES", "Exclusively Category Colors" );
    setDefault( RimRegularLegendConfig::CategoryColorModeType::INTERPOLATE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::RimRegularLegendConfig()
    : changed( this )
    , m_globalAutoMax( cvf::UNDEFINED_DOUBLE )
    , m_globalAutoMin( cvf::UNDEFINED_DOUBLE )
    , m_localAutoMax( cvf::UNDEFINED_DOUBLE )
    , m_localAutoMin( cvf::UNDEFINED_DOUBLE )
    , m_globalAutoPosClosestToZero( 0 )
    , m_globalAutoNegClosestToZero( 0 )
    , m_localAutoPosClosestToZero( 0 )
    , m_localAutoNegClosestToZero( 0 )
    , m_isAllTimeStepsRangeDisabled( false )
    , m_resetUserDefinedValues( false )
{
    CAF_PDM_InitObject( "Color Legend", ":/Legend.png" );
    CAF_PDM_InitField( &m_showLegend, "ShowLegend", true, "Show Legend" );
    m_showLegend.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_numLevels, "NumberOfLevels", 8, "Number of Levels", "", "A hint on how many tick marks you whish.", "" );
    CAF_PDM_InitField( &m_precision, "Precision", 4, "Significant Digits", "", "The number of significant digits displayed in the legend numbers", "" );
    m_significantDigitsInData = m_precision;
    CAF_PDM_InitField( &m_tickNumberFormat,
                       "TickNumberFormat",
                       caf::AppEnum<RiaNumberFormat::NumberFormatType>( RiaNumberFormat::NumberFormatType::FIXED ),
                       "Number format" );

    CAF_PDM_InitField( &m_colorRangeMode_OBSOLETE, "ColorRangeMode", ColorRangeEnum( ColorRangesType::UNDEFINED ), "Colors" );
    m_colorRangeMode_OBSOLETE.uiCapability()->setUiHidden( true );
    m_colorRangeMode_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_colorLegend, "ColorLegend", "Colors" );
    m_colorLegend = mapToColorLegend( ColorRangeEnum( ColorRangesType::NORMAL ) );
    CAF_PDM_InitField( &m_selectColorLegendButton, "selectColorLegendButton", false, "Edit" );
    m_selectColorLegendButton.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_selectColorLegendButton.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectColorLegendButton.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_mappingMode, "MappingMode", MappingEnum( MappingType::LINEAR_CONTINUOUS ), "Mapping" );
    CAF_PDM_InitField( &m_rangeMode,
                       "RangeType",
                       RangeModeEnum( RangeModeType::AUTOMATIC_ALLTIMESTEPS ),
                       "Range Type",
                       "",
                       "Switches between automatic and user defined range on the legend",
                       "" );
    CAF_PDM_InitField( &m_userDefinedMaxValue, "UserDefinedMax", 1.0, "Max", "", "Max value of the legend", "" );
    CAF_PDM_InitField( &m_userDefinedMinValue,
                       "UserDefinedMin",
                       0.0,
                       "Min",
                       "",
                       "Min value of the legend (if mapping is logarithmic only positive values are valid)",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_categoryColorMode, "CategoryColorMode", "Category Mode" );

    CAF_PDM_InitField( &resultVariableName, "ResultVariableUsage", QString( "" ), "" );
    resultVariableName.uiCapability()->setUiHidden( true );

    m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;
    m_logDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLog;
    m_linSmoothScalarMapper   = new cvf::ScalarMapperContinuousLinear;
    m_logSmoothScalarMapper   = new cvf::ScalarMapperContinuousLog;

    m_currentScalarMapper = m_linDiscreteScalarMapper;

    m_categoryMapper = new caf::CategoryMapper;

    cvf::Font* standardFont = RiaApplication::instance()->defaultSceneFont();
    m_scalarMapperLegend    = new caf::OverlayScalarMapperLegend( standardFont );
    m_categoryLegend        = new caf::CategoryLegend( standardFont, m_categoryMapper.p() );

    CAF_PDM_InitField( &m_resetUserDefinedValuesButton, "ResetDefaultValues", false, "Reset Default Values" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_resetUserDefinedValuesButton );

    CAF_PDM_InitField( &m_centerLegendAroundZero, "CenterLegendAroundZero", false, "Center Legend Around Zero" );

    updateFieldVisibility();
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::~RimRegularLegendConfig()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setNamedCategories( const std::vector<QString>& categoryNames )
{
    std::list<int>         nameIndices;
    std::list<cvf::String> names;

    int categoriesCount = static_cast<int>( categoryNames.size() );
    for ( int i = 0; i < categoriesCount; i++ )
    {
        nameIndices.push_back( i );
        names.push_back( cvfqt::Utils::toString( categoryNames[i] ) );
    }

    m_categories    = std::vector<int>( nameIndices.begin(), nameIndices.end() );
    m_categoryNames = std::vector<cvf::String>( names.begin(), names.end() );
    m_categoryColors.clear();

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    sendChangedSignal( changedField );
    if ( changedField == &m_numLevels )
    {
        int upperLimit = std::numeric_limits<int>::max();
        m_numLevels    = std::clamp( m_numLevels.v(), 1, upperLimit );
    }
    else if ( changedField == &m_rangeMode || changedField == &m_mappingMode )
    {
        if ( m_rangeMode == RangeModeType::USER_DEFINED )
        {
            if ( m_userDefinedMaxValue == m_userDefinedMaxValue.defaultValue() && m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
            {
                m_userDefinedMaxValue = RiaNumericalTools::roundToNumSignificantDigitsCeil( m_globalAutoMax, m_precision );
            }
            if ( m_userDefinedMinValue == m_userDefinedMinValue.defaultValue() && m_globalAutoMin != cvf::UNDEFINED_DOUBLE )
            {
                m_userDefinedMinValue = RiaNumericalTools::roundToNumSignificantDigitsFloor( m_globalAutoMin, m_precision );
            }
        }
        updateFieldVisibility();
    }
    else if ( changedField == &m_selectColorLegendButton )
    {
        m_selectColorLegendButton = false;
        if ( m_colorLegend != nullptr )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( m_colorLegend() );
        }
        return;
    }

    if ( ( changedField == &m_colorLegend || changedField == &m_mappingMode ) && m_mappingMode() == MappingType::CATEGORY_INTEGER )
    {
        updateCategoryItems();
    }

    if ( changedField == &m_resetUserDefinedValuesButton )
    {
        resetUserDefinedValues();

        m_resetUserDefinedValuesButton = false;
    }

    updateLegend();

    auto view = firstAncestorOrThisOfType<RimGridView>();
    if ( view )
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateCellResult();
        }

        view->updateDisplayModelForCurrentTimeStepAndRedraw();

        view->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    // Update stim plan templates if relevant
    auto stimPlanColors = firstAncestorOrThisOfType<RimStimPlanColors>();
    if ( stimPlanColors )
    {
        stimPlanColors->updateStimPlanTemplates();
    }

    // Update ensemble curve set if relevant
    auto ensembleCurveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
    if ( ensembleCurveSet )
    {
        ensembleCurveSet->onLegendDefinitionChanged();
    }

    auto crossPlotCurveSet = firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();
    if ( crossPlotCurveSet )
    {
        if ( changedField != &m_showLegend )
        {
            crossPlotCurveSet->destroyCurves();
            crossPlotCurveSet->destroyRegressionCurves();
        }

        crossPlotCurveSet->loadDataAndUpdate( true );
    }

    auto rftPlot = firstAncestorOrThisOfType<RimWellRftPlot>();
    if ( rftPlot )
    {
        rftPlot->onLegendDefinitionChanged();
    }

    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::sendChangedSignal( const caf::PdmFieldHandle* changedField )
{
    using ChangeType = RimLegendConfigChangeType;

    if ( changedField == &m_showLegend )
    {
        changed.send( ChangeType::VISIBILITY );
    }
    else if ( changedField == &m_numLevels )
    {
        changed.send( ChangeType::LEVELS );
    }
    else if ( changedField == &m_precision || changedField == &m_tickNumberFormat )
    {
        changed.send( ChangeType::NUMBER_FORMAT );
    }
    else if ( changedField == &m_userDefinedMinValue || changedField == &m_userDefinedMaxValue )
    {
        changed.send( ChangeType::RANGE );
    }
    else if ( changedField == &m_rangeMode || changedField == &m_mappingMode )
    {
        changed.send( ChangeType::COLOR_MODE );
    }
    else if ( changedField == &m_categoryColorMode )
    {
        changed.send( ChangeType::COLOR_MODE );
    }
    else if ( changedField == &m_colorLegend )
    {
        changed.send( ChangeType::COLORS );
    }
    else if ( changedField == &m_resetUserDefinedValuesButton )
    {
        changed.send( ChangeType::ALL );
    }
}

auto computeAdjustedMinMax = []( double minimum, double maximum, double precision ) -> std::pair<double, double>
{
    const double threshold = 1e-9;

    if ( std::fabs( maximum - minimum ) < threshold )
    {
        return std::make_pair( minimum, maximum );
    }

    auto adjustedMin = RiaNumericalTools::roundToNumSignificantDigitsFloor( minimum, precision );
    auto adjustedMax = RiaNumericalTools::roundToNumSignificantDigitsCeil( maximum, precision );

    return std::make_pair( adjustedMin, adjustedMax );
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateLegend()
{
    m_significantDigitsInData = m_precision;

    if ( m_resetUserDefinedValues && m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
    {
        updateTickCountAndUserDefinedRange();

        m_resetUserDefinedValues = false;
    }

    double adjustedMin = cvf::UNDEFINED_DOUBLE;
    double adjustedMax = cvf::UNDEFINED_DOUBLE;

    double posClosestToZero = cvf::UNDEFINED_DOUBLE;
    double negClosestToZero = cvf::UNDEFINED_DOUBLE;

    if ( m_rangeMode == RangeModeType::AUTOMATIC_ALLTIMESTEPS )
    {
        std::tie( adjustedMin, adjustedMax ) = computeAdjustedMinMax( m_globalAutoMin, m_globalAutoMax, m_precision );

        posClosestToZero = m_globalAutoPosClosestToZero;
        negClosestToZero = m_globalAutoNegClosestToZero;
    }
    else if ( m_rangeMode == RangeModeType::AUTOMATIC_CURRENT_TIMESTEP )
    {
        std::tie( adjustedMin, adjustedMax ) = computeAdjustedMinMax( m_localAutoMin, m_localAutoMax, m_precision );

        posClosestToZero = m_localAutoPosClosestToZero;
        negClosestToZero = m_localAutoNegClosestToZero;
    }
    else
    {
        std::tie( adjustedMin, adjustedMax ) = computeAdjustedMinMax( m_userDefinedMinValue, m_userDefinedMaxValue, m_precision );

        posClosestToZero = m_globalAutoPosClosestToZero;
        negClosestToZero = m_globalAutoNegClosestToZero;
    }

    if ( m_centerLegendAroundZero )
    {
        auto maxValue = std::max( std::abs( adjustedMax ), std::abs( adjustedMin ) );
        adjustedMax   = maxValue;
        adjustedMin   = -maxValue;
    }

    m_linDiscreteScalarMapper->setRange( adjustedMin, adjustedMax );
    m_linSmoothScalarMapper->setRange( adjustedMin, adjustedMax );

    if ( m_mappingMode == MappingType::LOG10_CONTINUOUS || m_mappingMode == MappingType::LOG10_DISCRETE )
    {
        if ( adjustedMin != adjustedMax )
        {
            if ( adjustedMin == 0 )
            {
                if ( adjustedMax > adjustedMin )
                {
                    adjustedMin = posClosestToZero;
                }
                else
                {
                    adjustedMin = negClosestToZero;
                }
            }
            else if ( adjustedMax == 0 )
            {
                if ( adjustedMin > adjustedMax )
                {
                    adjustedMax = posClosestToZero;
                }
                else
                {
                    adjustedMax = negClosestToZero;
                }
            }
            else if ( adjustedMin < 0 && adjustedMax > 0 )
            {
                adjustedMin = posClosestToZero;
            }
            else if ( adjustedMax < 0 && adjustedMin > 0 )
            {
                adjustedMin = negClosestToZero;
            }
        }
    }

    m_logDiscreteScalarMapper->setRange( adjustedMin, adjustedMax );
    m_logSmoothScalarMapper->setRange( adjustedMin, adjustedMax );

    if ( m_colorLegend() )
    {
        cvf::Color3ubArray legendColors = m_colorLegend()->colorArray();

        m_linDiscreteScalarMapper->setColors( legendColors );
        m_logDiscreteScalarMapper->setColors( legendColors );
        m_logSmoothScalarMapper->setColors( legendColors );
        m_linSmoothScalarMapper->setColors( legendColors );

        m_linDiscreteScalarMapper->setLevelCount( m_numLevels, true );
        m_logDiscreteScalarMapper->setLevelCount( m_numLevels, true );
        m_logSmoothScalarMapper->setLevelCount( m_numLevels, true );
        m_linSmoothScalarMapper->setLevelCount( m_numLevels, true );

        switch ( m_mappingMode() )
        {
            case MappingType::LINEAR_DISCRETE:
                m_currentScalarMapper = m_linDiscreteScalarMapper.p();
                break;
            case MappingType::LINEAR_CONTINUOUS:
                m_currentScalarMapper = m_linSmoothScalarMapper.p();
                break;
            case MappingType::LOG10_CONTINUOUS:
                m_currentScalarMapper = m_logSmoothScalarMapper.p();
                break;
            case MappingType::LOG10_DISCRETE:
                m_currentScalarMapper = m_logDiscreteScalarMapper.p();
                break;
            case MappingType::CATEGORY_INTEGER:
                configureCategoryMapper();
                m_currentScalarMapper = m_categoryMapper.p();
                break;
            default:
                break;
        }
    }

    if ( m_currentScalarMapper != m_categoryMapper.p() )
    {
        m_scalarMapperLegend->setScalarMapper( m_currentScalarMapper.p() );
    }
    double decadesInRange = 0;

    if ( m_mappingMode == MappingType::LOG10_CONTINUOUS || m_mappingMode == MappingType::LOG10_DISCRETE )
    {
        // For log mapping, use the min value as reference for num valid digits
        decadesInRange = cvf::Math::abs( adjustedMin ) < cvf::Math::abs( adjustedMax ) ? cvf::Math::abs( adjustedMin )
                                                                                       : cvf::Math::abs( adjustedMax );
        decadesInRange = log10( decadesInRange );
    }
    else
    {
        // For linear mapping, use the max value as reference for num valid digits
        double absRange = std::max( cvf::Math::abs( adjustedMax ), cvf::Math::abs( adjustedMin ) );
        decadesInRange  = log10( absRange );
    }

    decadesInRange = cvf::Math::ceil( decadesInRange );

    // Using Fixed format
    RiaNumberFormat::NumberFormatType nft = m_tickNumberFormat();
    m_scalarMapperLegend->setTickFormat( (caf::OverlayScalarMapperLegend::NumberFormat)nft );

    // Set the fixed number of digits after the decimal point to the number needed to show all the significant digits.
    int numDecimalDigits = m_precision();
    if ( nft != RiaNumberFormat::NumberFormatType::SCIENTIFIC )
    {
        numDecimalDigits -= static_cast<int>( decadesInRange );
    }
    numDecimalDigits          = std::clamp( numDecimalDigits, 0, 20 );
    m_significantDigitsInData = numDecimalDigits;
    m_scalarMapperLegend->setTickPrecision( numDecimalDigits );

    RiaPreferences* preferences = RiaPreferences::current();
    m_scalarMapperLegend->enableBackground( preferences->showLegendBackground() );
    m_categoryLegend->enableBackground( preferences->showLegendBackground() );

    if ( m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
    {
        m_userDefinedMaxValue.uiCapability()->setUiName( QString( "Max " ) + "(" + QString::number( m_globalAutoMax, 'g', m_precision ) + ")" );
    }
    else
    {
        m_userDefinedMaxValue.uiCapability()->setUiName( QString() );
    }

    if ( m_globalAutoMin != cvf::UNDEFINED_DOUBLE )
    {
        m_userDefinedMinValue.uiCapability()->setUiName( QString( "Min " ) + "(" + QString::number( m_globalAutoMin, 'g', m_precision ) + ")" );
    }
    else
    {
        m_userDefinedMinValue.uiCapability()->setUiName( QString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setTickNumberFormat( RiaNumberFormat::NumberFormatType numberFormat )
{
    m_tickNumberFormat = numberFormat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setUserDefinedRange( double minVal, double maxVal )
{
    m_userDefinedMinValue = minVal;
    m_userDefinedMaxValue = maxVal;
    updateLegend();
    sendChangedSignal( &m_userDefinedMaxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::resetUserDefinedValues()
{
    m_resetUserDefinedValues = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setCenterLegendAroundZero( bool enable )
{
    m_centerLegendAroundZero = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::disableAllTimeStepsRange( bool doDisable )
{
    // If we enable AllTimesteps, and we have used current timestep, then "restore" the default
    if ( m_isAllTimeStepsRangeDisabled && !doDisable && m_rangeMode == RangeModeType::AUTOMATIC_CURRENT_TIMESTEP )
        m_rangeMode = RangeModeType::AUTOMATIC_ALLTIMESTEPS;

    m_isAllTimeStepsRangeDisabled = doDisable;

    if ( doDisable && m_rangeMode == RangeModeType::AUTOMATIC_ALLTIMESTEPS ) m_rangeMode = RangeModeType::AUTOMATIC_CURRENT_TIMESTEP;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setAutomaticRanges( double globalMin, double globalMax, double localMin, double localMax )
{
    std::tie( m_globalAutoMin, m_globalAutoMax ) = computeAdjustedMinMax( globalMin, globalMax, m_precision );
    std::tie( m_localAutoMin, m_localAutoMax )   = computeAdjustedMinMax( localMin, localMax, m_precision );

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::initAfterRead()
{
    if ( resultVariableName() == "Binary Formation Allen" )
    {
        resultVariableName = RiaResultNames::formationBinaryAllanResultName();
    }
    else if ( resultVariableName() == "Formation Allen" )
    {
        resultVariableName = RiaResultNames::formationAllanResultName();
    }

    if ( m_colorRangeMode_OBSOLETE() != RimRegularLegendConfig::ColorRangesType::UNDEFINED )
    {
        m_colorLegend = RimRegularLegendConfig::mapToColorLegend( m_colorRangeMode_OBSOLETE() );
    }

    if ( m_mappingMode() == MappingType::CATEGORY_INTEGER )
    {
        updateCategoryItems();
    }

    updateFieldVisibility();

    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimRegularLegendConfig::objectToggleField()
{
    return &m_showLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_resetUserDefinedValuesButton == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Reset User Defined Values";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateFieldVisibility()
{
    bool showRangeItems = m_mappingMode != MappingType::CATEGORY_INTEGER;

    m_numLevels.uiCapability()->setUiHidden( !showRangeItems );
    m_precision.uiCapability()->setUiHidden( !showRangeItems );
    m_tickNumberFormat.uiCapability()->setUiHidden( !showRangeItems );
    m_rangeMode.uiCapability()->setUiHidden( !showRangeItems );

    if ( showRangeItems && m_rangeMode == RangeModeType::USER_DEFINED )
    {
        m_userDefinedMaxValue.uiCapability()->setUiHidden( false );
        m_userDefinedMinValue.uiCapability()->setUiHidden( false );
    }
    else
    {
        m_userDefinedMaxValue.uiCapability()->setUiHidden( true );
        m_userDefinedMinValue.uiCapability()->setUiHidden( true );
    }

    bool isCategoryMappingMode = ( m_mappingMode == MappingType::CATEGORY_INTEGER );
    m_categoryColorMode.uiCapability()->setUiHidden( !isCategoryMappingMode );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setColorLegend( RimColorLegend* colorLegend )
{
    m_colorLegend = colorLegend;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimRegularLegendConfig::colorLegend() const
{
    return m_colorLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setMappingMode( MappingType mappingType )
{
    m_mappingMode = mappingType;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::onRecreateLegend()
{
    // Due to possible visualization bug, we need to recreate the legend if the last viewer
    // has been removed, (and thus the opengl resources has been deleted) The text in
    // the legend disappeared because of this, so workaround: recreate the legend when needed:

    cvf::Font* font      = RiaApplication::instance()->sceneFont( fontSize() );
    m_scalarMapperLegend = new caf::OverlayScalarMapperLegend( font );
    m_categoryLegend     = new caf::CategoryLegend( font, m_categoryMapper.p() );

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateCategoryItems()
{
    std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
    if ( m_colorLegend() )
    {
        for ( auto item : m_colorLegend->colorLegendItems() )
        {
            cvf::Color3ub ubColor( item->color() );
            categories.push_back( std::make_tuple( item->itemName(), item->categoryValue(), ubColor ) );
        }

        // Reverse the categories to make the ordering identical to items in project tree
        std::reverse( categories.begin(), categories.end() );

        setCategoryItems( categories );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::configureCategoryMapper()
{
    if ( m_categoryColorMode() == CategoryColorModeType::EXCLUSIVELY_COLORS )
    {
        std::vector<RimColorLegendItem*> legendItems = m_colorLegend()->colorLegendItems();
        cvf::Color3ubArray               colorArray;

        if ( !m_categories.empty() ) colorArray.resize( m_categories.size() );

        colorArray.setAll( cvf::Color3ub( RiaColorTables::undefinedCellColor() ) );

        for ( auto value : m_categories )
        {
            for ( auto legendItem : legendItems )
            {
                if ( legendItem->categoryValue() == value )
                {
                    int zeroBasedIndex = std::clamp( value - 1, 0, int( colorArray.size() - 1 ) );
                    colorArray.set( zeroBasedIndex, cvf::Color3ub( legendItem->color() ) );
                }
            }
        }

        m_categoryMapper->setCategoriesValueNameColor( m_categories, m_categoryNames, colorArray );
    }
    else if ( m_categoryColorMode() == CategoryColorModeType::INTERPOLATE )
    {
        m_categoryMapper->setCategoriesWithNames( m_categories, m_categoryNames );

        if ( m_categoryColors.size() > 0 )
        {
            m_categoryMapper->setCycleColors( m_categoryColors );
        }
        else
        {
            cvf::Color3ubArray legendColors = m_colorLegend()->colorArray();

            m_categoryMapper->setInterpolateColors( legendColors );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateTickCountAndUserDefinedRange()
{
    if ( m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
    {
        if ( m_mappingMode() == MappingType::LOG10_CONTINUOUS || m_mappingMode() == MappingType::LOG10_DISCRETE )
        {
            double exponentMax = RiaNumericalTools::computeTenExponentCeil( m_globalAutoMax );
            double exponentMin = RiaNumericalTools::computeTenExponentFloor( m_globalAutoPosClosestToZero );

            m_userDefinedMaxValue = pow( 10, exponentMax );
            m_userDefinedMinValue = pow( 10, exponentMin );

            int numLevels = exponentMax - exponentMin;
            if ( numLevels > 0 )
            {
                m_numLevels = numLevels;
            }
        }
        else if ( m_mappingMode() == MappingType::LINEAR_CONTINUOUS || m_mappingMode() == MappingType::LINEAR_DISCRETE )
        {
            m_userDefinedMaxValue = m_globalAutoMax;
            m_userDefinedMinValue = m_globalAutoMin;
            m_numLevels           = 8;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setClosestToZeroValues( double globalPosClosestToZero,
                                                     double globalNegClosestToZero,
                                                     double localPosClosestToZero,
                                                     double localNegClosestToZero )
{
    bool         needsUpdate = false;
    const double epsilon     = std::numeric_limits<double>::epsilon();

    if ( cvf::Math::abs( globalPosClosestToZero - m_globalAutoPosClosestToZero ) > epsilon )
    {
        needsUpdate = true;
    }
    if ( cvf::Math::abs( globalNegClosestToZero - m_globalAutoNegClosestToZero ) > epsilon )
    {
        needsUpdate = true;
    }
    if ( cvf::Math::abs( localPosClosestToZero - m_localAutoPosClosestToZero ) > epsilon )
    {
        needsUpdate = true;
    }
    if ( cvf::Math::abs( localNegClosestToZero - m_localAutoNegClosestToZero ) > epsilon )
    {
        needsUpdate = true;
    }

    if ( needsUpdate )
    {
        m_globalAutoPosClosestToZero = globalPosClosestToZero;
        m_globalAutoNegClosestToZero = globalNegClosestToZero;
        m_localAutoPosClosestToZero  = localPosClosestToZero;
        m_localAutoNegClosestToZero  = localNegClosestToZero;

        if ( m_globalAutoPosClosestToZero == HUGE_VAL ) m_globalAutoPosClosestToZero = 0;
        if ( m_globalAutoNegClosestToZero == -HUGE_VAL ) m_globalAutoNegClosestToZero = 0;
        if ( m_localAutoPosClosestToZero == HUGE_VAL ) m_localAutoPosClosestToZero = 0;
        if ( m_localAutoNegClosestToZero == -HUGE_VAL ) m_localAutoNegClosestToZero = 0;

        updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setIntegerCategories( const std::vector<int>& categories )
{
    m_categories = categories;
    m_categoryNames.clear();
    m_categoryColors.clear();

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setCategoryItems( const std::vector<std::tuple<QString, int, cvf::Color3ub>>& categories )
{
    m_categories.clear();
    m_categoryNames.clear();
    m_categoryColors.clear();
    m_categoryColors.reserve( categories.size() );

    for ( auto item : categories )
    {
        m_categoryNames.push_back( cvfqt::Utils::toString( std::get<0>( item ) ) );
        m_categories.push_back( std::get<1>( item ) );
        m_categoryColors.add( std::get<2>( item ) );
    }

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRegularLegendConfig::categoryNameFromCategoryValue( double categoryResultValue ) const
{
    if ( categoryResultValue == HUGE_VAL ) return "Undefined";

    if ( !m_categoryNames.empty() )
    {
        for ( size_t categoryIndex = 0; categoryIndex < m_categories.size(); categoryIndex++ )
        {
            if ( categoryResultValue == m_categories[categoryIndex] )
            {
                return cvfqt::Utils::toQString( m_categoryNames[categoryIndex] );
            }
        }
    }

    return QString( "%1" ).arg( categoryResultValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimRegularLegendConfig::categoryValueFromCategoryName( const QString& categoryName ) const
{
    for ( int i = 0; i < (int)m_categoryNames.size(); i++ )
    {
        if ( cvfqt::Utils::toQString( m_categoryNames[i] ).compare( categoryName, Qt::CaseInsensitive ) == 0 )
        {
            return i;
        }
    }
    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setTitle( const QString& title )
{
    m_title = title;

    auto cvfTitle = cvfqt::Utils::toString( m_title );
    m_scalarMapperLegend->setTitle( cvfTitle );
    m_categoryLegend->setTitle( cvfTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularLegendConfig::showLegend() const
{
    return m_showLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setShowLegend( bool show )
{
    m_showLegend = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::TitledOverlayFrame* RimRegularLegendConfig::titledOverlayFrame()
{
    if ( m_currentScalarMapper == m_categoryMapper )
    {
        return m_categoryLegend.p();
    }
    else
    {
        return m_scalarMapperLegend.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::TitledOverlayFrame* RimRegularLegendConfig::titledOverlayFrame() const
{
    if ( m_currentScalarMapper == m_categoryMapper )
    {
        return m_categoryLegend.p();
    }
    else
    {
        return m_scalarMapperLegend.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuAbstractLegendFrame* RimRegularLegendConfig::makeLegendFrame()
{
    if ( m_currentScalarMapper == m_categoryMapper )
    {
        return new RiuCategoryLegendFrame( nullptr, m_title, m_categoryMapper.p() );
    }
    else
    {
        auto legend = new RiuScalarMapperLegendFrame( nullptr, m_title, m_currentScalarMapper.p() );
        legend->setTickFormat( m_tickNumberFormat() );
        legend->setTickPrecision( m_significantDigitsInData );
        return legend;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setRangeMode( RangeModeType rangeMode )
{
    m_rangeMode = rangeMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimLegendConfig::RangeModeType RimRegularLegendConfig::rangeMode() const
{
    return m_rangeMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setUiValuesFromLegendConfig( const RimRegularLegendConfig* otherLegendConfig )
{
    QString serializedObjectString = otherLegendConfig->writeObjectToXmlString();
    readObjectFromXmlString( serializedObjectString, caf::PdmDefaultObjectFactory::instance() );
    resolveReferencesRecursively();
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray RimRegularLegendConfig::colorArrayFromColorType( ColorRangesType colorType )
{
    switch ( colorType )
    {
        case RimRegularLegendConfig::ColorRangesType::NORMAL:
            return RiaColorTables::normalPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::OPPOSITE_NORMAL:
            return RiaColorTables::normalPaletteOppositeOrderingColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::WHITE_PINK:
            return RiaColorTables::whitePinkPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::PINK_WHITE:
            return RiaColorTables::pinkWhitePaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::WHITE_BLACK:
            return RiaColorTables::whiteBlackPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::BLACK_WHITE:
            return RiaColorTables::blackWhitePaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED:
            return RiaColorTables::blueWhiteRedPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::RED_WHITE_BLUE:
            return RiaColorTables::redWhiteBluePaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::CATEGORY:
            return RiaColorTables::categoryPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::ANGULAR:
            return RiaColorTables::angularPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::RAINBOW:
            return RiaColorTables::rainbowPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::STIMPLAN:
            return RiaColorTables::stimPlanPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::CORRELATION:
            return RiaColorTables::correlationPaletteColors().color3ubArray();
            break;
        case RimRegularLegendConfig::ColorRangesType::HEAT_MAP:
            return RiaColorTables::heatMapPaletteColors().color3ubArray();
            break;
        default:
            if ( ColorManager::isEnsembleColorRange( colorType ) ) return ColorManager::EnsembleColorRanges().at( colorType );
            break;
    }
    return RiaColorTables::normalPaletteColors().color3ubArray();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimRegularLegendConfig::mapToColorLegend( ColorRangesType colorType )
{
    RimProject* project = RimProject::current();
    return project->colorLegendCollection()->findByName( RimRegularLegendConfig::ColorRangeEnum::uiText( colorType ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateFonts()
{
    int  pointSize = fontSize();
    auto font      = RiaApplication::instance()->sceneFont( pointSize );

    m_scalarMapperLegend = new caf::OverlayScalarMapperLegend( font );
    m_categoryLegend     = new caf::CategoryLegend( font, m_categoryMapper.p() );
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRegularLegendConfig::valueToText( double value ) const
{
    return RiaNumberFormat::valueToText( value, m_tickNumberFormat(), m_significantDigitsInData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNumberFormat::NumberFormatType RimRegularLegendConfig::tickNumberFormat() const
{
    return m_tickNumberFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimRegularLegendConfig::significantDigitsInData() const
{
    return m_significantDigitsInData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setDefaultConfigForResultName( int caseId, const QString& resultName, bool useDiscreteLogLevels, bool isCategoryResult )
{
    bool useLog = RiaResultNames::isLogarithmicResult( resultName );

    RimRegularLegendConfig::MappingType mappingType  = MappingType::LINEAR_CONTINUOUS;
    RimLegendConfig::RangeModeType      rangeType    = RimLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS;
    RiaNumberFormat::NumberFormatType   numberFormat = RiaNumberFormat::NumberFormatType::FIXED;

    if ( useLog )
    {
        if ( useDiscreteLogLevels )
            mappingType = RimRegularLegendConfig::MappingType::LOG10_DISCRETE;
        else
            mappingType = RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS;

        numberFormat = RiaNumberFormat::NumberFormatType::AUTO;
        rangeType    = RimLegendConfig::RangeModeType::USER_DEFINED;
    }

    bool                                    centerLegendAroundZero = false;
    RimRegularLegendConfig::ColorRangesType colorRangeType         = RimRegularLegendConfig::ColorRangesType::UNDEFINED;

    if ( isCategoryResult )
    {
        colorRangeType = RimRegularLegendConfig::ColorRangesType::CATEGORY;
        mappingType    = RimRegularLegendConfig::MappingType::CATEGORY_INTEGER;
    }
    else if ( resultName == RiaResultNames::swat() )
    {
        colorRangeType = RimRegularLegendConfig::ColorRangesType::OPPOSITE_NORMAL;
    }
    else if ( RiaResultNames::isFlowResultWithBothPosAndNegValues( resultName ) )
    {
        colorRangeType         = RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED;
        centerLegendAroundZero = true;
    }
    else if ( resultName != RiaResultNames::undefinedResultName() )
    {
        colorRangeType = RimRegularLegendConfig::ColorRangesType::NORMAL;
    }

    resetUserDefinedValues();
    setRangeMode( rangeType );
    setMappingMode( mappingType );
    setCenterLegendAroundZero( centerLegendAroundZero );
    setTickNumberFormat( numberFormat );
    updateTickCountAndUserDefinedRange();

    RimProject* project       = RimProject::current();
    auto        defaultLegend = project->colorLegendCollection()->findDefaultLegendForResult( caseId, resultName );
    if ( defaultLegend )
    {
        setColorLegend( defaultLegend );
    }
    else if ( colorRangeType != RimRegularLegendConfig::ColorRangesType::UNDEFINED )
    {
        RimColorLegend* colorLegend = RimRegularLegendConfig::mapToColorLegend( colorRangeType );

        setColorLegend( colorLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == "NumLevelsOnly" )
    {
        uiOrdering.add( &m_numLevels );
        uiOrdering.skipRemainingFields( true );
    }
    else if ( uiConfigName == "NumIntervalsOnly" )
    {
        m_numLevels.uiCapability()->setUiName( "Number of Intervals" );
        uiOrdering.add( &m_numLevels );
        uiOrdering.skipRemainingFields( true );
    }
    else if ( uiConfigName == "ColorsOnly" )
    {
        uiOrdering.add( &m_colorLegend );
        uiOrdering.skipRemainingFields( true );
    }
    else if ( uiConfigName == "FlagAndColorsOnly" )
    {
        uiOrdering.add( &m_showLegend );
        uiOrdering.add( &m_colorLegend );
        uiOrdering.skipRemainingFields( true );
    }
    else if ( uiConfigName == "UserDefinedMinMaxOnly" )
    {
        uiOrdering.add( &m_userDefinedMaxValue );
        uiOrdering.add( &m_userDefinedMinValue );
        uiOrdering.skipRemainingFields( true );
    }
    else if ( uiConfigName == "RangeModeAndUserDefinedMinMaxOnly" )
    {
        // TODO: DELETE!!!
        uiOrdering.add( &m_rangeMode );
        uiOrdering.add( &m_userDefinedMaxValue );
        uiOrdering.add( &m_userDefinedMinValue );
        uiOrdering.skipRemainingFields( true );
    }
    else
    {
        caf::PdmUiOrdering* formatGr = uiOrdering.addNewGroup( "Format" );
        formatGr->add( &m_numLevels );
        formatGr->add( &m_precision );
        formatGr->add( &m_tickNumberFormat );

        formatGr->add( &m_colorLegend, { .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        formatGr->add( &m_selectColorLegendButton, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );

        caf::PdmUiOrdering* mappingGr = uiOrdering.addNewGroup( "Mapping" );
        mappingGr->add( &m_mappingMode );
        mappingGr->add( &m_rangeMode );
        mappingGr->add( &m_userDefinedMaxValue );
        mappingGr->add( &m_userDefinedMinValue );
        mappingGr->add( &m_categoryColorMode );
        mappingGr->add( &m_centerLegendAroundZero );

        uiOrdering.add( &m_resetUserDefinedValuesButton );
    }

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::defineUiOrderingColorOnly( caf::PdmUiOrdering* colorGroup )
{
    colorGroup->add( &m_colorLegend, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    colorGroup->add( &m_selectColorLegendButton, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRegularLegendConfig::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    bool hasStimPlanParent         = false;
    bool hasEnsembleCurveSetParent = false;
    bool hasRftPlotParent          = false;

    auto stimPlanColors = firstAncestorOrThisOfType<RimStimPlanColors>();
    if ( stimPlanColors ) hasStimPlanParent = true;

    auto ensembleCurveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
    if ( ensembleCurveSet ) hasEnsembleCurveSetParent = true;

    auto crossPlotCurveSet = firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();

    auto rftCurveSet = firstAncestorOrThisOfType<RimWellRftEnsembleCurveSet>();
    if ( rftCurveSet ) hasRftPlotParent = true;

    bool isAllanDiagram = false;
    {
        auto eclCellColors = firstAncestorOrThisOfType<RimEclipseCellColors>();
        if ( eclCellColors && eclCellColors->resultType() == RiaDefines::ResultCatType::ALLAN_DIAGRAMS )
        {
            isAllanDiagram = true;
        }
    }

    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_mappingMode )
    {
        // This is an app enum field, see cafInternalPdmFieldTypeSpecializations.h for the default specialization of
        // this type
        std::vector<MappingType> mappingTypes;
        if ( !isAllanDiagram )
        {
            mappingTypes.push_back( MappingType::LINEAR_DISCRETE );

            if ( !crossPlotCurveSet )
            {
                mappingTypes.push_back( MappingType::LINEAR_CONTINUOUS );
                mappingTypes.push_back( MappingType::LOG10_CONTINUOUS );
            }
            mappingTypes.push_back( MappingType::LOG10_DISCRETE );
        }

        mappingTypes.push_back( MappingType::CATEGORY_INTEGER );

        for ( MappingType mapType : mappingTypes )
        {
            options.push_back( caf::PdmOptionItemInfo( MappingEnum::uiText( mapType ), mapType ) );
        }
    }
    else if ( fieldNeedingOptions == &m_colorLegend )
    {
        RimTools::colorLegendOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_rangeMode )
    {
        if ( !m_isAllTimeStepsRangeDisabled )
        {
            QString uiText;
            if ( !hasEnsembleCurveSetParent )
                uiText = RangeModeEnum::uiText( RimRegularLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS );
            else
                uiText = "Auto Range";

            options.push_back( caf::PdmOptionItemInfo( uiText, RimRegularLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS ) );
        }
        if ( !hasStimPlanParent && !hasEnsembleCurveSetParent && !hasRftPlotParent )
        {
            options.push_back( caf::PdmOptionItemInfo( RangeModeEnum::uiText( RimRegularLegendConfig::RangeModeType::AUTOMATIC_CURRENT_TIMESTEP ),
                                                       RimRegularLegendConfig::RangeModeType::AUTOMATIC_CURRENT_TIMESTEP ) );
        }
        options.push_back( caf::PdmOptionItemInfo( RangeModeEnum::uiText( RimRegularLegendConfig::RangeModeType::USER_DEFINED ),
                                                   RimRegularLegendConfig::RangeModeType::USER_DEFINED ) );
    }

    return options;
}
