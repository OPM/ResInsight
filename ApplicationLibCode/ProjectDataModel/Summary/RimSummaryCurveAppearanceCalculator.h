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

#include "RiuPlotCurveSymbol.h"

#include "cvfColor3.h"

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

class RimSummaryCurve;
class RimSummaryCase;
class RifEclipseSummaryAddress;
class RiaSummaryCurveDefinition;

class RimSummaryCurveAppearanceCalculator
{
public:
    explicit RimSummaryCurveAppearanceCalculator( const std::set<RiaSummaryCurveDefinition>& curveDefinitions );
    explicit RimSummaryCurveAppearanceCalculator( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    enum CurveAppearanceType
    {
        NONE,
        COLOR,
        SYMBOL,
        LINE_STYLE,
        GRADIENT,
        LINE_THICKNESS
    };

    void assignDimensions( CurveAppearanceType caseAppearance,
                           CurveAppearanceType variAppearance,
                           CurveAppearanceType wellAppearance,
                           CurveAppearanceType gropAppearance,
                           CurveAppearanceType regiAppearance );
    void getDimensions( CurveAppearanceType* caseAppearance,
                        CurveAppearanceType* variAppearance,
                        CurveAppearanceType* wellAppearance,
                        CurveAppearanceType* gropAppearance,
                        CurveAppearanceType* regiAppearance ) const;

    void setupCurveLook( RimSummaryCurve* curve );

    static cvf::Color3f assignColorByPhase( const RifEclipseSummaryAddress& address );
    static cvf::Color3f computeTintedCurveColorForAddress( const RifEclipseSummaryAddress& address, int colorIndex );

    static cvf::Color3f                        cycledPaletteColor( int colorIndex );
    static cvf::Color3f                        cycledNoneRGBBrColor( int colorIndex );
    static cvf::Color3f                        cycledGreenColor( int colorIndex );
    static cvf::Color3f                        cycledBlueColor( int colorIndex );
    static cvf::Color3f                        cycledRedColor( int colorIndex );
    static cvf::Color3f                        cycledBrownColor( int colorIndex );
    static RiuPlotCurveSymbol::PointSymbolEnum cycledSymbol( int index );

private:
    void init( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    void setOneCurveAppearance( CurveAppearanceType appeaType, size_t totalCount, int appeaIdx, RimSummaryCurve* curve );
    void updateApperanceIndices();
    std::map<std::string, size_t> mapNameToAppearanceIndex( CurveAppearanceType& appearance, const std::set<std::string>& names );

    int   cycledLineThickness( int index );
    float gradient( size_t totalCount, int index );

    cvf::Color3f gradeColor( const cvf::Color3f& color, float factor );
    void         assignColorByPhase( RimSummaryCurve* curve, int colorIndex );

    static std::set<std::string> getAllSummaryCaseNames();
    static std::set<std::string> getAllSummaryWellNames();

    cvf::Color3f m_currentCurveBaseColor;
    float        m_currentCurveGradient;

    int m_dimensionCount;

    CurveAppearanceType m_caseAppearanceType;
    CurveAppearanceType m_varAppearanceType;
    CurveAppearanceType m_wellAppearanceType;
    CurveAppearanceType m_groupAppearanceType;
    CurveAppearanceType m_regionAppearanceType;

    std::map<RimSummaryCase*, int>       m_caseToAppearanceIdxMap;
    std::unordered_map<std::string, int> m_varToAppearanceIdxMap;
    std::unordered_map<std::string, int> m_welToAppearanceIdxMap;
    std::unordered_map<std::string, int> m_grpToAppearanceIdxMap;
    std::unordered_map<int, int>         m_regToAppearanceIdxMap;

    std::map<char, std::map<std::string, int>> m_secondCharToVarToAppearanceIdxMap;

    std::set<std::string> m_allSummaryCaseNames;
    std::set<std::string> m_allSummaryWellNames;
};
