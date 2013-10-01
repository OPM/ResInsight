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

#include "RimLegendConfig.h"

#include "RimReservoirView.h"
#include "cafFactory.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiComboBoxEditor.h"

#include "cvfScalarMapperDiscreteLog.h"
#include "cvfScalarMapperContinuousLog.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include <cmath>

#include "RiaApplication.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

CAF_PDM_SOURCE_INIT(RimLegendConfig, "Legend");

namespace caf {
    template<>
    void AppEnum<RimLegendConfig::RangeModeType>::setUp()
    {
        addItem(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS,    "AUTOMATIC_ALLTIMESTEPS",       "Global range");
        addItem(RimLegendConfig::AUTOMATIC_CURRENT_TIMESTEP,"AUTOMATIC_CURRENT_TIMESTEP",   "Local range");
        addItem(RimLegendConfig::USER_DEFINED,              "USER_DEFINED_MAX_MIN",         "User defined range");
        setDefault(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS);
    }
}

namespace caf {
    template<>
    void RimLegendConfig::ColorRangeEnum::setUp()
    {
        addItem(RimLegendConfig::NORMAL,         "NORMAL",          "Full color, Red on top");
        addItem(RimLegendConfig::OPPOSITE_NORMAL,"OPPOSITE_NORMAL", "Full color, Blue on top");
        addItem(RimLegendConfig::WHITE_PINK,     "WHITE_PIMK",      "White to pink");
        addItem(RimLegendConfig::PINK_WHITE,     "PINK_WHITE",      "Pink to white");
        addItem(RimLegendConfig::BLUE_WHITE_RED,    "BLUE_WHITE_RED",     "Blue, white, red");
        addItem(RimLegendConfig::RED_WHITE_BLUE,    "RED_WHITE_BLUE",     "Red, white, blue");
        addItem(RimLegendConfig::WHITE_BLACK,    "WHITE_BLACK",     "White to black");
        addItem(RimLegendConfig::BLACK_WHITE,    "BLACK_WHITE",     "Black to white");
        setDefault(RimLegendConfig::NORMAL);
    }
}

namespace caf {
    template<>
    void RimLegendConfig::MappingEnum::setUp()
    {
        addItem(RimLegendConfig::LINEAR_DISCRETE,    "LinearDiscrete",   "Discrete Linear");
        addItem(RimLegendConfig::LINEAR_CONTINUOUS,  "LinearContinuous", "Continuous Linear");
        addItem(RimLegendConfig::LOG10_CONTINUOUS,   "Log10Continuous",  "Continuous Logarithmic");
        addItem(RimLegendConfig::LOG10_DISCRETE,     "Log10Discrete",    "Discrete Logarithmic");
        setDefault(RimLegendConfig::LINEAR_CONTINUOUS);
    }
}

namespace caf {
    template<>
    void AppEnum<RimLegendConfig::NumberFormatType>::setUp()
    {
        addItem(   RimLegendConfig::AUTO,       "AUTO", "Automatic");
        addItem(   RimLegendConfig::FIXED,      "FIXED",  "Fixed, decimal");
        addItem(   RimLegendConfig::SCIENTIFIC, "SCIENTIFIC",    "Scientific notation");
        setDefault(RimLegendConfig::FIXED);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig::RimLegendConfig() 
    :   m_globalAutoMax(cvf::UNDEFINED_DOUBLE),
        m_globalAutoMin(cvf::UNDEFINED_DOUBLE),
        m_localAutoMax(cvf::UNDEFINED_DOUBLE),
        m_localAutoMin(cvf::UNDEFINED_DOUBLE),
        m_globalAutoPosClosestToZero(0),
        m_globalAutoNegClosestToZero(0),
        m_localAutoPosClosestToZero(0),
        m_localAutoNegClosestToZero(0)
{
    CAF_PDM_InitObject("Legend Definition", ":/Legend.png", "", "");
    CAF_PDM_InitField(&m_numLevels, "NumberOfLevels", 8, "Number of levels", "", "A hint on how many tick marks you whish.","");
    CAF_PDM_InitField(&m_precision, "Precision", 2, "Significant digits", "", "The number of significant digits displayed in the legend numbers","");
    CAF_PDM_InitField(&m_tickNumberFormat, "TickNumberFormat", caf::AppEnum<RimLegendConfig::NumberFormatType>(FIXED), "Number format", "", "","");

    CAF_PDM_InitField(&m_colorRangeMode, "ColorRangeMode", ColorRangeEnum(NORMAL) , "Colors", "", "", "");
    CAF_PDM_InitField(&m_mappingMode, "MappingMode", MappingEnum(LINEAR_CONTINUOUS) , "Mapping", "", "", "");
    CAF_PDM_InitField(&m_rangeMode, "RangeType", caf::AppEnum<RimLegendConfig::RangeModeType>(AUTOMATIC_ALLTIMESTEPS), "Range type", "", "Switches between automatic and user defined range on the legend", "");
    CAF_PDM_InitField(&m_userDefinedMaxValue, "UserDefinedMax", 1.0, "Max", "", "Min value of the legend", "");
    CAF_PDM_InitField(&m_userDefinedMinValue, "UserDefinedMin", 0.0, "Min", "", "Max value of the legend", "");
    CAF_PDM_InitField(&resultVariableName, "ResultVariableUsage", QString(""), "", "", "", "");
    resultVariableName.setUiHidden(true);

    m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;
    m_logDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLog;
    m_linSmoothScalarMapper = new cvf::ScalarMapperContinuousLinear;
    m_logSmoothScalarMapper = new cvf::ScalarMapperContinuousLog;

    m_currentScalarMapper = m_linDiscreteScalarMapper;

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_legend = new cvf::OverlayScalarMapperLegend(standardFont);
    m_position = cvf::Vec2ui(20, 50);

    updateFieldVisibility();
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig::~RimLegendConfig()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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

    if (m_reservoirView) m_reservoirView->updateCurrentTimeStepAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::updateLegend()
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

   cvf::Color3ubArray legendColors;
   switch (m_colorRangeMode())
   {
   case NORMAL: 
       {
           legendColors.reserve(7);
           legendColors.add(cvf::Color3ub(  0,   0, 255));
           legendColors.add(cvf::Color3ub(  0, 127, 255));
           legendColors.add(cvf::Color3ub(  0, 255, 255));
           legendColors.add(cvf::Color3ub(  0, 255,   0));
           legendColors.add(cvf::Color3ub(255, 255,   0));
           legendColors.add(cvf::Color3ub(255, 127,   0));
           legendColors.add(cvf::Color3ub(255,   0,   0));
       }
       break;
   case OPPOSITE_NORMAL: 
       {
           legendColors.reserve(7);
           legendColors.add(cvf::Color3ub(255,   0,   0));
           legendColors.add(cvf::Color3ub(255, 127,   0));
           legendColors.add(cvf::Color3ub(255, 255,   0));
           legendColors.add(cvf::Color3ub(  0, 255,   0));
           legendColors.add(cvf::Color3ub(  0, 255, 255));
           legendColors.add(cvf::Color3ub(  0, 127, 255));
           legendColors.add(cvf::Color3ub(  0,   0, 255));
       }
       break;  
   case BLACK_WHITE:
   case WHITE_BLACK:
       {
           legendColors.reserve(2);
           if (m_colorRangeMode() == BLACK_WHITE)
           {
               legendColors.add(cvf::Color3ub::BLACK);
               legendColors.add(cvf::Color3ub::WHITE);
           }
           else
           {
               legendColors.add(cvf::Color3ub::WHITE);
               legendColors.add(cvf::Color3ub::BLACK);
           }
       }
       break;
   case PINK_WHITE:
   case WHITE_PINK:
       {
           legendColors.reserve(2);
           if (m_colorRangeMode() == PINK_WHITE)
           {
               legendColors.add(cvf::Color3ub::DEEP_PINK);
               legendColors.add(cvf::Color3ub::WHITE);
           }
           else
           {
               legendColors.add(cvf::Color3ub::WHITE);
               legendColors.add(cvf::Color3ub::DEEP_PINK);
           }
       }
       break;
   case BLUE_WHITE_RED:
   case RED_WHITE_BLUE:
       {
           legendColors.reserve(3);
           if (m_colorRangeMode() == BLUE_WHITE_RED)
           {
               legendColors.add(cvf::Color3ub::BLUE);
               legendColors.add(cvf::Color3ub::WHITE);
               legendColors.add(cvf::Color3ub::RED);
           }
           else
           {
               legendColors.add(cvf::Color3ub::RED);
               legendColors.add(cvf::Color3ub::WHITE);
               legendColors.add(cvf::Color3ub::BLUE);
           }
       }
       break;

   }

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
   default:
       break;
   }

   m_legend->setScalarMapper(m_currentScalarMapper.p());
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
   m_legend->setTickFormat((cvf::OverlayScalarMapperLegend::NumberFormat)nft);

   // Set the fixed number of digits after the decimal point to the number needed to show all the significant digits.
   int numDecimalDigits = m_precision();
   if (nft != SCIENTIFIC)
   {
       numDecimalDigits -= static_cast<int>(decadesInRange);
   }
   m_legend->setTickPrecision(cvf::Math::clamp(numDecimalDigits, 0, 20));


   if (m_globalAutoMax != cvf::UNDEFINED_DOUBLE )
   {
       m_userDefinedMaxValue.setUiName(QString("Max ") + "(" + QString::number(m_globalAutoMax, 'g', m_precision) + ")");
   }
   else
   {
       m_userDefinedMaxValue.setUiName(QString());
   }

   if (m_globalAutoMin != cvf::UNDEFINED_DOUBLE )
   {
       m_userDefinedMinValue.setUiName(QString("Min ") + "(" + QString::number(m_globalAutoMin, 'g', m_precision) + ")");
   }
   else
   {
        m_userDefinedMinValue.setUiName(QString());
   }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::setAutomaticRanges(double globalMin, double globalMax, double localMin, double localMax)
{
    double candidateGlobalAutoMin = roundToNumSignificantDigits(globalMin, m_precision);
    double candidateGlobalAutoMax = roundToNumSignificantDigits(globalMax, m_precision);

    double candidateLocalAutoMin = roundToNumSignificantDigits(localMin, m_precision);
    double candidateLocalAutoMax = roundToNumSignificantDigits(localMax, m_precision);

    bool needsUpdate = false;
    const double epsilon = std::numeric_limits<double>::epsilon();
    
    if (cvf::Math::abs(candidateGlobalAutoMax - m_globalAutoMax) > epsilon)
    {
        needsUpdate = true;
    }

    if (cvf::Math::abs(candidateGlobalAutoMin - m_globalAutoMin) > epsilon)
    {
        needsUpdate = true;
    }

    if (cvf::Math::abs(candidateLocalAutoMax - m_localAutoMax) > epsilon)
    {
        needsUpdate = true;
    }

    if (cvf::Math::abs(candidateLocalAutoMin - m_localAutoMin) > epsilon)
    {
        needsUpdate = true;
    }

    if (needsUpdate)
    {
        m_globalAutoMin = candidateGlobalAutoMin;
        m_globalAutoMax = candidateGlobalAutoMax;

        m_localAutoMin = candidateLocalAutoMin;
        m_localAutoMax = candidateLocalAutoMax;

        updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::initAfterRead()
{
    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::updateFieldVisibility()
{
    if (m_rangeMode == USER_DEFINED)
    {
        m_userDefinedMaxValue.setUiHidden(false);
        m_userDefinedMinValue.setUiHidden(false);
    }
    else
    {
        m_userDefinedMaxValue.setUiHidden(true);
        m_userDefinedMinValue.setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::setColorRangeMode(ColorRangesType colorMode)
{
    m_colorRangeMode = colorMode;
    updateLegend();
}

/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Color3ubArray> RimLegendConfig::interpolateColorArray(const cvf::Color3ubArray& colorArray, cvf::uint targetColorCount)
{
    uint inputColorCount = static_cast<uint>(colorArray.size());
    CVF_ASSERT(inputColorCount > 1);
    CVF_ASSERT(targetColorCount > 1);

    cvf::ref<cvf::Color3ubArray> colors = new cvf::Color3ubArray;
    colors->reserve(targetColorCount);

    const uint inputLevelCount = inputColorCount - 1;
    const uint outputLevelCount = targetColorCount - 1;

    uint outputLevelIdx;
    for (outputLevelIdx = 0; outputLevelIdx < outputLevelCount; outputLevelIdx++)
    {
        double dblInputLevelIndex = inputLevelCount * (outputLevelIdx / static_cast<double>(outputLevelCount));

        const uint inputLevelIndex = static_cast<uint>(dblInputLevelIndex);
        CVF_ASSERT(inputLevelIndex < inputLevelCount);

        double t = dblInputLevelIndex - inputLevelIndex;
        CVF_ASSERT(t >= 0 && t <= 1.0);

        cvf::Color3ub c1 = colorArray[inputLevelIndex];
        cvf::Color3ub c2 = colorArray[inputLevelIndex + 1];

        int r = static_cast<int>(c1.r() + t*(c2.r() - c1.r()) + 0.5);
        int g = static_cast<int>(c1.g() + t*(c2.g() - c1.g()) + 0.5);
        int b = static_cast<int>(c1.b() + t*(c2.b() - c1.b()) + 0.5);

        r = cvf::Math::clamp(r, 0, 255);
        g = cvf::Math::clamp(g, 0, 255);
        b = cvf::Math::clamp(b, 0, 255);

        cvf::Color3ub col((cvf::ubyte)r, (cvf::ubyte)g, (cvf::ubyte)b);
        colors->add(col);
    }

    colors->add(colorArray[colorArray.size() - 1]);

    return colors;
}
*/
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::setPosition(cvf::Vec2ui position)
{
    m_position = position;
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLegendConfig::recreateLegend()
{
    // Due to possible visualization bug, we need to recreate the legend if the last viewer 
    // has been removed, (and thus the opengl resources has been deleted) The text in 
    // the legend disappeared because of this, so workaround: recreate the legend when needed:

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_legend = new cvf::OverlayScalarMapperLegend(standardFont);

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// Rounding the double value to given number of significant digits
//--------------------------------------------------------------------------------------------------
double RimLegendConfig::roundToNumSignificantDigits(double domainValue, double numSignificantDigits)
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
void RimLegendConfig::setClosestToZeroValues(double globalPosClosestToZero, double globalNegClosestToZero, double localPosClosestToZero, double localNegClosestToZero)
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
void RimLegendConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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

