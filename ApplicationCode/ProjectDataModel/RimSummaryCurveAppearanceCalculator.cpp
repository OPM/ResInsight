/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryCurve.h"
#include "cvfVector3.h"

#include <cmath>

namespace caf
{
template<>
void caf::AppEnum< RimSummaryCurveAppearanceCalculator::CurveAppearanceType >::setUp()
{
    addItem(RimSummaryCurveAppearanceCalculator::NONE,           "NONE",           "None");
    addItem(RimSummaryCurveAppearanceCalculator::COLOR,          "COLOR",          "Color");
    addItem(RimSummaryCurveAppearanceCalculator::SYMBOL,         "SYMBOL",         "Symbols");
    addItem(RimSummaryCurveAppearanceCalculator::LINE_STYLE,     "LINE_STYLE",     "Line Style");
    addItem(RimSummaryCurveAppearanceCalculator::GRADIENT,       "GRADIENT",       "Gradient");
    addItem(RimSummaryCurveAppearanceCalculator::LINE_THICKNESS, "LINE_THICKNESS", "Line Thickness");

    setDefault(RimSummaryCurveAppearanceCalculator::NONE);
}
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveAppearanceCalculator::RimSummaryCurveAppearanceCalculator(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions)
{
    for(const std::pair<RimSummaryCase*, RifEclipseSummaryAddress>& curveDef : curveDefinitions)
    {
        if(curveDef.first)                           m_caseToAppearanceIdxMap[curveDef.first]                  = -1;
        if(!curveDef.second.quantityName().empty())  m_varToAppearanceIdxMap[curveDef.second.quantityName()]  = -1;
        if(!curveDef.second.wellName().empty())      m_welToAppearanceIdxMap[curveDef.second.wellName()]      = -1;
        if(!curveDef.second.wellGroupName().empty()) m_grpToAppearanceIdxMap[curveDef.second.wellGroupName()] = -1;
        if(!(curveDef.second.regionNumber() == -1))  m_regToAppearanceIdxMap[curveDef.second.regionNumber()]  = -1;
    }

    m_caseCount     = m_caseToAppearanceIdxMap.size();
    m_variableCount = m_varToAppearanceIdxMap .size();
    m_wellCount     = m_welToAppearanceIdxMap .size();
    m_groupCount    = m_grpToAppearanceIdxMap .size();
    m_regionCount   = m_regToAppearanceIdxMap .size();

    // Select the appearance type for each data "dimension"
    m_caseAppearanceType   = NONE;
    m_varAppearanceType    = NONE;
    m_wellAppearanceType   = NONE;
    m_groupAppearanceType  = NONE;
    m_regionAppearanceType = NONE;

    std::set<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> unusedAppearTypes;
    unusedAppearTypes.insert(COLOR);
    unusedAppearTypes.insert(GRADIENT);
    unusedAppearTypes.insert(LINE_STYLE);
    unusedAppearTypes.insert(SYMBOL);
    unusedAppearTypes.insert(LINE_THICKNESS);

    m_dimensionCount = 0;
    if(m_variableCount > 1) { m_varAppearanceType    = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_caseCount     > 1) { m_caseAppearanceType   = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_wellCount     > 1) { m_wellAppearanceType   = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_groupCount    > 1) { m_groupAppearanceType  = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_regionCount   > 1) { m_regionAppearanceType = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }

    // Assign increasing indexes             
    { int idx = 0; for(auto& pair : m_caseToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_varToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_welToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_grpToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_regToAppearanceIdxMap) pair.second = idx++; }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAppearanceCalculator::assignDimensions(  CurveAppearanceType caseAppearance, 
                                                CurveAppearanceType variAppearance, 
                                                CurveAppearanceType wellAppearance, 
                                                CurveAppearanceType gropAppearance, 
                                                CurveAppearanceType regiAppearance)
{
    m_dimensionCount = 0;
    m_caseAppearanceType   = caseAppearance;
    m_varAppearanceType    = variAppearance;
    m_wellAppearanceType   = wellAppearance;
    m_groupAppearanceType  = gropAppearance;
    m_regionAppearanceType = regiAppearance;

    if(m_caseAppearanceType   != NONE) ++m_dimensionCount;
    if(m_varAppearanceType    != NONE) ++m_dimensionCount;
    if(m_wellAppearanceType   != NONE) ++m_dimensionCount;
    if(m_groupAppearanceType  != NONE) ++m_dimensionCount;
    if(m_regionAppearanceType != NONE) ++m_dimensionCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAppearanceCalculator::getDimensions(CurveAppearanceType* caseAppearance, 
                                                        CurveAppearanceType* variAppearance, 
                                                        CurveAppearanceType* wellAppearance, 
                                                        CurveAppearanceType* gropAppearance, 
                                                        CurveAppearanceType* regiAppearance) const
{
    *caseAppearance =  m_caseAppearanceType  ;
    *variAppearance =  m_varAppearanceType   ;
    *wellAppearance =  m_wellAppearanceType  ;
    *gropAppearance =  m_groupAppearanceType ;
    *regiAppearance =  m_regionAppearanceType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAppearanceCalculator::setupCurveLook(RimSummaryCurve* curve)
{
    m_currentCurveBaseColor = cvf::Color3f(0, 0, 0);
    m_currentCurveGradient = 0.0f;

    int caseAppearanceIdx = m_caseToAppearanceIdxMap[curve->summaryCase()];
    int varAppearanceIdx = m_varToAppearanceIdxMap[curve->summaryAddress().quantityName()];
    int welAppearanceIdx = m_welToAppearanceIdxMap[curve->summaryAddress().wellName()];
    int grpAppearanceIdx = m_grpToAppearanceIdxMap[curve->summaryAddress().wellGroupName()];
    int regAppearanceIdx = m_regToAppearanceIdxMap[curve->summaryAddress().regionNumber()];

    setOneCurveAppearance(m_caseAppearanceType,   m_caseCount,     caseAppearanceIdx, curve);
    setOneCurveAppearance(m_varAppearanceType,    m_variableCount, varAppearanceIdx,  curve);
    setOneCurveAppearance(m_wellAppearanceType,   m_wellCount,     welAppearanceIdx,  curve);
    setOneCurveAppearance(m_groupAppearanceType,  m_groupCount,    grpAppearanceIdx,  curve);
    setOneCurveAppearance(m_regionAppearanceType, m_regionCount,   regAppearanceIdx,  curve);

    curve->setColor(gradeColor(m_currentCurveBaseColor, m_currentCurveGradient));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAppearanceCalculator::setOneCurveAppearance(CurveAppearanceType appeaType, size_t totalCount, int appeaIdx, RimSummaryCurve* curve)
{
    switch(appeaType)
    {
        case NONE:
        break;
        case COLOR:
        m_currentCurveBaseColor = cycledPaletteColor(appeaIdx);
        break;
        case GRADIENT:
        m_currentCurveGradient = gradient(totalCount, appeaIdx);
        break;
        case LINE_STYLE:
        curve->setLineStyle(cycledLineStyle(appeaIdx));
        break;
        case SYMBOL:
        curve->setSymbol(cycledSymbol(appeaIdx));
        break;
        case LINE_THICKNESS:
        curve->setLineThickness(cycledLineThickness(appeaIdx));
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledPaletteColor(int colorIndex)
{
    static const int RI_LOGPLOT_CURVECOLORSCOUNT = 11;
    static const cvf::ubyte RI_LOGPLOT_CURVECOLORS[][3] =
    {
      { 202,   0,   0 },
      { 236, 118,   0 },
      { 236, 188,   0 },
      { 164, 193,   0 },
      { 78,  204,   0 },
      { 0,   205,  68 },
      { 0,   221, 221 },
      { 0,   143, 239 },
      { 56,   56, 255 },
      { 169,   2, 240 },
      { 248,   0, 170 }
    };

    int paletteIdx = colorIndex % RI_LOGPLOT_CURVECOLORSCOUNT;

    cvf::Color3ub ubColor(RI_LOGPLOT_CURVECOLORS[paletteIdx][0], RI_LOGPLOT_CURVECOLORS[paletteIdx][1], RI_LOGPLOT_CURVECOLORS[paletteIdx][2]);
    cvf::Color3f cvfColor(ubColor);
    return cvfColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPlotCurve::LineStyleEnum RimSummaryCurveAppearanceCalculator::cycledLineStyle(int index)
{
    return caf::AppEnum<RimPlotCurve::LineStyleEnum>::fromIndex(1 + (index % (caf::AppEnum<RimPlotCurve::LineStyleEnum>::size() - 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPlotCurve::PointSymbolEnum RimSummaryCurveAppearanceCalculator::cycledSymbol(int index)
{
    return caf::AppEnum<RimPlotCurve::PointSymbolEnum>::fromIndex(1 + (index % (caf::AppEnum<RimPlotCurve::PointSymbolEnum>::size() - 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimSummaryCurveAppearanceCalculator::cycledLineThickness(int index)
{
    static const int thicknessCount = 3;
    static const int thicknesses[] ={ 1, 3, 5 };

    return (thicknesses[(index) % 3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimSummaryCurveAppearanceCalculator::gradient(size_t totalCount, int index)
{
    if(totalCount == 1) return 0.0f;

    const float darkLimit = -0.45f;
    const float lightLimit = 0.7f;
    float totalSpan = lightLimit - darkLimit;
    float step = totalSpan / (totalCount -1);
    return darkLimit + (index * step);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::gradeColor(const cvf::Color3f& color, float factor)
{
    CVF_ASSERT(-1.0 <= factor && factor <= 1.0);

    cvf::Vec3f orgC(color.r(), color.g(), color.b());
    cvf::Vec3f targetC;
    if(factor < 0)
    {
        targetC = cvf::Vec3f(0, 0, 0);
    }
    else
    {
        targetC = cvf::Vec3f(1, 1, 1);
    }

    cvf::Vec3f newColor = ((float)fabs(factor)) * (targetC - orgC) + orgC;

    return cvf::Color3f(newColor[0], newColor[1], newColor[2]);
}
