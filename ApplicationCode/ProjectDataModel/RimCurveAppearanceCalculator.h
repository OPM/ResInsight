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
#pragma once
#include "cvfColor3.h"
#include <set>
#include "RifEclipseSummaryAddress.h"
#include "RimPlotCurve.h"

class RimSummaryCurve;
class RimSummaryCase;

class RimCurveLookCalculator
{
public:
#if 0
    RimCurveLookCalculator(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions)
    {
        
        std::set<RimSummaryCase*> casSet;
        std::set<std::string>     varSet;
        std::set<std::string>     welSet;
        std::set<std::string>     grpSet;
        std::set<int>             regSet;

        for (const std::pair<RimSummaryCase*, RifEclipseSummaryAddress>& curveDef : curveDefinitions)
        {
            if (curveDef.first)                             casSet.insert(curveDef.first);
            if (!curveDef.second.quantityName().empty())    varSet.insert(curveDef.second.quantityName());
            if (!curveDef.second.wellName().empty())        welSet.insert(curveDef.second.wellName());
            if (!curveDef.second.wellGroupName().empty())   grpSet.insert(curveDef.second.wellGroupName());
            if (!(curveDef.second.regionNumber() == -1))    regSet.insert(curveDef.second.regionNumber());
        }

        m_caseCount =       casSet.size();
        m_variableCount =   varSet.size();
        m_wellCount =       welSet.size();
        m_groupCount =      grpSet.size();
        m_regionCount =     regSet.size();

        prevCase = nullptr;
        colorIndex = 0;
        lineStyleIdx = -1;
    }
#endif

    RimCurveLookCalculator(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions);

    #if 0
    void setupCurveLook2(RimSummaryCurve* curve)
    {
        RimPlotCurve::LineStyleEnum  lineStyle = RimPlotCurve::STYLE_SOLID;

        if(curve->summaryCase() != prevCase)
        {
            prevCase = curve->summaryCase();
            lineStyleIdx++;
        }

        lineStyle = caf::AppEnum<RimPlotCurve::LineStyleEnum>::fromIndex(lineStyleIdx%caf::AppEnum<RimPlotCurve::LineStyleEnum>::size());
        if(lineStyle == RimPlotCurve::STYLE_NONE)
        {
            lineStyle = RimPlotCurve::STYLE_SOLID;
            lineStyleIdx++;
        }

        cvf::Color3f curveColor = cycledPaletteColor(colorIndex);
        colorIndex++;

        curve->setColor(curveColor);
        curve->setLineStyle(lineStyle);
    }
    #endif

    void setupCurveLook(RimSummaryCurve* curve);

    //--------------------------------------------------------------------------------------------------
    /// Pick default curve color from an index based palette
    //--------------------------------------------------------------------------------------------------
    cvf::Color3f cycledPaletteColor(int colorIndex);

    RimPlotCurve::LineStyleEnum cycledLineStyle(int index);

    RimPlotCurve::PointSymbolEnum cycledSymbol(int index);

    int cycledLineThickness(int index);

    float gradient(size_t totalCount, int index);

private:
    int colorIndex;
    RimSummaryCase* prevCase;
    int lineStyleIdx;

    enum CurveAppearanceType
    {
        NONE,
        COLOR, 
        GRADIENT,
        LINE_STYLE,
        SYMBOL,
        LINE_THICKNESS
    };

    void setOneCurveAppearance(CurveAppearanceType appeaType, size_t totalCount, int appeaIdx, RimSummaryCurve* curve);
    
    cvf::Color3f gradeColor(const cvf::Color3f& color , float factor);

    cvf::Color3f                               m_currentCurveBaseColor;
    float                                      m_currentCurveGradient;

    size_t                                     m_caseCount;
    size_t                                     m_variableCount;
    size_t                                     m_wellCount;
    size_t                                     m_groupCount;
    size_t                                     m_regionCount;

    CurveAppearanceType                        m_caseAppearanceType;
    CurveAppearanceType                        m_varAppearanceType;
    CurveAppearanceType                        m_wellAppearanceType;
    CurveAppearanceType                        m_groupAppearanceType;
    CurveAppearanceType                        m_regionAppearanceType;

    std::map<RimSummaryCase*, int>             m_caseToAppearanceIdxMap;
    std::map<std::string    , int>             m_varToAppearanceIdxMap;
    std::map<std::string    , int>             m_welToAppearanceIdxMap;
    std::map<std::string    , int>             m_grpToAppearanceIdxMap;
    std::map<int            , int>             m_regToAppearanceIdxMap;

    #if 0
    std::vector<cvf::Color3f>                  m_colorsPrCurveDimension;
    std::vector<float>                         m_gradientPrCurveDimension;
    std::vector<RimPlotCurve::LineStyleEnum>   m_lineStylePrCurveDimension;
    std::vector<RimPlotCurve::PointSymbolEnum> m_symbolPrCurveDimension;
    std::vector<int>                           m_lineThicknessPrCurveDimension;
    #endif
};



