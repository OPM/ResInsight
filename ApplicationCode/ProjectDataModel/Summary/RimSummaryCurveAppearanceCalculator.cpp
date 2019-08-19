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

#include "RiaColorTables.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaApplication.h"

#include "RiuQwtPlotCurve.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCase.h"
#include "RimProject.h"

#include "RifSummaryReaderInterface.h"

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
bool isExcplicitHandled(char secondChar)
{
    if (secondChar == 'W' || secondChar == 'O' || secondChar == 'G' || secondChar == 'V')
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveAppearanceCalculator::RimSummaryCurveAppearanceCalculator(const std::set<RiaSummaryCurveDefinition>& curveDefinitions)
{
    m_allSummaryCaseNames = getAllSummaryCaseNames();
    m_allSummaryWellNames = getAllSummaryWellNames();

    for(const RiaSummaryCurveDefinition& curveDef : curveDefinitions)
    {
        if(curveDef.summaryCase())                              m_caseToAppearanceIdxMap[curveDef.summaryCase()]                    = -1;
        if(!curveDef.summaryAddress().wellName().empty())       m_welToAppearanceIdxMap[curveDef.summaryAddress().wellName()]       = -1;
        if(!curveDef.summaryAddress().wellGroupName().empty())  m_grpToAppearanceIdxMap[curveDef.summaryAddress().wellGroupName()]  = -1;
        if(!(curveDef.summaryAddress().regionNumber() == -1))   m_regToAppearanceIdxMap[curveDef.summaryAddress().regionNumber()]   = -1;

        if(!curveDef.summaryAddress().quantityName().empty())
        {
            std::string varname = curveDef.summaryAddress().quantityName();
            m_varToAppearanceIdxMap[varname]  = -1;

            // Indexes for sub color ranges
            char secondChar = 0;
            if(varname.size() > 1)
            {
                secondChar = varname[1];
                if (!isExcplicitHandled(secondChar))
                {
                    secondChar = 0; // Consider all others as one group for coloring
                }
            }
            m_secondCharToVarToAppearanceIdxMap[secondChar][varname] = -1;
        }
    }

    // Select the default appearance type for each data "dimension"
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
    m_currentCurveGradient = 0.0f;

    m_dimensionCount = 0;
    if(m_varToAppearanceIdxMap.size() > 1) { m_varAppearanceType    = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_caseToAppearanceIdxMap.size() > 1) { m_caseAppearanceType   = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_welToAppearanceIdxMap.size() > 1) { m_wellAppearanceType   = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_grpToAppearanceIdxMap.size() > 1) { m_groupAppearanceType  = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }
    if(m_regToAppearanceIdxMap.size() > 1) { m_regionAppearanceType = *(unusedAppearTypes.begin()); unusedAppearTypes.erase(unusedAppearTypes.begin()); m_dimensionCount++; }

    if (m_dimensionCount == 0) m_varAppearanceType = COLOR; // basically one curve
    

    updateApperanceIndices();
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
    m_caseAppearanceType   = caseAppearance;
    m_varAppearanceType    = variAppearance;
    m_wellAppearanceType   = wellAppearance;
    m_groupAppearanceType  = gropAppearance;
    m_regionAppearanceType = regiAppearance;

    // Update the dimensionCount
    m_dimensionCount = 0;
    if(m_caseAppearanceType   != NONE) ++m_dimensionCount;
    if(m_varAppearanceType    != NONE) ++m_dimensionCount;
    if(m_wellAppearanceType   != NONE) ++m_dimensionCount;
    if(m_groupAppearanceType  != NONE) ++m_dimensionCount;
    if(m_regionAppearanceType != NONE) ++m_dimensionCount;

    updateApperanceIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAppearanceCalculator::updateApperanceIndices()
{
    {
        std::map<std::string, size_t> caseAppearanceIndices = mapNameToAppearanceIndex(m_caseAppearanceType, m_allSummaryCaseNames);
        for (auto& pair : m_caseToAppearanceIdxMap)
        {
            pair.second = static_cast<int>(caseAppearanceIndices[pair.first->summaryHeaderFilename().toUtf8().constData()]);
        }
    }
    {
        std::map<std::string, size_t> wellAppearanceIndices = mapNameToAppearanceIndex(m_wellAppearanceType, m_allSummaryWellNames);
        for (auto& pair : m_welToAppearanceIdxMap)
        {
            pair.second = static_cast<int>(wellAppearanceIndices[pair.first]);
        }
    }
    // Assign increasing indexes             
    { int idx = 0; for(auto& pair : m_varToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_grpToAppearanceIdxMap) pair.second = idx++; }
    { int idx = 0; for(auto& pair : m_regToAppearanceIdxMap) pair.second = idx++; }

    for (auto& charMapPair : m_secondCharToVarToAppearanceIdxMap)
    {
        int idx = 0;
        for (auto& pair : charMapPair.second) pair.second = idx++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, size_t> RimSummaryCurveAppearanceCalculator::mapNameToAppearanceIndex(CurveAppearanceType& appearance, const std::set<std::string>& names)
{
    std::map<std::string, size_t> nameToIndex;
    size_t numOptions;
    if (appearance == CurveAppearanceType::COLOR)
    {
        numOptions = RiaColorTables::summaryCurveDefaultPaletteColors().size();
    }
    else if (appearance == CurveAppearanceType::SYMBOL)
    {
        numOptions = caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>::size() - 1; // -1 since the No symbol option is not counted see cycledSymbol()
    }
    else if (appearance == CurveAppearanceType::LINE_STYLE)
    {
        numOptions = caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum>::size() - 1; // -1 since the No symbol option is not counted see cycledLineStyle()
    }
    else {
        // If none of these styles are used, fall back to a simply incrementing index
        size_t idx = 0;
        for (const std::string& name : names)
        {
            nameToIndex[name] = idx;
            ++idx;
        }
        return nameToIndex;
    }

    std::hash<std::string> stringHasher;
    std::vector< std::set<std::string> > nameIndices;
    for (const std::string& name : names)
    {
        size_t nameHash = stringHasher(name);
        nameHash = nameHash % numOptions;

        size_t index = nameHash;
        while (true)
        {
            while (nameIndices.size() <= index)
            {
                // Ensure there exists a set at the insertion index
                nameIndices.push_back(std::set<std::string>());
            }

            std::set<std::string>& matches = nameIndices[index];
            if (matches.empty())
            {
                // If there are no matches here, the summary case has not been added.
                matches.insert(name);
                break;
            }
            else if (matches.find(name) != matches.end())
            {
                // Check to see if the summary case exists at this index.
                break;
            }
            else
            {
                // Simply increment index to check if the next bucket is available.
                index = (index + 1) % numOptions;
                if (index == nameHash)
                {
                    // If we've reached `caseHash` again, no other slot was available, so add it here.
                    matches.insert(name);
                    break;
                }
            }
        }
    }

    size_t index = 0;
    for (std::set<std::string>& nameIndex : nameIndices)
    {
        for (const std::string& name : nameIndex)
        {
            nameToIndex[name] = index;
        }
        index++;
    }

    return nameToIndex;
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
    // The gradient is from negative to positive.
    // Choose default base color as the midpoint between black and white.
    m_currentCurveBaseColor = cvf::Color3f(0.5f, 0.5f, 0.5f);
    m_currentCurveGradient = 0.0f;

    int caseAppearanceIdx = m_caseToAppearanceIdxMap[curve->summaryCaseY()];
    int varAppearanceIdx = m_varToAppearanceIdxMap[curve->summaryAddressY().quantityName()];
    int welAppearanceIdx = m_welToAppearanceIdxMap[curve->summaryAddressY().wellName()];
    int grpAppearanceIdx = m_grpToAppearanceIdxMap[curve->summaryAddressY().wellGroupName()];
    int regAppearanceIdx = m_regToAppearanceIdxMap[curve->summaryAddressY().regionNumber()];

    // Remove index for curves without value at the specific dimension
    if(curve->summaryAddressY().wellName().empty())  welAppearanceIdx = -1;
    if(curve->summaryAddressY().wellGroupName().empty())  grpAppearanceIdx = -1;
    if(curve->summaryAddressY().regionNumber() < 0)  regAppearanceIdx = -1;

    setOneCurveAppearance(m_caseAppearanceType, m_allSummaryCaseNames.size(), caseAppearanceIdx, curve);
    setOneCurveAppearance(m_wellAppearanceType, m_allSummaryWellNames.size(), welAppearanceIdx, curve);
    setOneCurveAppearance(m_groupAppearanceType, m_grpToAppearanceIdxMap.size(), grpAppearanceIdx, curve);
    setOneCurveAppearance(m_regionAppearanceType, m_regToAppearanceIdxMap.size(), regAppearanceIdx, curve);

    if (m_varAppearanceType == COLOR && m_secondCharToVarToAppearanceIdxMap.size() > 1)
    { 
        int subColorIndex = -1;
        char secondChar = 0;
        std::string varname = curve->summaryAddressY().quantityName();
        if (varname.size() > 1)
        {
            secondChar = varname[1];
            if (!isExcplicitHandled(secondChar))
            {
                secondChar = 0; // Consider all others as one group for coloring
            }
        }

        subColorIndex = m_secondCharToVarToAppearanceIdxMap[secondChar][varname];
        
        if (secondChar == 'W')
        {
            // Pick blue
            m_currentCurveBaseColor = cycledBlueColor(subColorIndex);
        }
        else if (secondChar == 'O')
        {
            // Pick Green
            m_currentCurveBaseColor = cycledGreenColor(subColorIndex);
        }
        else if (secondChar == 'G')
        {
            // Pick Red
            m_currentCurveBaseColor = cycledRedColor(subColorIndex);
        }
        else if(secondChar == 'V')
        {
            // Pick Brown
            m_currentCurveBaseColor = cycledBrownColor(subColorIndex);
        }
        else
        {
            m_currentCurveBaseColor = cycledNoneRGBBrColor(subColorIndex);
        }
    }
    else
    {
        setOneCurveAppearance(m_varAppearanceType, m_varToAppearanceIdxMap.size(), varAppearanceIdx, curve);
    }
    
    curve->setColor(gradeColor(m_currentCurveBaseColor, m_currentCurveGradient));

    curve->setCurveAppearanceFromCaseType();
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
    if (colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(colorIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(int colorIndex)
{
    if(colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveNoneRedGreenBlueBrownPaletteColors().cycledColor3f(colorIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledGreenColor(int colorIndex)
{
    if(colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveGreenPaletteColors().cycledColor3f(colorIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledBlueColor(int colorIndex)
{
    if(colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveBluePaletteColors().cycledColor3f(colorIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledRedColor(int colorIndex)
{
    if(colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveRedPaletteColors().cycledColor3f(colorIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSummaryCurveAppearanceCalculator::cycledBrownColor(int colorIndex)
{
    if(colorIndex < 0) return cvf::Color3f::BLACK;

    return RiaColorTables::summaryCurveBrownPaletteColors().cycledColor3f(colorIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurve::LineStyleEnum RimSummaryCurveAppearanceCalculator::cycledLineStyle(int index)
{
    if (index < 0) return RiuQwtPlotCurve::STYLE_SOLID;

    return caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum>::fromIndex(1 + (index % (caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum>::size() - 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RimSummaryCurveAppearanceCalculator::cycledSymbol(int index)
{
    if (index < 0) return RiuQwtSymbol::SYMBOL_NONE;

    return caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>::fromIndex(1 + (index % (caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>::size() - 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimSummaryCurveAppearanceCalculator::cycledLineThickness(int index)
{
    static const int thicknessCount = 3;
    static const int thicknesses[] ={ 1, 3, 5 };

    if (index < 0) return 1;
    return (thicknesses[(index) % thicknessCount]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimSummaryCurveAppearanceCalculator::gradient(size_t totalCount, int index)
{
    if(totalCount == 1 || index < 0) return 0.0f;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RimSummaryCurveAppearanceCalculator::getAllSummaryCaseNames()
{
    std::set<std::string> summaryCaseHashes;
    RimProject*           proj = RiaApplication::instance()->project();

    std::vector<RimSummaryCase*> cases = proj->allSummaryCases();
    for (RimSummaryCase* rimCase : cases)
    {
        summaryCaseHashes.insert(rimCase->summaryHeaderFilename().toUtf8().constData());
    }

    return summaryCaseHashes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RimSummaryCurveAppearanceCalculator::getAllSummaryWellNames()
{
    std::set<std::string> summaryWellNames;
    RimProject*           proj = RiaApplication::instance()->project();

    std::vector<RimSummaryCase*> cases = proj->allSummaryCases();
    for (RimSummaryCase* rimCase : cases)
    {
        RifSummaryReaderInterface* reader = nullptr;
        if (rimCase)
        {
            reader = rimCase->summaryReader();
        }

        if (reader)
        {
            const std::set<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

            for (auto& address : allAddresses)
            {
                if (address.category() == RifEclipseSummaryAddress::SUMMARY_WELL)
                {
                    summaryWellNames.insert(address.wellName());
                }
            }
        }
    }
    return summaryWellNames;
}
