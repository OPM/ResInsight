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
#include "RiaPreferences.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimIntersectionCollection.h"
#include "RimStimPlanColors.h"
#include "RimViewLinker.h"

#include "cafTitledOverlayFrame.h"
#include "cafCategoryLegend.h"
#include "cafCategoryMapper.h"
#include "cafOverlayScalarMapperLegend.h"

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


CAF_PDM_SOURCE_INIT(RimRegularLegendConfig, "Legend");

namespace caf {
    template<>
    void AppEnum<RimRegularLegendConfig::RangeModeType>::setUp()
    {
        addItem(RimRegularLegendConfig::AUTOMATIC_ALLTIMESTEPS,    "AUTOMATIC_ALLTIMESTEPS",       "Min and Max for All Timesteps");
        addItem(RimRegularLegendConfig::AUTOMATIC_CURRENT_TIMESTEP,"AUTOMATIC_CURRENT_TIMESTEP",   "Min and Max for Current Timestep");
        addItem(RimRegularLegendConfig::USER_DEFINED,              "USER_DEFINED_MAX_MIN",         "User Defined Range");
        setDefault(RimRegularLegendConfig::AUTOMATIC_ALLTIMESTEPS);
    }
}

namespace caf {
    template<>
    void RimRegularLegendConfig::ColorRangeEnum::setUp()
    {
        addItem(RimRegularLegendConfig::NORMAL,         "NORMAL",          "Full color, Red on top");
        addItem(RimRegularLegendConfig::OPPOSITE_NORMAL,"OPPOSITE_NORMAL", "Full color, Blue on top");
        addItem(RimRegularLegendConfig::WHITE_PINK,     "WHITE_PIMK",      "White to pink");
        addItem(RimRegularLegendConfig::PINK_WHITE,     "PINK_WHITE",      "Pink to white");
        addItem(RimRegularLegendConfig::BLUE_WHITE_RED, "BLUE_WHITE_RED",  "Blue, white, red");
        addItem(RimRegularLegendConfig::RED_WHITE_BLUE, "RED_WHITE_BLUE",  "Red, white, blue");
        addItem(RimRegularLegendConfig::WHITE_BLACK,    "WHITE_BLACK",     "White to black");
        addItem(RimRegularLegendConfig::BLACK_WHITE,    "BLACK_WHITE",     "Black to white");
        addItem(RimRegularLegendConfig::CATEGORY,       "CATEGORY",        "Category colors");
        addItem(RimRegularLegendConfig::ANGULAR,        "ANGULAR",         "Full color cyclic");
        addItem(RimRegularLegendConfig::STIMPLAN,       "STIMPLAN",        "StimPlan colors");
        setDefault(RimRegularLegendConfig::NORMAL);
    }
}

namespace caf {
    template<>
    void RimRegularLegendConfig::MappingEnum::setUp()
    {
        addItem(RimRegularLegendConfig::LINEAR_DISCRETE,    "LinearDiscrete",   "Discrete Linear");
        addItem(RimRegularLegendConfig::LINEAR_CONTINUOUS,  "LinearContinuous", "Continuous Linear");
        addItem(RimRegularLegendConfig::LOG10_CONTINUOUS,   "Log10Continuous",  "Continuous Logarithmic");
        addItem(RimRegularLegendConfig::LOG10_DISCRETE,     "Log10Discrete",    "Discrete Logarithmic");
        addItem(RimRegularLegendConfig::CATEGORY_INTEGER,   "Category",         "Category");
        setDefault(RimRegularLegendConfig::LINEAR_CONTINUOUS);
    }
}

namespace caf {
    template<>
    void AppEnum<RimRegularLegendConfig::NumberFormatType>::setUp()
    {
        addItem(   RimRegularLegendConfig::AUTO,       "AUTO", "Automatic");
        addItem(   RimRegularLegendConfig::FIXED,      "FIXED",  "Fixed, decimal");
        addItem(   RimRegularLegendConfig::SCIENTIFIC, "SCIENTIFIC",    "Scientific notation");
        setDefault(RimRegularLegendConfig::FIXED);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::RimRegularLegendConfig() 
    :   m_globalAutoMax(cvf::UNDEFINED_DOUBLE),
        m_globalAutoMin(cvf::UNDEFINED_DOUBLE),
        m_localAutoMax(cvf::UNDEFINED_DOUBLE),
        m_localAutoMin(cvf::UNDEFINED_DOUBLE),
        m_globalAutoPosClosestToZero(0),
        m_globalAutoNegClosestToZero(0),
        m_localAutoPosClosestToZero(0),
        m_localAutoNegClosestToZero(0),
        m_isAllTimeStepsRangeDisabled(false)
{
    CAF_PDM_InitObject("Legend Definition", ":/Legend.png", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Show Legend", "", "", "");    
    m_showLegend.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_numLevels, "NumberOfLevels", 8, "Number of Levels", "", "A hint on how many tick marks you whish.","");
    CAF_PDM_InitField(&m_precision, "Precision", 4, "Significant Digits", "", "The number of significant digits displayed in the legend numbers","");
    CAF_PDM_InitField(&m_tickNumberFormat, "TickNumberFormat", caf::AppEnum<RimRegularLegendConfig::NumberFormatType>(FIXED), "Number format", "", "","");

    CAF_PDM_InitField(&m_colorRangeMode, "ColorRangeMode", ColorRangeEnum(NORMAL) , "Colors", "", "", "");
    CAF_PDM_InitField(&m_mappingMode, "MappingMode", MappingEnum(LINEAR_CONTINUOUS) , "Mapping", "", "", "");
    CAF_PDM_InitField(&m_rangeMode, "RangeType", RangeModeEnum(AUTOMATIC_ALLTIMESTEPS), "Range Type", "", "Switches between automatic and user defined range on the legend", "");
    CAF_PDM_InitField(&m_userDefinedMaxValue, "UserDefinedMax", 1.0, "Max", "", "Max value of the legend", "");
    CAF_PDM_InitField(&m_userDefinedMinValue, "UserDefinedMin", 0.0, "Min", "", "Min value of the legend (if mapping is logarithmic only positive values are valid)", "");
    CAF_PDM_InitField(&resultVariableName, "ResultVariableUsage", QString(""), "", "", "", "");
    resultVariableName.uiCapability()->setUiHidden(true);

    m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;
    m_logDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLog;
    m_linSmoothScalarMapper = new cvf::ScalarMapperContinuousLinear;
    m_logSmoothScalarMapper = new cvf::ScalarMapperContinuousLog;

    m_currentScalarMapper = m_linDiscreteScalarMapper;

    m_categoryMapper = new caf::CategoryMapper;

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_scalarMapperLegend = new caf::OverlayScalarMapperLegend(standardFont);
    m_categoryLegend = new caf::CategoryLegend(standardFont, m_categoryMapper.p());

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
void RimRegularLegendConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_numLevels)
    {
        int upperLimit = std::numeric_limits<int>::max();
        m_numLevels = cvf::Math::clamp(m_numLevels.v(), 1, upperLimit);
    }
    else if (changedField == &m_rangeMode ||
             changedField == &m_mappingMode)
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateLegend()
{
    double adjustedMin = cvf::UNDEFINED_DOUBLE;
    double adjustedMax = cvf::UNDEFINED_DOUBLE;
    
    double posClosestToZero = cvf::UNDEFINED_DOUBLE;
    double negClosestToZero = cvf::UNDEFINED_DOUBLE;

   if (m_rangeMode == AUTOMATIC_ALLTIMESTEPS)
   {
       adjustedMin = roundToNumSignificantDigits(m_globalAutoMin, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_globalAutoMax, m_precision);

       posClosestToZero = m_globalAutoPosClosestToZero;
       negClosestToZero = m_globalAutoNegClosestToZero;
   }
   else if (m_rangeMode == AUTOMATIC_CURRENT_TIMESTEP)
   {
       adjustedMin = roundToNumSignificantDigits(m_localAutoMin, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_localAutoMax, m_precision);

       posClosestToZero = m_localAutoPosClosestToZero;
       negClosestToZero = m_localAutoNegClosestToZero;
   }
   else
   {
       adjustedMin = roundToNumSignificantDigits(m_userDefinedMinValue, m_precision);
       adjustedMax = roundToNumSignificantDigits(m_userDefinedMaxValue, m_precision);

       posClosestToZero = m_globalAutoPosClosestToZero;
       negClosestToZero = m_globalAutoNegClosestToZero;
   }

   m_linDiscreteScalarMapper->setRange(adjustedMin, adjustedMax);
   m_linSmoothScalarMapper->setRange(adjustedMin, adjustedMax);

   if (m_mappingMode == LOG10_CONTINUOUS || m_mappingMode == LOG10_DISCRETE)
   {
       if (adjustedMin != adjustedMax)
       {
           if (adjustedMin == 0)
           {
               if (adjustedMax > adjustedMin)
               {
                   adjustedMin = posClosestToZero;
               }
               else
               {
                   adjustedMin = negClosestToZero;
               }
           }
           else if (adjustedMax == 0)
           {
               if (adjustedMin > adjustedMax)
               {
                   adjustedMax = posClosestToZero;
               }
               else
               {
                   adjustedMax = negClosestToZero;
               }
           }
           else if (adjustedMin < 0 && adjustedMax > 0)
           {
               adjustedMin = posClosestToZero;
           }
           else if (adjustedMax < 0 && adjustedMin > 0)
           {
               adjustedMin = negClosestToZero;
           }
       }
   }

   m_logDiscreteScalarMapper->setRange(adjustedMin, adjustedMax);
   m_logSmoothScalarMapper->setRange(adjustedMin, adjustedMax);

   cvf::Color3ubArray legendColors = colorArrayFromColorType(m_colorRangeMode());

   m_linDiscreteScalarMapper->setColors(legendColors);
   m_logDiscreteScalarMapper->setColors(legendColors);
   m_logSmoothScalarMapper->setColors(legendColors);
   m_linSmoothScalarMapper->setColors(legendColors);


   m_linDiscreteScalarMapper->setLevelCount(m_numLevels, true);
   m_logDiscreteScalarMapper->setLevelCount(m_numLevels, true);
   m_logSmoothScalarMapper->setLevelCount(m_numLevels, true);
   m_linSmoothScalarMapper->setLevelCount(m_numLevels, true);

   switch(m_mappingMode())
   {
   case LINEAR_DISCRETE:
       m_currentScalarMapper = m_linDiscreteScalarMapper.p();
       break;
   case LINEAR_CONTINUOUS:
       m_currentScalarMapper = m_linSmoothScalarMapper.p();
       break;
   case LOG10_CONTINUOUS:
       m_currentScalarMapper = m_logSmoothScalarMapper.p();
       break;
   case LOG10_DISCRETE:
       m_currentScalarMapper = m_logDiscreteScalarMapper.p();
       break;
   case CATEGORY_INTEGER:
       m_categoryMapper->setCategoriesWithNames(m_categories, m_categoryNames);

       if (m_categoryColors.size() > 0)
       {
            m_categoryMapper->setCycleColors(m_categoryColors);
       }
       else
       {
            m_categoryMapper->setInterpolateColors(legendColors);
       }
       m_currentScalarMapper = m_categoryMapper.p();
       break;
   default:
       break;
   }

   if (m_currentScalarMapper != m_categoryMapper.p())
   {
        m_scalarMapperLegend->setScalarMapper(m_currentScalarMapper.p());
   }
   double decadesInRange = 0;

   if (m_mappingMode == LOG10_CONTINUOUS || m_mappingMode == LOG10_DISCRETE)
   {
       // For log mapping, use the min value as reference for num valid digits
       decadesInRange  = cvf::Math::abs(adjustedMin) < cvf::Math::abs(adjustedMax) ? cvf::Math::abs(adjustedMin) : cvf::Math::abs(adjustedMax);
       decadesInRange = log10(decadesInRange);
   }
   else
   {
       // For linear mapping, use the max value as reference for num valid digits
       double absRange = CVF_MAX(cvf::Math::abs(adjustedMax), cvf::Math::abs(adjustedMin));
       decadesInRange = log10(absRange);
   }

   decadesInRange = cvf::Math::ceil(decadesInRange);

   // Using Fixed format 
   NumberFormatType nft = m_tickNumberFormat();
   m_scalarMapperLegend->setTickFormat((caf::OverlayScalarMapperLegend::NumberFormat)nft);

   // Set the fixed number of digits after the decimal point to the number needed to show all the significant digits.
   int numDecimalDigits = m_precision();
   if (nft != SCIENTIFIC)
   {
       numDecimalDigits -= static_cast<int>(decadesInRange);
   }
   m_scalarMapperLegend->setTickPrecision(cvf::Math::clamp(numDecimalDigits, 0, 20));
   m_scalarMapperLegend->computeLayoutAndExtents(cvf::Vec2i(0, 0), m_scalarMapperLegend->sizeHint());

   RiaApplication* app = RiaApplication::instance();
   RiaPreferences* preferences = app->preferences();
   m_scalarMapperLegend->enableBackground(preferences->showLegendBackground());
   m_categoryLegend->enableBackground(preferences->showLegendBackground());

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
void RimRegularLegendConfig::disableAllTimeStepsRange(bool doDisable)
{
    // If we enable AllTimesteps, and we have used current timestep, then "restore" the default
    if (m_isAllTimeStepsRangeDisabled && !doDisable &&  m_rangeMode == AUTOMATIC_CURRENT_TIMESTEP)  m_rangeMode = AUTOMATIC_ALLTIMESTEPS;

    m_isAllTimeStepsRangeDisabled = doDisable;

    if (doDisable && m_rangeMode == AUTOMATIC_ALLTIMESTEPS) m_rangeMode = AUTOMATIC_CURRENT_TIMESTEP;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax)
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
void RimRegularLegendConfig::initAfterRead()
{
    updateFieldVisibility();
}

caf::PdmFieldHandle* RimRegularLegendConfig::objectToggleField()
{
    return &m_showLegend;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::updateFieldVisibility()
{
    bool showRangeItems = m_mappingMode == CATEGORY_INTEGER ? false : true;

    m_numLevels.uiCapability()->setUiHidden(!showRangeItems);
    m_precision.uiCapability()->setUiHidden(!showRangeItems);
    m_tickNumberFormat.uiCapability()->setUiHidden(!showRangeItems);
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
void RimRegularLegendConfig::setColorRangeMode(ColorRangesType colorMode)
{
    m_colorRangeMode = colorMode;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setMappingMode(MappingType mappingType)
{
    m_mappingMode = mappingType;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::recreateLegend()
{
    // Due to possible visualization bug, we need to recreate the legend if the last viewer 
    // has been removed, (and thus the opengl resources has been deleted) The text in 
    // the legend disappeared because of this, so workaround: recreate the legend when needed:

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_scalarMapperLegend = new caf::OverlayScalarMapperLegend(standardFont);
    m_categoryLegend = new caf::CategoryLegend(standardFont, m_categoryMapper.p());

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// Rounding the double value to given number of significant digits
//--------------------------------------------------------------------------------------------------
double RimRegularLegendConfig::roundToNumSignificantDigits(double domainValue, double numSignificantDigits)
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
void RimRegularLegendConfig::setClosestToZeroValues(double globalPosClosestToZero, double globalNegClosestToZero, double localPosClosestToZero, double localNegClosestToZero)
{
    bool needsUpdate = false;
    const double epsilon = std::numeric_limits<double>::epsilon();

    if (cvf::Math::abs(globalPosClosestToZero - m_globalAutoPosClosestToZero) > epsilon)
    {
        needsUpdate = true;
    }
    if (cvf::Math::abs(globalNegClosestToZero - m_globalAutoNegClosestToZero) > epsilon)
    {
        needsUpdate = true;
    }
    if (cvf::Math::abs(localPosClosestToZero - m_localAutoPosClosestToZero) > epsilon)
    {
        needsUpdate = true;
    }
    if (cvf::Math::abs(localNegClosestToZero - m_localAutoNegClosestToZero) > epsilon)
    {
        needsUpdate = true;
    }

    if (needsUpdate)
    {
        m_globalAutoPosClosestToZero = globalPosClosestToZero;
        m_globalAutoNegClosestToZero = globalNegClosestToZero;
        m_localAutoPosClosestToZero = localPosClosestToZero;
        m_localAutoNegClosestToZero = localNegClosestToZero;

        if (m_globalAutoPosClosestToZero == HUGE_VAL) m_globalAutoPosClosestToZero = 0;
        if (m_globalAutoNegClosestToZero == -HUGE_VAL) m_globalAutoNegClosestToZero = 0; 
        if (m_localAutoPosClosestToZero == HUGE_VAL) m_localAutoPosClosestToZero = 0;
        if (m_localAutoNegClosestToZero == -HUGE_VAL) m_localAutoNegClosestToZero = 0;

        updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setIntegerCategories(const std::vector<int>& categories)
{
    m_categories = categories;
    m_categoryNames.clear();
    m_categoryColors.clear();

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setNamedCategoriesInverse(const std::vector<QString>& categoryNames)
{
    std::vector<int> nameIndices;
    std::vector<cvf::String> names;
    for(int i =  static_cast<int>(categoryNames.size()) - 1; i >= 0; --i)
    {
        nameIndices.push_back(i);
        names.push_back(cvfqt::Utils::toString(categoryNames[i]));
    }
    
    m_categories = nameIndices;
    m_categoryNames = names;
    m_categoryColors.clear();

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setCategoryItems(const std::vector< std::tuple<QString, int, cvf::Color3ub> >& categories)
{
    m_categories.clear();
    m_categoryNames.clear();
    m_categoryColors.clear();
    m_categoryColors.reserve(categories.size());

    for (auto item : categories)
    {
        m_categoryNames.push_back(cvfqt::Utils::toString(std::get<0>(item)));
        m_categories.push_back(std::get<1>(item));
        m_categoryColors.add(std::get<2>(item));
    }

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimRegularLegendConfig::categoryNameFromCategoryValue(double categoryResultValue) const
{
    if (categoryResultValue == HUGE_VAL) return "Undefined";

    if (m_categoryNames.size() > 0)
    {
        for (size_t categoryIndex = 0; categoryIndex < m_categories.size(); categoryIndex++)
        {
            if (categoryResultValue == m_categories[categoryIndex])
            {
                return cvfqt::Utils::toQString(m_categoryNames[categoryIndex]);
            }
        }
    }

    return QString("%1").arg(categoryResultValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::setTitle(const QString& title)
{
    auto cvfTitle = cvfqt::Utils::toString(title);
    m_scalarMapperLegend->setTitle(cvfTitle);
    m_categoryLegend->setTitle(cvfTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::TitledOverlayFrame* RimRegularLegendConfig::legend()
{
    if (m_currentScalarMapper == m_categoryMapper)
    {
        return m_categoryLegend.p();
    }
    else
    {
        return m_scalarMapperLegend.p();
    }
}

bool RimRegularLegendConfig::showLegend() const
{
    return m_showLegend;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::TitledOverlayFrame* RimRegularLegendConfig::titledOverlayFrame()
{
    if (m_currentScalarMapper == m_categoryMapper)
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
    if (m_currentScalarMapper == m_categoryMapper)
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
void RimRegularLegendConfig::setUiValuesFromLegendConfig(const RimRegularLegendConfig* otherLegendConfig)
{
    QString serializedObjectString = otherLegendConfig->writeObjectToXmlString();
    this->readObjectFromXmlString(serializedObjectString, caf::PdmDefaultObjectFactory::instance());
    this->updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray RimRegularLegendConfig::colorArrayFromColorType(ColorRangesType colorType)
{
    switch (colorType)
    {
    case RimRegularLegendConfig::NORMAL:
        return RiaColorTables::normalPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::OPPOSITE_NORMAL:
        return RiaColorTables::normalPaletteOppositeOrderingColors().color3ubArray();
        break;
    case RimRegularLegendConfig::WHITE_PINK:
        return RiaColorTables::whitePinkPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::PINK_WHITE:
        return RiaColorTables::pinkWhitePaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::WHITE_BLACK:
        return RiaColorTables::whiteBlackPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::BLACK_WHITE:
        return RiaColorTables::blackWhitePaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::BLUE_WHITE_RED:
        return RiaColorTables::blueWhiteRedPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::RED_WHITE_BLUE:
        return RiaColorTables::redWhiteBluePaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::CATEGORY:
        return RiaColorTables::categoryPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::ANGULAR:
        return RiaColorTables::angularPaletteColors().color3ubArray();
        break;
    case RimRegularLegendConfig::STIMPLAN:
        return RiaColorTables::stimPlanPaletteColors().color3ubArray();
        break;
    default:
        break;
    }

    return RiaColorTables::normalPaletteColors().color3ubArray();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRegularLegendConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiOrdering * formatGr = uiOrdering.addNewGroup("Format");
        formatGr->add(&m_numLevels);
        formatGr->add(&m_precision);
        formatGr->add(&m_tickNumberFormat);
        formatGr->add(&m_colorRangeMode);

        caf::PdmUiOrdering * mappingGr = uiOrdering.addNewGroup("Mapping");
        mappingGr->add(&m_mappingMode);
        mappingGr->add(&m_rangeMode);
        mappingGr->add(&m_userDefinedMaxValue);
        mappingGr->add(&m_userDefinedMinValue);
    }

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRegularLegendConfig::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    bool hasStimPlanParent = false;

    RimStimPlanColors* stimPlanColors = nullptr;
    this->firstAncestorOrThisOfType(stimPlanColors);
    if (stimPlanColors) hasStimPlanParent = true;

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
            || ( eclCellEdgColors && eclCellEdgColors->hasCategoryResult()) )
        {
            isCategoryResult = true;
        }
    }

    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_mappingMode)
    {
        // This is an app enum field, see cafInternalPdmFieldTypeSpecializations.h for the default specialization of this type
        std::vector<MappingType> mappingTypes;
        mappingTypes.push_back(LINEAR_DISCRETE);
        mappingTypes.push_back(LINEAR_CONTINUOUS);
        mappingTypes.push_back(LOG10_CONTINUOUS);
        mappingTypes.push_back(LOG10_DISCRETE);

        if (isCategoryResult)
        {
            mappingTypes.push_back(CATEGORY_INTEGER);
        }

        for(MappingType mapType: mappingTypes)
        {
            options.push_back(caf::PdmOptionItemInfo(MappingEnum::uiText(mapType), mapType));
        }
    }
    else if (fieldNeedingOptions == &m_colorRangeMode)
    {
        // This is an app enum field, see cafInternalPdmFieldTypeSpecializations.h for the default specialization of this type
        std::vector<ColorRangesType> rangeTypes;
        rangeTypes.push_back(NORMAL);
        rangeTypes.push_back(OPPOSITE_NORMAL);
        rangeTypes.push_back(WHITE_PINK);
        rangeTypes.push_back(PINK_WHITE);
        rangeTypes.push_back(BLUE_WHITE_RED);
        rangeTypes.push_back(RED_WHITE_BLUE);
        rangeTypes.push_back(WHITE_BLACK);
        rangeTypes.push_back(BLACK_WHITE);
        rangeTypes.push_back(ANGULAR);
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
            options.push_back(caf::PdmOptionItemInfo(RangeModeEnum::uiText(RimRegularLegendConfig::AUTOMATIC_ALLTIMESTEPS), RimRegularLegendConfig::AUTOMATIC_ALLTIMESTEPS));
        }
        if (!hasStimPlanParent)
        {
            options.push_back(caf::PdmOptionItemInfo(RangeModeEnum::uiText(RimRegularLegendConfig::AUTOMATIC_CURRENT_TIMESTEP), RimRegularLegendConfig::AUTOMATIC_CURRENT_TIMESTEP));
        }
        options.push_back(caf::PdmOptionItemInfo(RangeModeEnum::uiText(RimRegularLegendConfig::USER_DEFINED), RimRegularLegendConfig::USER_DEFINED));
    }
 
    return options;
}

