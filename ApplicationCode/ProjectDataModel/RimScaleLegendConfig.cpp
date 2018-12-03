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

#include "RimScaleLegendConfig.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimEclipseView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimIntersectionCollection.h"
#include "RimStimPlanColors.h"
#include "RimViewLinker.h"

#include "cafTitledOverlayFrame.h"
#include "cafCategoryLegend.h"
#include "cafCategoryMapper.h"
#include "cafOverlayScaleLegend.h"

#include "cafFactory.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include "cvfScalarMapperContinuousLinear.h"
#include "cvfScalarMapperContinuousLog.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfScalarMapperDiscreteLog.h"
#include "cvfqtUtils.h"

#include <cmath>
#include <algorithm>

using ColorManager = RimEnsembleCurveSetColorManager;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT(RimScaleLegendConfig, "ScaleLegend");


namespace caf {
    template<>
    void RimScaleLegendConfig::ColorRangeEnum::setUp()
    {
        addItem(RimScaleLegendConfig::NORMAL,             "NORMAL",               "Full color, Red on top");
        addItem(RimScaleLegendConfig::OPPOSITE_NORMAL,    "OPPOSITE_NORMAL",      "Full color, Blue on top");
        addItem(RimScaleLegendConfig::WHITE_PINK,         "WHITE_PIMK",           "White to pink");
        addItem(RimScaleLegendConfig::PINK_WHITE,         "PINK_WHITE",           "Pink to white");
        addItem(RimScaleLegendConfig::BLUE_WHITE_RED,     "BLUE_WHITE_RED",       "Blue, white, red");
        addItem(RimScaleLegendConfig::RED_WHITE_BLUE,     "RED_WHITE_BLUE",       "Red, white, blue");
        addItem(RimScaleLegendConfig::WHITE_BLACK,        "WHITE_BLACK",          "White to black");
        addItem(RimScaleLegendConfig::BLACK_WHITE,        "BLACK_WHITE",          "Black to white");
        addItem(RimScaleLegendConfig::CATEGORY,           "CATEGORY",             "Category colors");
        addItem(RimScaleLegendConfig::ANGULAR,            "ANGULAR",              "Full color cyclic");
        addItem(RimScaleLegendConfig::STIMPLAN,           "STIMPLAN",             "StimPlan colors");
        addItem(RimScaleLegendConfig::RED_LIGHT_DARK,     "RED_DARK_LIGHT",       "Red Light to Dark");
        addItem(RimScaleLegendConfig::GREEN_LIGHT_DARK,   "GREEN_DARK_LIGHT",     "Green Light to Dark");
        addItem(RimScaleLegendConfig::BLUE_LIGHT_DARK,    "BLUE_DARK_LIGHT",      "Blue Light to Dark");
        addItem(RimScaleLegendConfig::GREEN_RED,          "GREEN_RED",            "Green to Red");
        addItem(RimScaleLegendConfig::BLUE_MAGENTA,       "BLUE_MAGENTA",         "Blue to Magenta");
        setDefault(RimScaleLegendConfig::NORMAL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimScaleLegendConfig::RimScaleLegendConfig() 
    :   m_globalAutoMax(cvf::UNDEFINED_DOUBLE),
        m_globalAutoMin(cvf::UNDEFINED_DOUBLE),
        m_localAutoMax(cvf::UNDEFINED_DOUBLE),
        m_localAutoMin(cvf::UNDEFINED_DOUBLE),
        m_isAllTimeStepsRangeDisabled(false)
{
    CAF_PDM_InitObject("Legend Definition", ":/Legend.png", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Show Legend", "", "", "");    
    m_showLegend.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_numLevels, "NumberOfLevels", 8, "Number of Levels", "", "A hint on how many tick marks you whish.","");
    CAF_PDM_InitField(&m_precision, "Precision", 4, "Significant Digits", "", "The number of significant digits displayed in the legend numbers","");

    CAF_PDM_InitField(&m_colorRangeMode, "ColorRangeMode", ColorRangeEnum(NORMAL) , "Colors", "", "", "");
    CAF_PDM_InitField(&m_rangeMode, "RangeType", RangeModeEnum(AUTOMATIC_ALLTIMESTEPS), "Range Type", "", "Switches between automatic and user defined range on the legend", "");
    CAF_PDM_InitField(&m_userDefinedMaxValue, "UserDefinedMax", 1.0, "Max", "", "Max value of the legend", "");
    CAF_PDM_InitField(&m_userDefinedMinValue, "UserDefinedMin", 0.0, "Min", "", "Min value of the legend (if mapping is logarithmic only positive values are valid)", "");
    CAF_PDM_InitField(&resultVariableName, "ResultVariableUsage", QString(""), "", "", "", "");
    resultVariableName.uiCapability()->setUiHidden(true);

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_scaleLegend = new caf::OverlayScaleLegend(standardFont);

    updateFieldVisibility();
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimScaleLegendConfig::~RimScaleLegendConfig()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_numLevels)
    {
        int upperLimit = std::numeric_limits<int>::max();
        m_numLevels = cvf::Math::clamp(m_numLevels.v(), 1, upperLimit);
    }
    else if (changedField == &m_rangeMode)
    {
        if (m_rangeMode == USER_DEFINED)
        {
            if (m_userDefinedMaxValue == m_userDefinedMaxValue.defaultValue() && m_globalAutoMax != cvf::UNDEFINED_DOUBLE)
            {
                m_userDefinedMaxValue = roundToNumSignificantDigits(m_globalAutoMax, m_precision);
            }
            if (m_userDefinedMinValue == m_userDefinedMinValue.defaultValue() && m_globalAutoMin != cvf::UNDEFINED_DOUBLE)
            {   
                m_userDefinedMinValue = roundToNumSignificantDigits(m_globalAutoMin, m_precision);
            }
        }

        updateFieldVisibility();
    }

    updateLegend();

    RimGridView* view = nullptr;
    this->firstAncestorOrThisOfType(view);

    if (view)
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->updateCellResult();
        }

        view->updateCurrentTimeStepAndRedraw();

        view->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    // Update stim plan templates if relevant
    RimStimPlanColors* stimPlanColors;
    firstAncestorOrThisOfType(stimPlanColors);
    if (stimPlanColors)
    {
        stimPlanColors->updateStimPlanTemplates();
    }

    // Update ensemble curve set if relevant
    RimEnsembleCurveSet* ensembleCurveSet;
    firstAncestorOrThisOfType(ensembleCurveSet);
    if (ensembleCurveSet)
    {
        ensembleCurveSet->onLegendDefinitionChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::updateLegend()
{
    double adjustedMin = cvf::UNDEFINED_DOUBLE;
    double adjustedMax = cvf::UNDEFINED_DOUBLE;
    
   if (m_rangeMode == AUTOMATIC_ALLTIMESTEPS)
   {
       adjustedMin = roundToNumSignificantDigits(m_globalAutoMin, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_globalAutoMax, m_precision);
   }
   else if (m_rangeMode == AUTOMATIC_CURRENT_TIMESTEP)
   {
       adjustedMin = roundToNumSignificantDigits(m_localAutoMin, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_localAutoMax, m_precision);
   }
   else
   {
       adjustedMin = roundToNumSignificantDigits(m_userDefinedMinValue, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_userDefinedMaxValue, m_precision);
   }

   cvf::Color3ubArray legendColors = colorArrayFromColorType(m_colorRangeMode());

   double decadesInRange = 0;

   {
       // For linear mapping, use the max value as reference for num valid digits
       double absRange = CVF_MAX(cvf::Math::abs(adjustedMax), cvf::Math::abs(adjustedMin));
       decadesInRange = log10(absRange);
   }

   decadesInRange = cvf::Math::ceil(decadesInRange);

   // Using Fixed format 
   //caf::OverlayScaleLegend::NumberFormat nft = m_tickNumberFormat();
   //m_scaleLegend->setTickFormat((caf::OverlayScaleLegend::NumberFormat)nft);

   // Set the fixed number of digits after the decimal point to the number needed to show all the significant digits.
   int numDecimalDigits = m_precision();
   m_scaleLegend->setTickPrecision(cvf::Math::clamp(numDecimalDigits, 0, 20));

   RiaApplication* app = RiaApplication::instance();
   RiaPreferences* preferences = app->preferences();
   m_scaleLegend->enableBackground(preferences->showLegendBackground());

   if (m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
   {
       m_userDefinedMaxValue.uiCapability()->setUiName(QString("Max ") + "(" + QString::number(m_globalAutoMax, 'g', m_precision) + ")");
   }
   else
   {
       m_userDefinedMaxValue.uiCapability()->setUiName(QString());
   }

   if (m_globalAutoMin != cvf::UNDEFINED_DOUBLE )
   {
       m_userDefinedMinValue.uiCapability()->setUiName(QString("Min ") + "(" + QString::number(m_globalAutoMin, 'g', m_precision) + ")");
   }
   else
   {
        m_userDefinedMinValue.uiCapability()->setUiName(QString());
   }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::disableAllTimeStepsRange(bool doDisable)
{
    // If we enable AllTimesteps, and we have used current timestep, then "restore" the default
    if (m_isAllTimeStepsRangeDisabled && !doDisable &&  m_rangeMode == AUTOMATIC_CURRENT_TIMESTEP)  m_rangeMode = AUTOMATIC_ALLTIMESTEPS;

    m_isAllTimeStepsRangeDisabled = doDisable;

    if (doDisable && m_rangeMode == AUTOMATIC_ALLTIMESTEPS) m_rangeMode = AUTOMATIC_CURRENT_TIMESTEP;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax)
{
    double candidateGlobalAutoMin = roundToNumSignificantDigits(globalMin, m_precision);
    double candidateGlobalAutoMax = roundToNumSignificantDigits(globalMax, m_precision);

    double candidateLocalAutoMin = roundToNumSignificantDigits(localMin, m_precision);
    double candidateLocalAutoMax = roundToNumSignificantDigits(localMax, m_precision);

    m_globalAutoMin = candidateGlobalAutoMin;
    m_globalAutoMax = candidateGlobalAutoMax;

    m_localAutoMin = candidateLocalAutoMin;
    m_localAutoMax = candidateLocalAutoMax;

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::initAfterRead()
{
    updateFieldVisibility();
}

caf::PdmFieldHandle* RimScaleLegendConfig::objectToggleField()
{
    return &m_showLegend;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::updateFieldVisibility()
{
    bool showRangeItems = true;

    m_numLevels.uiCapability()->setUiHidden(!showRangeItems);
    m_precision.uiCapability()->setUiHidden(!showRangeItems);
    m_rangeMode.uiCapability()->setUiHidden(!showRangeItems);

    if (showRangeItems && m_rangeMode == USER_DEFINED)
    {
        m_userDefinedMaxValue.uiCapability()->setUiHidden(false);
        m_userDefinedMinValue.uiCapability()->setUiHidden(false);
    }
    else
    {
        m_userDefinedMaxValue.uiCapability()->setUiHidden(true);
        m_userDefinedMinValue.uiCapability()->setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setColorRange(ColorRangesType colorMode)
{
    m_colorRangeMode = colorMode;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::recreateLegend()
{
    // Due to possible visualization bug, we need to recreate the legend if the last viewer 
    // has been removed, (and thus the opengl resources has been deleted) The text in 
    // the legend disappeared because of this, so workaround: recreate the legend when needed:

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_scaleLegend = new caf::OverlayScaleLegend(standardFont);

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// Rounding the double value to given number of significant digits
//--------------------------------------------------------------------------------------------------
double RimScaleLegendConfig::roundToNumSignificantDigits(double domainValue, double numSignificantDigits)
{
    double absDomainValue = cvf::Math::abs(domainValue);
    if (absDomainValue == 0.0)
    {
        return 0.0;
    }

    double logDecValue = log10(absDomainValue);
    logDecValue = cvf::Math::ceil(logDecValue);

    double factor = pow(10.0, numSignificantDigits - logDecValue);

    double tmp = domainValue * factor;
    double integerPart;
    double fraction = modf(tmp, &integerPart);

    if (cvf::Math::abs(fraction)>= 0.5) (integerPart >= 0) ? integerPart++: integerPart-- ;

    double newDomainValue = integerPart / factor;

    return newDomainValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setTitle(const QString& title)
{
    auto cvfTitle = cvfqt::Utils::toString(title);
    m_scaleLegend->setTitle(cvfTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimScaleLegendConfig::showLegend() const
{
    return m_showLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setShowLegend(bool show)
{
    m_showLegend = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::TitledOverlayFrame* RimScaleLegendConfig::titledOverlayFrame()
{
    return m_scaleLegend.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::TitledOverlayFrame* RimScaleLegendConfig::titledOverlayFrame() const
{
    return m_scaleLegend.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OverlayItem* RimScaleLegendConfig::overlayItem()
{
    return m_scaleLegend.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::OverlayItem* RimScaleLegendConfig::overlayItem() const
{
    return m_scaleLegend.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimLegendConfig::RangeModeType RimScaleLegendConfig::rangeMode() const
{
    return m_rangeMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setCurrentScale(double scale)
{
    m_currentScale = scale;

    // Update legend
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::setUiValuesFromLegendConfig(const RimScaleLegendConfig* otherLegendConfig)
{
    QString serializedObjectString = otherLegendConfig->writeObjectToXmlString();
    this->readObjectFromXmlString(serializedObjectString, caf::PdmDefaultObjectFactory::instance());
    this->updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray RimScaleLegendConfig::colorArrayFromColorType(ColorRangesType colorType)
{
    switch (colorType)
    {
    case RimScaleLegendConfig::NORMAL:
        return RiaColorTables::normalPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::OPPOSITE_NORMAL:
        return RiaColorTables::normalPaletteOppositeOrderingColors().color3ubArray();
        break;
    case RimScaleLegendConfig::WHITE_PINK:
        return RiaColorTables::whitePinkPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::PINK_WHITE:
        return RiaColorTables::pinkWhitePaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::WHITE_BLACK:
        return RiaColorTables::whiteBlackPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::BLACK_WHITE:
        return RiaColorTables::blackWhitePaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::BLUE_WHITE_RED:
        return RiaColorTables::blueWhiteRedPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::RED_WHITE_BLUE:
        return RiaColorTables::redWhiteBluePaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::CATEGORY:
        return RiaColorTables::categoryPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::ANGULAR:
        return RiaColorTables::angularPaletteColors().color3ubArray();
        break;
    case RimScaleLegendConfig::STIMPLAN:
        return RiaColorTables::stimPlanPaletteColors().color3ubArray();
        break;
    default:
        //if (ColorManager::isEnsembleColorRange(colorType)) return ColorManager::EnsembleColorRanges().at(colorType);
        break;
    }

    return RiaColorTables::normalPaletteColors().color3ubArray();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScaleLegendConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiOrdering * formatGr = uiOrdering.addNewGroup("Format");
        formatGr->add(&m_numLevels);
        formatGr->add(&m_precision);
        formatGr->add(&m_colorRangeMode);

        caf::PdmUiOrdering * mappingGr = uiOrdering.addNewGroup("Mapping");
        mappingGr->add(&m_rangeMode);
        mappingGr->add(&m_userDefinedMaxValue);
        mappingGr->add(&m_userDefinedMinValue);
    }

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimScaleLegendConfig::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    bool hasStimPlanParent = false;
    bool hasEnsembleCurveSetParent = false;

    RimStimPlanColors* stimPlanColors = nullptr;
    this->firstAncestorOrThisOfType(stimPlanColors);
    if (stimPlanColors) hasStimPlanParent = true;

    RimEnsembleCurveSet* ensembleCurveSet = nullptr;
    this->firstAncestorOrThisOfType(ensembleCurveSet);
    if (ensembleCurveSet) hasEnsembleCurveSetParent = true;

    bool isCategoryResult = false;
    {
        RimEclipseCellColors* eclCellColors = nullptr;
        this->firstAncestorOrThisOfType(eclCellColors);
        RimGeoMechResultDefinition* gmCellColors = nullptr;
        this->firstAncestorOrThisOfType(gmCellColors);
        RimCellEdgeColors* eclCellEdgColors = nullptr;
        this->firstAncestorOrThisOfType(eclCellEdgColors);

        if (   ( eclCellColors && eclCellColors->hasCategoryResult())
            || ( gmCellColors && gmCellColors->hasCategoryResult())
            || ( eclCellEdgColors && eclCellEdgColors->hasCategoryResult())
            || ( ensembleCurveSet && ensembleCurveSet->currentEnsembleParameterType() == EnsembleParameter::TYPE_TEXT) )
        {
            isCategoryResult = true;
        }
    }

    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_colorRangeMode)
    {
        // This is an app enum field, see cafInternalPdmFieldTypeSpecializations.h for the default specialization of this type
        std::vector<ColorRangesType> rangeTypes;
        if (!hasEnsembleCurveSetParent)
        {
            rangeTypes.push_back(NORMAL);
            rangeTypes.push_back(OPPOSITE_NORMAL);
            rangeTypes.push_back(WHITE_PINK);
            rangeTypes.push_back(PINK_WHITE);
            rangeTypes.push_back(BLUE_WHITE_RED);
            rangeTypes.push_back(RED_WHITE_BLUE);
            rangeTypes.push_back(WHITE_BLACK);
            rangeTypes.push_back(BLACK_WHITE);
            rangeTypes.push_back(ANGULAR);
        }
        else
        {
            //for (const auto& col : ColorManager::EnsembleColorRanges())
            //{
            //    rangeTypes.push_back(col.first);
            //}
        }

        if (hasStimPlanParent) rangeTypes.push_back(STIMPLAN);

        if (isCategoryResult)
        {
            rangeTypes.push_back(CATEGORY);
        }

        for(ColorRangesType colType: rangeTypes)
        {
            options.push_back(caf::PdmOptionItemInfo(ColorRangeEnum::uiText(colType), colType));
        }
    }
    else if (fieldNeedingOptions == &m_rangeMode)
    {
        if (!m_isAllTimeStepsRangeDisabled)
        {
            QString uiText;
            if(!hasEnsembleCurveSetParent) uiText = RangeModeEnum::uiText(RimScaleLegendConfig::AUTOMATIC_ALLTIMESTEPS);
            else                           uiText = "Auto Range";
            
            options.push_back(caf::PdmOptionItemInfo(uiText, RimScaleLegendConfig::AUTOMATIC_ALLTIMESTEPS));
        }
        if (!hasStimPlanParent && !hasEnsembleCurveSetParent)
        {
            options.push_back(caf::PdmOptionItemInfo(RangeModeEnum::uiText(RimScaleLegendConfig::AUTOMATIC_CURRENT_TIMESTEP), RimScaleLegendConfig::AUTOMATIC_CURRENT_TIMESTEP));
        }
        options.push_back(caf::PdmOptionItemInfo(RangeModeEnum::uiText(RimScaleLegendConfig::USER_DEFINED), RimScaleLegendConfig::USER_DEFINED));
    }
 
    return options;
}

