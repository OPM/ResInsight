/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RiuSummaryVectorDescriptionMap.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorDescriptionMap* RiuSummaryVectorDescriptionMap::instance()
{
    static RiuSummaryVectorDescriptionMap* singleton = new RiuSummaryVectorDescriptionMap;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorInfo RiuSummaryVectorDescriptionMap::vectorInfo( const std::string& vectorName )
{
    std::map<std::string, RiuSummaryVectorInfo>::iterator it = m_summaryToDescMap.find( vectorName );

    if ( it != m_summaryToDescMap.end() )
    {
        return it->second;
    }
    else if ( vectorName.size() > 1 && vectorName[1] == 'U' )
    {
        // User defined vector name

        std::string key = vectorName.substr( 0, 2 );

        it = m_summaryToDescMap.find( key );

        if ( it != m_summaryToDescMap.end() )
        {
            return it->second;
        }
    }
    else if ( vectorName.size() > 5 )
    {
        // Check for custom vector naming

        std::string baseName = vectorName.substr( 0, 5 );
        while ( baseName.back() == '_' )
            baseName.pop_back();

        it = m_summaryToDescMap.find( baseName );

        if ( it != m_summaryToDescMap.end() )
        {
            return it->second;
        }
    }

    return RiuSummaryVectorInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiuSummaryVectorDescriptionMap::vectorLongName( const std::string& vectorName,
                                                            bool               returnVectorNameIfNotFound )
{
    auto info = vectorInfo( vectorName );
    return info.category != RifEclipseSummaryAddress::SUMMARY_INVALID || !returnVectorNameIfNotFound ? info.longName
                                                                                                     : vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorDescriptionMap::RiuSummaryVectorDescriptionMap()
{
    populateFieldToInfoMap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorDescriptionMap::populateFieldToInfoMap()
{
    using A = RifEclipseSummaryAddress;

    m_summaryToDescMap.insert( {"FOPR", {A::SUMMARY_FIELD, "Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"FOPRA", {A::SUMMARY_FIELD, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"FOPRB", {A::SUMMARY_FIELD, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"FOPTA", {A::SUMMARY_FIELD, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"FOPTB", {A::SUMMARY_FIELD, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"FOPR1", {A::SUMMARY_FIELD, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"FOPR2", {A::SUMMARY_FIELD, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"FOPT1", {A::SUMMARY_FIELD, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"FOPT2", {A::SUMMARY_FIELD, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"FOMR", {A::SUMMARY_FIELD, "Oil Mass Rate"}} );
    m_summaryToDescMap.insert( {"FOMT", {A::SUMMARY_FIELD, "Oil Mass Total"}} );
    m_summaryToDescMap.insert( {"FODN", {A::SUMMARY_FIELD, "Oil Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"FOPRH", {A::SUMMARY_FIELD, "Oil Production Rate History"}} );
    m_summaryToDescMap.insert( {"FOPRT", {A::SUMMARY_FIELD, "Oil Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FOPRF", {A::SUMMARY_FIELD, "Free Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"FOPRS", {A::SUMMARY_FIELD, "Solution Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"FOPT", {A::SUMMARY_FIELD, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"FOPTH", {A::SUMMARY_FIELD, "Oil Production Total History"}} );
    m_summaryToDescMap.insert( {"FOPTF", {A::SUMMARY_FIELD, "Free Oil Production Total"}} );
    m_summaryToDescMap.insert( {"FOPTS", {A::SUMMARY_FIELD, "Solution Oil Production Total"}} );
    m_summaryToDescMap.insert( {"FOIR", {A::SUMMARY_FIELD, "Oil Injection Rate"}} );
    m_summaryToDescMap.insert( {"FOIRH", {A::SUMMARY_FIELD, "Oil Injection Rate History"}} );
    m_summaryToDescMap.insert( {"FOIRT", {A::SUMMARY_FIELD, "Oil Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FOIT", {A::SUMMARY_FIELD, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"FOITH", {A::SUMMARY_FIELD, "Oil Injection Total History"}} );
    m_summaryToDescMap.insert( {"FOPP", {A::SUMMARY_FIELD, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FOPP2", {A::SUMMARY_FIELD, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FOPI", {A::SUMMARY_FIELD, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FOPI2", {A::SUMMARY_FIELD, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FOVPR", {A::SUMMARY_FIELD, "Oil Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"FOVPT", {A::SUMMARY_FIELD, "Oil Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"FOVIR", {A::SUMMARY_FIELD, "Oil Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"FOVIT", {A::SUMMARY_FIELD, "Oil Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"FOnPR", {A::SUMMARY_FIELD, "nth separator stage oil rate"}} );
    m_summaryToDescMap.insert( {"FOnPT", {A::SUMMARY_FIELD, "nth separator stage oil total"}} );
    m_summaryToDescMap.insert( {"FEOR", {A::SUMMARY_FIELD, "Export Oil Rate"}} );
    m_summaryToDescMap.insert( {"FEOT", {A::SUMMARY_FIELD, "Export Oil Total"}} );
    m_summaryToDescMap.insert( {"FEOMF", {A::SUMMARY_FIELD, "Export Oil Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FWPR", {A::SUMMARY_FIELD, "Water Production Rate"}} );
    m_summaryToDescMap.insert( {"FWMR", {A::SUMMARY_FIELD, "Water Mass Rate"}} );
    m_summaryToDescMap.insert( {"FWMT", {A::SUMMARY_FIELD, "Water Mass Total"}} );
    m_summaryToDescMap.insert( {"FWPRH", {A::SUMMARY_FIELD, "Water Production Rate History"}} );
    m_summaryToDescMap.insert( {"FWPRT", {A::SUMMARY_FIELD, "Water Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FWPT", {A::SUMMARY_FIELD, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"FWPTH", {A::SUMMARY_FIELD, "Water Production Total History"}} );
    m_summaryToDescMap.insert( {"FWIR", {A::SUMMARY_FIELD, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"FWIRH", {A::SUMMARY_FIELD, "Water Injection Rate History"}} );
    m_summaryToDescMap.insert( {"FWIRT", {A::SUMMARY_FIELD, "Water Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FWIT", {A::SUMMARY_FIELD, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"FWITH", {A::SUMMARY_FIELD, "Water Injection Total History"}} );
    m_summaryToDescMap.insert( {"FWPP", {A::SUMMARY_FIELD, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FWPP2", {A::SUMMARY_FIELD, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FWPI", {A::SUMMARY_FIELD, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FWPI2", {A::SUMMARY_FIELD, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FWVPR", {A::SUMMARY_FIELD, "Water Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"FWVPT", {A::SUMMARY_FIELD, "Water Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"FWVIR", {A::SUMMARY_FIELD, "Water Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"FWVIT", {A::SUMMARY_FIELD, "Water Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"FWPIR", {A::SUMMARY_FIELD, "Ratio of produced water to injected water (percentage)"}} );
    m_summaryToDescMap.insert( {"FWMPR", {A::SUMMARY_FIELD, "Water component molar production rate"}} );
    m_summaryToDescMap.insert( {"FWMPT", {A::SUMMARY_FIELD, "Water component molar production total"}} );
    m_summaryToDescMap.insert( {"FWMIR", {A::SUMMARY_FIELD, "Water component molar injection rate"}} );
    m_summaryToDescMap.insert( {"FWMIT", {A::SUMMARY_FIELD, "Water component molar injection total"}} );
    m_summaryToDescMap.insert( {"FGPR", {A::SUMMARY_FIELD, "Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"FGPRA", {A::SUMMARY_FIELD, "Gas Production Rate above"}} );
    m_summaryToDescMap.insert( {"FGPRB", {A::SUMMARY_FIELD, "Gas Production Rate below"}} );
    m_summaryToDescMap.insert( {"FGPTA", {A::SUMMARY_FIELD, "Gas Production Total above"}} );
    m_summaryToDescMap.insert( {"FGPTB", {A::SUMMARY_FIELD, "Gas Production Total below"}} );
    m_summaryToDescMap.insert( {"FGPR1", {A::SUMMARY_FIELD, "Gas Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"FGPR2", {A::SUMMARY_FIELD, "Gas Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"FGPT1", {A::SUMMARY_FIELD, "Gas Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"FGPT2", {A::SUMMARY_FIELD, "Gas Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"FGMR", {A::SUMMARY_FIELD, "Gas Mass Rate"}} );
    m_summaryToDescMap.insert( {"FGMT", {A::SUMMARY_FIELD, "Gas Mass Total"}} );
    m_summaryToDescMap.insert( {"FGDN", {A::SUMMARY_FIELD, "Gas Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"FGPRH", {A::SUMMARY_FIELD, "Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"FGPRT", {A::SUMMARY_FIELD, "Gas Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FGPRF", {A::SUMMARY_FIELD, "Free Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"FGPRS", {A::SUMMARY_FIELD, "Solution Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"FGPT", {A::SUMMARY_FIELD, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"FGPTH", {A::SUMMARY_FIELD, "Gas Production Total History"}} );
    m_summaryToDescMap.insert( {"FGPTF", {A::SUMMARY_FIELD, "Free Gas Production Total"}} );
    m_summaryToDescMap.insert( {"FGPTS", {A::SUMMARY_FIELD, "Solution Gas Production Total"}} );
    m_summaryToDescMap.insert( {"FGIR", {A::SUMMARY_FIELD, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"FGIRH", {A::SUMMARY_FIELD, "Gas Injection Rate History"}} );
    m_summaryToDescMap.insert( {"FGIRT", {A::SUMMARY_FIELD, "Gas Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FGIT", {A::SUMMARY_FIELD, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"FGITH", {A::SUMMARY_FIELD, "Gas Injection Total History"}} );
    m_summaryToDescMap.insert( {"FGPP", {A::SUMMARY_FIELD, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FGPP2", {A::SUMMARY_FIELD, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FGPPS", {A::SUMMARY_FIELD, "Solution"}} );
    m_summaryToDescMap.insert( {"FGPPS2", {A::SUMMARY_FIELD, "Solution"}} );
    m_summaryToDescMap.insert( {"FGPPF", {A::SUMMARY_FIELD, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FGPPF2", {A::SUMMARY_FIELD, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"FGPI", {A::SUMMARY_FIELD, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FGPI2", {A::SUMMARY_FIELD, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"FSGR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"FGSR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"FSGT", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"FGST", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"FSMF", {A::SUMMARY_FIELD, "Sales Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FFGR", {A::SUMMARY_FIELD, "Fuel Gas Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FFGT", {A::SUMMARY_FIELD, "Fuel Gas cumulative Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FFMF", {A::SUMMARY_FIELD, "Fuel Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FGCR", {A::SUMMARY_FIELD, "Gas Consumption Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGCT", {A::SUMMARY_FIELD, "Gas Consumption Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGIMR", {A::SUMMARY_FIELD, "Gas Import Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGIMT", {A::SUMMARY_FIELD, "Gas Import Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGLIR", {A::SUMMARY_FIELD, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"FWGPR", {A::SUMMARY_FIELD, "Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"FWGPT", {A::SUMMARY_FIELD, "Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"FWGPRH", {A::SUMMARY_FIELD, "Wet Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"FWGIR", {A::SUMMARY_FIELD, "Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"FWGIT", {A::SUMMARY_FIELD, "Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"FEGR", {A::SUMMARY_FIELD, "Export Gas Rate"}} );
    m_summaryToDescMap.insert( {"FEGT", {A::SUMMARY_FIELD, "Export Gas Total"}} );
    m_summaryToDescMap.insert( {"FEMF", {A::SUMMARY_FIELD, "Export Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FEXGR", {A::SUMMARY_FIELD, "Excess Gas Rate"}} );
    m_summaryToDescMap.insert( {"FEXGT", {A::SUMMARY_FIELD, "Excess Gas Total"}} );
    m_summaryToDescMap.insert( {"FRGR", {A::SUMMARY_FIELD, "Re-injection Gas Rate"}} );
    m_summaryToDescMap.insert( {"FRGT", {A::SUMMARY_FIELD, "Re-injection Gas Total"}} );
    m_summaryToDescMap.insert( {"FGnPR", {A::SUMMARY_FIELD, "nth separator stage gas rate"}} );
    m_summaryToDescMap.insert( {"FGnPT", {A::SUMMARY_FIELD, "nth separator stage gas total"}} );
    m_summaryToDescMap.insert( {"FGVPR", {A::SUMMARY_FIELD, "Gas Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"FGVPT", {A::SUMMARY_FIELD, "Gas Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"FGVIR", {A::SUMMARY_FIELD, "Gas Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"FGVIT", {A::SUMMARY_FIELD, "Gas Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"FGQ", {A::SUMMARY_FIELD, "Gas Quality"}} );
    m_summaryToDescMap.insert( {"FLPR", {A::SUMMARY_FIELD, "Liquid Production Rate"}} );
    m_summaryToDescMap.insert( {"FLPRH", {A::SUMMARY_FIELD, "Liquid Production Rate History"}} );
    m_summaryToDescMap.insert( {"FLPRT", {A::SUMMARY_FIELD, "Liquid Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FLPT", {A::SUMMARY_FIELD, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"FLPTH", {A::SUMMARY_FIELD, "Liquid Production Total History"}} );
    m_summaryToDescMap.insert( {"FVPR", {A::SUMMARY_FIELD, "Res Volume Production Rate"}} );
    m_summaryToDescMap.insert( {"FVPRT", {A::SUMMARY_FIELD, "Res Volume Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FVPT", {A::SUMMARY_FIELD, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"FVIR", {A::SUMMARY_FIELD, "Res Volume Injection Rate"}} );
    m_summaryToDescMap.insert( {"FVIRT", {A::SUMMARY_FIELD, "Res Volume Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"FVIT", {A::SUMMARY_FIELD, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"FWCT", {A::SUMMARY_FIELD, "Water Cut"}} );
    m_summaryToDescMap.insert( {"FWCTH", {A::SUMMARY_FIELD, "Water Cut History"}} );
    m_summaryToDescMap.insert( {"FGOR", {A::SUMMARY_FIELD, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"FGORH", {A::SUMMARY_FIELD, "Gas-Oil Ratio History"}} );
    m_summaryToDescMap.insert( {"FOGR", {A::SUMMARY_FIELD, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"FOGRH", {A::SUMMARY_FIELD, "Oil-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"FWGR", {A::SUMMARY_FIELD, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"FWGRH", {A::SUMMARY_FIELD, "Water-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"FGLR", {A::SUMMARY_FIELD, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"FGLRH", {A::SUMMARY_FIELD, "Gas-Liquid Ratio History"}} );
    m_summaryToDescMap.insert( {"FMCTP", {A::SUMMARY_FIELD, "Mode of Control for group Production"}} );
    m_summaryToDescMap.insert( {"FMCTW", {A::SUMMARY_FIELD, "Mode of Control for group Water Injection"}} );
    m_summaryToDescMap.insert( {"FMCTG", {A::SUMMARY_FIELD, "Mode of Control for group Gas Injection"}} );
    m_summaryToDescMap.insert( {"FMWPT", {A::SUMMARY_FIELD, "Total number of production wells"}} );
    m_summaryToDescMap.insert( {"FMWPR", {A::SUMMARY_FIELD, "Number of production wells currently flowing"}} );
    m_summaryToDescMap.insert( {"FMWPA", {A::SUMMARY_FIELD, "Number of abandoned production wells"}} );
    m_summaryToDescMap.insert( {"FMWPU", {A::SUMMARY_FIELD, "Number of unused production wells"}} );
    m_summaryToDescMap.insert( {"FMWPG", {A::SUMMARY_FIELD, "Number of producers on group control"}} );
    m_summaryToDescMap.insert( {"FMWPO", {A::SUMMARY_FIELD, "Number of producers controlled by own oil rate limit"}} );
    m_summaryToDescMap.insert( {"FMWPS", {A::SUMMARY_FIELD, "Number of producers on own surface rate limit control"}} );
    m_summaryToDescMap.insert(
        {"FMWPV", {A::SUMMARY_FIELD, "Number of producers on own reservoir volume rate limit control"}} );
    m_summaryToDescMap.insert( {"FMWPP", {A::SUMMARY_FIELD, "Number of producers on pressure control"}} );
    m_summaryToDescMap.insert( {"FMWPL", {A::SUMMARY_FIELD, "Number of producers using artificial lift"}} );
    m_summaryToDescMap.insert( {"FMWIT", {A::SUMMARY_FIELD, "Total number of injection wells"}} );
    m_summaryToDescMap.insert( {"FMWIN", {A::SUMMARY_FIELD, "Number of injection wells currently flowing"}} );
    m_summaryToDescMap.insert( {"FMWIA", {A::SUMMARY_FIELD, "Number of abandoned injection wells"}} );
    m_summaryToDescMap.insert( {"FMWIU", {A::SUMMARY_FIELD, "Number of unused injection wells"}} );
    m_summaryToDescMap.insert( {"FMWIG", {A::SUMMARY_FIELD, "Number of injectors on group control"}} );
    m_summaryToDescMap.insert( {"FMWIS", {A::SUMMARY_FIELD, "Number of injectors on own surface rate limit control"}} );
    m_summaryToDescMap.insert(
        {"FMWIV", {A::SUMMARY_FIELD, "Number of injectors on own reservoir volume rate limit control"}} );
    m_summaryToDescMap.insert( {"FMWIP", {A::SUMMARY_FIELD, "Number of injectors on pressure control"}} );
    m_summaryToDescMap.insert( {"FMWDR", {A::SUMMARY_FIELD, "Number of drilling events this timestep"}} );
    m_summaryToDescMap.insert( {"FMWDT", {A::SUMMARY_FIELD, "Number of drilling events in total"}} );
    m_summaryToDescMap.insert( {"FMWWO", {A::SUMMARY_FIELD, "Number of workover events this timestep"}} );
    m_summaryToDescMap.insert( {"FMWWT", {A::SUMMARY_FIELD, "Number of workover events in total"}} );
    m_summaryToDescMap.insert( {"FEPR", {A::SUMMARY_FIELD, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"FEPT", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"FNLPR", {A::SUMMARY_FIELD, "NGL Production Rate"}} );
    m_summaryToDescMap.insert( {"FNLPT", {A::SUMMARY_FIELD, "NGL Production Total"}} );
    m_summaryToDescMap.insert( {"FNLPRH", {A::SUMMARY_FIELD, "NGL Production Rate History"}} );
    m_summaryToDescMap.insert( {"FNLPTH", {A::SUMMARY_FIELD, "NGL Production Total History"}} );
    m_summaryToDescMap.insert(
        {"FAMF", {A::SUMMARY_FIELD, "Component aqueous mole fraction, from producing completions"}} );
    m_summaryToDescMap.insert( {"FXMF", {A::SUMMARY_FIELD, "Liquid Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FYMF", {A::SUMMARY_FIELD, "Vapor Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FXMFn", {A::SUMMARY_FIELD, "Liquid Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"FYMFn", {A::SUMMARY_FIELD, "Vapor Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"FZMF", {A::SUMMARY_FIELD, "Total Mole Fraction"}} );
    m_summaryToDescMap.insert( {"FCMPR", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Production Rates"}} );
    m_summaryToDescMap.insert( {"FCMPT", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"FCMIR", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Injection Rates"}} );
    m_summaryToDescMap.insert( {"FCMIT", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Injection Totals"}} );
    m_summaryToDescMap.insert( {"FHMIR", {A::SUMMARY_FIELD, "Hydrocarbon Molar Injection Rate"}} );
    m_summaryToDescMap.insert( {"FHMIT", {A::SUMMARY_FIELD, "Hydrocarbon Molar Injection Total"}} );
    m_summaryToDescMap.insert( {"FHMPR", {A::SUMMARY_FIELD, "Hydrocarbon Molar Production Rate"}} );
    m_summaryToDescMap.insert( {"FHMPT", {A::SUMMARY_FIELD, "Hydrocarbon Molar Production Total"}} );
    m_summaryToDescMap.insert( {"FCHMR", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"FCHMT", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"FCWGPR", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"FCWGPT", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"FCWGIR", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"FCWGIT", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"FCGMR", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCGMT", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCOMR", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCOMT", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCNMR", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the NGL phase"}} );
    m_summaryToDescMap.insert( {"FCNWR", {A::SUMMARY_FIELD, "Hydrocarbon component mass rates in the NGL phase"}} );
    m_summaryToDescMap.insert(
        {"FCGMRn", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert(
        {"FCGRn", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert( {"FCOMRn", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCORn", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FMUF", {A::SUMMARY_FIELD, "Make-up fraction"}} );
    m_summaryToDescMap.insert( {"FAMR", {A::SUMMARY_FIELD, "Make-up gas rate"}} );
    m_summaryToDescMap.insert( {"FAMT", {A::SUMMARY_FIELD, "Make-up gas total"}} );
    m_summaryToDescMap.insert(
        {"FGSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert(
        {"FGSRL",
         {A::SUMMARY_FIELD, "Maximum tested rate sustained for the test period during the most recent sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert( {"FGSRU",
                                {A::SUMMARY_FIELD,
                                 "Minimum tested rate not sustained for the test period during the most recent "
                                 "sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert( {"FGSSP",
                                {A::SUMMARY_FIELD,
                                 "Period for which target sustainable rate could be maintained for the most recent "
                                 "sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert(
        {"FGSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert(
        {"FOSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert(
        {"FOSRL",
         {A::SUMMARY_FIELD, "Maximum tested rate sustained for the test period during the most recent sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert( {"FOSRU",
                                {A::SUMMARY_FIELD,
                                 "Minimum tested rate not sustained for the test period during the most recent "
                                 "sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert( {"FOSSP",
                                {A::SUMMARY_FIELD,
                                 "Period for which target sustainable rate could be maintained for the most recent "
                                 "sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert(
        {"FOSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert(
        {"FWSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for water"}} );
    m_summaryToDescMap.insert(
        {"FWSRL",
         {A::SUMMARY_FIELD, "Maximum tested rate sustained for the test period during the most recent sustainable capacity test for water"}} );
    m_summaryToDescMap.insert( {"FWSRU",
                                {A::SUMMARY_FIELD,
                                 "Minimum tested rate not sustained for the test period during the most recent "
                                 "sustainable capacity test for water"}} );
    m_summaryToDescMap.insert(
        {"FWSSP",
         {A::SUMMARY_FIELD,
          "Period for which target sustainable rate could be maintained for the most recent sustainable capacity test "
          "for water"}} );
    m_summaryToDescMap.insert(
        {"FWSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for water"}} );
    m_summaryToDescMap.insert( {"FGPRG", {A::SUMMARY_FIELD, "Gas production rate"}} );
    m_summaryToDescMap.insert( {"FOPRG", {A::SUMMARY_FIELD, "Oil production rate"}} );
    m_summaryToDescMap.insert( {"FNLPRG", {A::SUMMARY_FIELD, "NGL production rate"}} );
    m_summaryToDescMap.insert( {"FXMFG", {A::SUMMARY_FIELD, "Liquid mole fraction"}} );
    m_summaryToDescMap.insert( {"FYMFG", {A::SUMMARY_FIELD, "Vapor mole fraction"}} );
    m_summaryToDescMap.insert( {"FCOMRG", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"FCGMRG", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase"}} );
    m_summaryToDescMap.insert( {"FCNMRG", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the NGL phase"}} );
    m_summaryToDescMap.insert( {"FPR", {A::SUMMARY_FIELD, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"FPRH", {A::SUMMARY_FIELD, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"FPRP", {A::SUMMARY_FIELD, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"FPRGZ", {A::SUMMARY_FIELD, "P/Z"}} );
    m_summaryToDescMap.insert( {"FRS", {A::SUMMARY_FIELD, "Gas-oil ratio"}} );
    m_summaryToDescMap.insert( {"FRV", {A::SUMMARY_FIELD, "Oil-gas ratio"}} );
    m_summaryToDescMap.insert( {"FCHIP", {A::SUMMARY_FIELD, "Component Hydrocarbon as Wet Gas"}} );
    m_summaryToDescMap.insert( {"FCMIP", {A::SUMMARY_FIELD, "Component Hydrocarbon as Moles"}} );
    m_summaryToDescMap.insert( {"FPPC", {A::SUMMARY_FIELD, "Initial Contact Corrected Potential"}} );
    m_summaryToDescMap.insert(
        {"FREAC", {A::SUMMARY_FIELD, "Reaction rate. The reaction number is given as a component index"}} );
    m_summaryToDescMap.insert(
        {"FREAT", {A::SUMMARY_FIELD, "Reaction total. The reaction number is given as a component index"}} );
    m_summaryToDescMap.insert( {"FRPV", {A::SUMMARY_FIELD, "Pore Volume at Reservoir conditions"}} );
    m_summaryToDescMap.insert( {"FOPV", {A::SUMMARY_FIELD, "Pore Volume containing Oil"}} );
    m_summaryToDescMap.insert( {"FWPV", {A::SUMMARY_FIELD, "Pore Volume containing Water"}} );
    m_summaryToDescMap.insert( {"FGPV", {A::SUMMARY_FIELD, "Pore Volume containing Gas"}} );
    m_summaryToDescMap.insert( {"FHPV", {A::SUMMARY_FIELD, "Pore Volume containing Hydrocarbon"}} );
    m_summaryToDescMap.insert(
        {"FRTM", {A::SUMMARY_FIELD, "Transmissibility Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert( {"FOE", {A::SUMMARY_FIELD, "(OIP(initial - OIP(now) / OIP(initial)"}} );
    m_summaryToDescMap.insert( {"FOEW", {A::SUMMARY_FIELD, "Oil Production from Wells / OIP(initial)"}} );
    m_summaryToDescMap.insert(
        {"FOEIW", {A::SUMMARY_FIELD, "(OIP(initial - OIP(now) / Initial Mobile Oil with respect to Water"}} );
    m_summaryToDescMap.insert(
        {"FOEWW", {A::SUMMARY_FIELD, "Oil Production from Wells / Initial Mobile Oil with respect to Water"}} );
    m_summaryToDescMap.insert(
        {"FOEIG", {A::SUMMARY_FIELD, "(OIP(initial - OIP(now) / Initial Mobile Oil with respect to Gas"}} );
    m_summaryToDescMap.insert(
        {"FOEWG", {A::SUMMARY_FIELD, "Oil Production from Wells / Initial Mobile Oil with respect to Gas"}} );
    m_summaryToDescMap.insert( {"FORMR", {A::SUMMARY_FIELD, "Total stock tank oil produced by rock compaction"}} );
    m_summaryToDescMap.insert( {"FORMW", {A::SUMMARY_FIELD, "Total stock tank oil produced by water influx"}} );
    m_summaryToDescMap.insert( {"FORMG", {A::SUMMARY_FIELD, "Total stock tank oil produced by gas influx"}} );
    m_summaryToDescMap.insert( {"FORME", {A::SUMMARY_FIELD, "Total stock tank oil produced by oil expansion"}} );
    m_summaryToDescMap.insert( {"FORMS", {A::SUMMARY_FIELD, "Total stock tank oil produced by solution gas"}} );
    m_summaryToDescMap.insert( {"FORMF", {A::SUMMARY_FIELD, "Total stock tank oil produced by free gas influx"}} );
    m_summaryToDescMap.insert( {"FORMX", {A::SUMMARY_FIELD, "Total stock tank oil produced by 'traced' water influx"}} );
    m_summaryToDescMap.insert( {"FORMY", {A::SUMMARY_FIELD, "Total stock tank oil produced by other water influx"}} );
    m_summaryToDescMap.insert( {"FORFR", {A::SUMMARY_FIELD, "Fraction of total oil produced by rock compaction"}} );
    m_summaryToDescMap.insert( {"FORFW", {A::SUMMARY_FIELD, "Fraction of total oil produced by water influx"}} );
    m_summaryToDescMap.insert( {"FORFG", {A::SUMMARY_FIELD, "Fraction of total oil produced by gas influx"}} );
    m_summaryToDescMap.insert( {"FORFE", {A::SUMMARY_FIELD, "Fraction of total oil produced by oil expansion"}} );
    m_summaryToDescMap.insert( {"FORFS", {A::SUMMARY_FIELD, "Fraction of total oil produced by solution gas"}} );
    m_summaryToDescMap.insert( {"FORFF", {A::SUMMARY_FIELD, "Fraction of total oil produced by free gas influx"}} );
    m_summaryToDescMap.insert( {"FORFX", {A::SUMMARY_FIELD, "Fraction of total oil produced by 'traced' water influx"}} );
    m_summaryToDescMap.insert( {"FORFY", {A::SUMMARY_FIELD, "Fraction of total oil produced by other water influx"}} );
    m_summaryToDescMap.insert( {"FAQR", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"FAQT", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"FAQRG", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"FAQTG", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"FAQER", {A::SUMMARY_FIELD, "Aquifer thermal energy influx rate"}} );
    m_summaryToDescMap.insert( {"FAQET", {A::SUMMARY_FIELD, "Cumulative aquifer thermal energy influx"}} );
    m_summaryToDescMap.insert( {"FNQR", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"FNQT", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"FTPR", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPT", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"FTPC", {A::SUMMARY_FIELD, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"FTIR", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTIT", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"FTIC", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FTMR", {A::SUMMARY_FIELD, "Traced mass Rate"}} );
    m_summaryToDescMap.insert( {"FTMT", {A::SUMMARY_FIELD, "Traced mass Total"}} );
    m_summaryToDescMap.insert( {"FTQR", {A::SUMMARY_FIELD, "Traced molar Rate"}} );
    m_summaryToDescMap.insert( {"FTCM", {A::SUMMARY_FIELD, "Tracer Carrier molar Rate"}} );
    m_summaryToDescMap.insert( {"FTMF", {A::SUMMARY_FIELD, "Traced molar fraction"}} );
    m_summaryToDescMap.insert( {"FTVL", {A::SUMMARY_FIELD, "Traced liquid volume rate"}} );
    m_summaryToDescMap.insert( {"FTVV", {A::SUMMARY_FIELD, "Traced vapor volume rate"}} );
    m_summaryToDescMap.insert( {"FTTL", {A::SUMMARY_FIELD, "Traced liquid volume total"}} );
    m_summaryToDescMap.insert( {"FTTV", {A::SUMMARY_FIELD, "Traced vapor volume total"}} );
    m_summaryToDescMap.insert( {"FTML", {A::SUMMARY_FIELD, "Traced mass liquid rate"}} );
    m_summaryToDescMap.insert( {"FTMV", {A::SUMMARY_FIELD, "Traced mass vapor rate"}} );
    m_summaryToDescMap.insert( {"FTLM", {A::SUMMARY_FIELD, "Traced mass liquid total"}} );
    m_summaryToDescMap.insert( {"FTVM", {A::SUMMARY_FIELD, "Traced mass vapor total"}} );
    m_summaryToDescMap.insert( {"FTIPT", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FTIPF", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FTIPS", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FAPI", {A::SUMMARY_FIELD, "Oil API"}} );
    m_summaryToDescMap.insert( {"FSPR", {A::SUMMARY_FIELD, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"FSPT", {A::SUMMARY_FIELD, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"FSIR", {A::SUMMARY_FIELD, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"FSIT", {A::SUMMARY_FIELD, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"FSPC", {A::SUMMARY_FIELD, "Salt Production Concentration"}} );
    m_summaryToDescMap.insert( {"FSIC", {A::SUMMARY_FIELD, "Salt Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FSIP", {A::SUMMARY_FIELD, "Salt In Place"}} );
    m_summaryToDescMap.insert( {"GTPRANI", {A::SUMMARY_FIELD, "Anion Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPTANI", {A::SUMMARY_FIELD, "Anion Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRANI", {A::SUMMARY_FIELD, "Anion Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTITANI", {A::SUMMARY_FIELD, "Anion Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPRCAT", {A::SUMMARY_FIELD, "Cation Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPTCAT", {A::SUMMARY_FIELD, "Cation Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRCAT", {A::SUMMARY_FIELD, "Cation Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTITCAT", {A::SUMMARY_FIELD, "Cation Injection Total"}} );
    m_summaryToDescMap.insert( {"FTPCHEA", {A::SUMMARY_FIELD, "Production Temperature"}} );
    m_summaryToDescMap.insert( {"FTICHEA", {A::SUMMARY_FIELD, "Injection Temperature"}} );
    m_summaryToDescMap.insert( {"FTPRHEA", {A::SUMMARY_FIELD, "Energy flows"}} );
    m_summaryToDescMap.insert( {"FTPTHEA", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"FTIRHEA", {A::SUMMARY_FIELD, "Energy flows"}} );
    m_summaryToDescMap.insert( {"FTITHEA", {A::SUMMARY_FIELD, "Energy Injection Total"}} );
    m_summaryToDescMap.insert(
        {"FTIPTHEA", {A::SUMMARY_FIELD, "Difference in Energy in place between current and initial time"}} );
    m_summaryToDescMap.insert( {"FTPR", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPT", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"FTPC", {A::SUMMARY_FIELD, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"FTIR", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTIT", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"FTIC", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FTIPT", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FTIPF", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FTIPS", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"FTIP#", {A::SUMMARY_FIELD, " Tracer In Place in phase # (1,2,3,...)"}} );
    m_summaryToDescMap.insert( {"FTADS", {A::SUMMARY_FIELD, "Tracer Adsorption total"}} );
    m_summaryToDescMap.insert( {"FTDCY", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"FTIRF", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTIRS", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTPRF", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPRS", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"FTITF", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"FTITS", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"FTPTF", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"FTPTS", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"FTICF", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FTICS", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FTPCF", {A::SUMMARY_FIELD, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"FTPCS", {A::SUMMARY_FIELD, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"FMPR", {A::SUMMARY_FIELD, "Methane Production Rate"}} );
    m_summaryToDescMap.insert( {"FMPT", {A::SUMMARY_FIELD, "Methane Production Total"}} );
    m_summaryToDescMap.insert( {"FMIR", {A::SUMMARY_FIELD, "Methane Injection Rate"}} );
    m_summaryToDescMap.insert( {"FMIT", {A::SUMMARY_FIELD, "Methane Injection Total"}} );
    m_summaryToDescMap.insert( {"FCGC", {A::SUMMARY_FIELD, "Bulk Coal Gas Concentration"}} );
    m_summaryToDescMap.insert( {"FCSC", {A::SUMMARY_FIELD, "Bulk Coal Solvent Concentration"}} );
    m_summaryToDescMap.insert( {"FTPRFOA", {A::SUMMARY_FIELD, "Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPTFOA", {A::SUMMARY_FIELD, "Production Total"}} );
    m_summaryToDescMap.insert( {"FTIRFOA", {A::SUMMARY_FIELD, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTITFOA", {A::SUMMARY_FIELD, "Injection Total"}} );
    m_summaryToDescMap.insert( {"FTIPTFOA", {A::SUMMARY_FIELD, "In Solution"}} );
    m_summaryToDescMap.insert( {"FTADSFOA", {A::SUMMARY_FIELD, "Adsorption total"}} );
    m_summaryToDescMap.insert( {"FTDCYFOA", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"FTMOBFOA", {A::SUMMARY_FIELD, "Gas mobility factor"}} );
    m_summaryToDescMap.insert( {"FTPRFOA", {A::SUMMARY_FIELD, "Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPTFOA", {A::SUMMARY_FIELD, "Production Total"}} );
    m_summaryToDescMap.insert( {"FTIRFOA", {A::SUMMARY_FIELD, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTITFOA", {A::SUMMARY_FIELD, "Injection Total"}} );
    m_summaryToDescMap.insert( {"FTIPTFOA", {A::SUMMARY_FIELD, "In Solution"}} );
    m_summaryToDescMap.insert( {"FTADSFOA", {A::SUMMARY_FIELD, "Adsorption total"}} );
    m_summaryToDescMap.insert( {"FTDCYFOA", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"FTMOBFOA", {A::SUMMARY_FIELD, "Gas mobility factor"}} );
    m_summaryToDescMap.insert( {"FSGR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"FGSR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"FSGT", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"FGST", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"FGDC", {A::SUMMARY_FIELD, "Gas Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"FGDCQ", {A::SUMMARY_FIELD, "Field/Group Gas DCQ"}} );
    m_summaryToDescMap.insert( {"FGCR", {A::SUMMARY_FIELD, "Gas consumption rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGCT", {A::SUMMARY_FIELD, "Gas consumption cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FFGR", {A::SUMMARY_FIELD, "Fuel Gas rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FFGT", {A::SUMMARY_FIELD, "Fuel Gas cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGIMR", {A::SUMMARY_FIELD, "Gas import rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGIMT", {A::SUMMARY_FIELD, "Gas import cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert( {"FGLIR", {A::SUMMARY_FIELD, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"FGCV", {A::SUMMARY_FIELD, "Gas Calorific Value"}} );
    m_summaryToDescMap.insert( {"FGQ", {A::SUMMARY_FIELD, "Gas molar Quality"}} );
    m_summaryToDescMap.insert( {"FEPR", {A::SUMMARY_FIELD, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"FEPT", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"FESR", {A::SUMMARY_FIELD, "Energy Sales Rate"}} );
    m_summaryToDescMap.insert( {"FEST", {A::SUMMARY_FIELD, "Energy Sales Total"}} );
    m_summaryToDescMap.insert( {"FEDC", {A::SUMMARY_FIELD, "Energy Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"FEDCQ", {A::SUMMARY_FIELD, "Energy DCQ"}} );
    m_summaryToDescMap.insert( {"FCPR", {A::SUMMARY_FIELD, "Polymer Production Rate"}} );
    m_summaryToDescMap.insert( {"FCPC", {A::SUMMARY_FIELD, "Polymer Production Concentration"}} );
    m_summaryToDescMap.insert( {"FCPT", {A::SUMMARY_FIELD, "Polymer Production Total"}} );
    m_summaryToDescMap.insert( {"FCIR", {A::SUMMARY_FIELD, "Polymer Injection Rate"}} );
    m_summaryToDescMap.insert( {"FCIC", {A::SUMMARY_FIELD, "Polymer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"FCIT", {A::SUMMARY_FIELD, "Polymer Injection Total"}} );
    m_summaryToDescMap.insert( {"FCIP", {A::SUMMARY_FIELD, "Polymer In Solution"}} );
    m_summaryToDescMap.insert( {"FCAD", {A::SUMMARY_FIELD, "Polymer Adsorption total"}} );
    m_summaryToDescMap.insert( {"FSPR", {A::SUMMARY_FIELD, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"FSPT", {A::SUMMARY_FIELD, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"FSIR", {A::SUMMARY_FIELD, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"FSIT", {A::SUMMARY_FIELD, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"FSIP", {A::SUMMARY_FIELD, "Salt In Place"}} );
    m_summaryToDescMap.insert( {"PSSPR", {A::SUMMARY_FIELD, "Log of the pressure change per unit time"}} );
    m_summaryToDescMap.insert( {"PSSSO", {A::SUMMARY_FIELD, "Log of the oil saturation change per unit time"}} );
    m_summaryToDescMap.insert( {"PSSSW", {A::SUMMARY_FIELD, "Log of the water saturation change per unit time"}} );
    m_summaryToDescMap.insert( {"PSSSG", {A::SUMMARY_FIELD, "Log of the gas saturation change per unit time"}} );
    m_summaryToDescMap.insert( {"PSSSC", {A::SUMMARY_FIELD, "Log of the salt concentration change per unit time"}} );
    m_summaryToDescMap.insert( {"FNPR", {A::SUMMARY_FIELD, "Solvent Production Rate"}} );
    m_summaryToDescMap.insert( {"FNPT", {A::SUMMARY_FIELD, "Solvent Production Total"}} );
    m_summaryToDescMap.insert( {"FNIR", {A::SUMMARY_FIELD, "Solvent Injection Rate"}} );
    m_summaryToDescMap.insert( {"FNIT", {A::SUMMARY_FIELD, "Solvent Injection Total"}} );
    m_summaryToDescMap.insert( {"FNIP", {A::SUMMARY_FIELD, "Solvent In Place"}} );
    m_summaryToDescMap.insert( {"FTPRSUR", {A::SUMMARY_FIELD, "Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPTSUR", {A::SUMMARY_FIELD, "Production Total"}} );
    m_summaryToDescMap.insert( {"FTIRSUR", {A::SUMMARY_FIELD, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTITSUR", {A::SUMMARY_FIELD, "Injection Total"}} );
    m_summaryToDescMap.insert( {"FTIPTSUR", {A::SUMMARY_FIELD, "In Solution"}} );
    m_summaryToDescMap.insert( {"FTADSUR", {A::SUMMARY_FIELD, "Adsorption total"}} );
    m_summaryToDescMap.insert( {"FTPRALK", {A::SUMMARY_FIELD, "Production Rate"}} );
    m_summaryToDescMap.insert( {"FTPTALK", {A::SUMMARY_FIELD, "Production Total"}} );
    m_summaryToDescMap.insert( {"FTIRALK", {A::SUMMARY_FIELD, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"FTITALK", {A::SUMMARY_FIELD, "Injection Total"}} );
    m_summaryToDescMap.insert( {"FU", {A::SUMMARY_FIELD, "User-defined field quantity"}} );

    m_summaryToDescMap.insert( {"GOPR", {A::SUMMARY_WELL_GROUP, "Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"GOPRA", {A::SUMMARY_WELL_GROUP, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"GOPRB", {A::SUMMARY_WELL_GROUP, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"GOPTA", {A::SUMMARY_WELL_GROUP, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"GOPTB", {A::SUMMARY_WELL_GROUP, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"GOPR1", {A::SUMMARY_WELL_GROUP, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"GOPR2", {A::SUMMARY_WELL_GROUP, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"GOPT1", {A::SUMMARY_WELL_GROUP, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"GOPT2", {A::SUMMARY_WELL_GROUP, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"GOMR", {A::SUMMARY_WELL_GROUP, "Oil Mass Rate"}} );
    m_summaryToDescMap.insert( {"GOMT", {A::SUMMARY_WELL_GROUP, "Oil Mass Total"}} );
    m_summaryToDescMap.insert( {"GODN", {A::SUMMARY_WELL_GROUP, "Oil Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"GOPRH", {A::SUMMARY_WELL_GROUP, "Oil Production Rate History"}} );
    m_summaryToDescMap.insert( {"GOPRT", {A::SUMMARY_WELL_GROUP, "Oil Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GOPRL", {A::SUMMARY_WELL_GROUP, "Oil Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GOPRF", {A::SUMMARY_WELL_GROUP, "Free Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"GOPRS", {A::SUMMARY_WELL_GROUP, "Solution Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"GOPT", {A::SUMMARY_WELL_GROUP, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"GOPTH", {A::SUMMARY_WELL_GROUP, "Oil Production Total History"}} );
    m_summaryToDescMap.insert( {"GOPTF", {A::SUMMARY_WELL_GROUP, "Free Oil Production Total"}} );
    m_summaryToDescMap.insert( {"GOPTS", {A::SUMMARY_WELL_GROUP, "Solution Oil Production Total"}} );
    m_summaryToDescMap.insert( {"GOIR", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate"}} );
    m_summaryToDescMap.insert( {"GOIRH", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate History"}} );
    m_summaryToDescMap.insert( {"GOIRT", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GOIRL", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GOIT", {A::SUMMARY_WELL_GROUP, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"GOITH", {A::SUMMARY_WELL_GROUP, "Oil Injection Total History"}} );
    m_summaryToDescMap.insert( {"GOPP", {A::SUMMARY_WELL_GROUP, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GOPP2", {A::SUMMARY_WELL_GROUP, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GOPI", {A::SUMMARY_WELL_GROUP, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GOPI2", {A::SUMMARY_WELL_GROUP, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GOPGR", {A::SUMMARY_WELL_GROUP, "Oil Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"GOIGR", {A::SUMMARY_WELL_GROUP, "Oil Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"GOVPR", {A::SUMMARY_WELL_GROUP, "Oil Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"GOVPT", {A::SUMMARY_WELL_GROUP, "Oil Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"GOVIR", {A::SUMMARY_WELL_GROUP, "Oil Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"GOVIT", {A::SUMMARY_WELL_GROUP, "Oil Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"GOnPR", {A::SUMMARY_WELL_GROUP, "nth separator stage oil rate"}} );
    m_summaryToDescMap.insert( {"GOnPT", {A::SUMMARY_WELL_GROUP, "nth separator stage oil total"}} );
    m_summaryToDescMap.insert( {"GEOR", {A::SUMMARY_WELL_GROUP, "Export Oil Rate"}} );
    m_summaryToDescMap.insert( {"GEOT", {A::SUMMARY_WELL_GROUP, "Export Oil Total"}} );
    m_summaryToDescMap.insert( {"GEOMF", {A::SUMMARY_WELL_GROUP, "Export Oil Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GWPR", {A::SUMMARY_WELL_GROUP, "Water Production Rate"}} );
    m_summaryToDescMap.insert( {"GWMR", {A::SUMMARY_WELL_GROUP, "Water Mass Rate"}} );
    m_summaryToDescMap.insert( {"GWMT", {A::SUMMARY_WELL_GROUP, "Water Mass Total"}} );
    m_summaryToDescMap.insert( {"GWPRH", {A::SUMMARY_WELL_GROUP, "Water Production Rate History"}} );
    m_summaryToDescMap.insert( {"GWPRT", {A::SUMMARY_WELL_GROUP, "Water Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GWPRL", {A::SUMMARY_WELL_GROUP, "Water Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GWPT", {A::SUMMARY_WELL_GROUP, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"GWPTH", {A::SUMMARY_WELL_GROUP, "Water Production Total History"}} );
    m_summaryToDescMap.insert( {"GWIR", {A::SUMMARY_WELL_GROUP, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"GWIRH", {A::SUMMARY_WELL_GROUP, "Water Injection Rate History"}} );
    m_summaryToDescMap.insert( {"GWIRT", {A::SUMMARY_WELL_GROUP, "Water Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GWIRL", {A::SUMMARY_WELL_GROUP, "Water Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GWIT", {A::SUMMARY_WELL_GROUP, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"GWITH", {A::SUMMARY_WELL_GROUP, "Water Injection Total History"}} );
    m_summaryToDescMap.insert( {"GWPP", {A::SUMMARY_WELL_GROUP, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GWPP2", {A::SUMMARY_WELL_GROUP, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GWPI", {A::SUMMARY_WELL_GROUP, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GWPI2", {A::SUMMARY_WELL_GROUP, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GWPGR", {A::SUMMARY_WELL_GROUP, "Water Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"GWIGR", {A::SUMMARY_WELL_GROUP, "Water Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"GWVPR", {A::SUMMARY_WELL_GROUP, "Water Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"GWVPT", {A::SUMMARY_WELL_GROUP, "Water Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"GWVIR", {A::SUMMARY_WELL_GROUP, "Water Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"GWVIT", {A::SUMMARY_WELL_GROUP, "Water Voidage Injection Total"}} );
    m_summaryToDescMap.insert(
        {"GWPIR", {A::SUMMARY_WELL_GROUP, "Ratio of produced water to injected water (percentage)"}} );
    m_summaryToDescMap.insert( {"GWMPR", {A::SUMMARY_WELL_GROUP, "Water component molar production rate"}} );
    m_summaryToDescMap.insert( {"GWMPT", {A::SUMMARY_WELL_GROUP, "Water component molar production total"}} );
    m_summaryToDescMap.insert( {"GWMIR", {A::SUMMARY_WELL_GROUP, "Water component molar injection rate"}} );
    m_summaryToDescMap.insert( {"GWMIT", {A::SUMMARY_WELL_GROUP, "Water component molar injection total"}} );
    m_summaryToDescMap.insert( {"GGPR", {A::SUMMARY_WELL_GROUP, "Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"GGPRA", {A::SUMMARY_WELL_GROUP, "Gas Production Rate above"}} );
    m_summaryToDescMap.insert( {"GGPRB", {A::SUMMARY_WELL_GROUP, "Gas Production Rate below"}} );
    m_summaryToDescMap.insert( {"GGPTA", {A::SUMMARY_WELL_GROUP, "Gas Production Total above"}} );
    m_summaryToDescMap.insert( {"GGPTB", {A::SUMMARY_WELL_GROUP, "Gas Production Total below"}} );
    m_summaryToDescMap.insert( {"GGPR1", {A::SUMMARY_WELL_GROUP, "Gas Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"GGPR2", {A::SUMMARY_WELL_GROUP, "Gas Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"GGPT1", {A::SUMMARY_WELL_GROUP, "Gas Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"GGPT2", {A::SUMMARY_WELL_GROUP, "Gas Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"GGMR", {A::SUMMARY_WELL_GROUP, "Gas Mass Rate"}} );
    m_summaryToDescMap.insert( {"GGMT", {A::SUMMARY_WELL_GROUP, "Gas Mass Total"}} );
    m_summaryToDescMap.insert( {"GGDN", {A::SUMMARY_WELL_GROUP, "Gas Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"GGPRH", {A::SUMMARY_WELL_GROUP, "Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"GGPRT", {A::SUMMARY_WELL_GROUP, "Gas Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GGPRL", {A::SUMMARY_WELL_GROUP, "Gas Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GGPRF", {A::SUMMARY_WELL_GROUP, "Free Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"GGPRS", {A::SUMMARY_WELL_GROUP, "Solution Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"GGPT", {A::SUMMARY_WELL_GROUP, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"GGPTH", {A::SUMMARY_WELL_GROUP, "Gas Production Total History"}} );
    m_summaryToDescMap.insert( {"GGPTF", {A::SUMMARY_WELL_GROUP, "Free Gas Production Total"}} );
    m_summaryToDescMap.insert( {"GGPTS", {A::SUMMARY_WELL_GROUP, "Solution Gas Production Total"}} );
    m_summaryToDescMap.insert( {"GGIR", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"GGIRH", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate History"}} );
    m_summaryToDescMap.insert( {"GGIRT", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GGIRL", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GGIT", {A::SUMMARY_WELL_GROUP, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"GGITH", {A::SUMMARY_WELL_GROUP, "Gas Injection Total History"}} );
    m_summaryToDescMap.insert( {"GGPP", {A::SUMMARY_WELL_GROUP, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GGPP2", {A::SUMMARY_WELL_GROUP, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GGPPS", {A::SUMMARY_WELL_GROUP, "Solution"}} );
    m_summaryToDescMap.insert( {"GGPPS2", {A::SUMMARY_WELL_GROUP, "Solution"}} );
    m_summaryToDescMap.insert( {"GGPPF", {A::SUMMARY_WELL_GROUP, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GGPPF2", {A::SUMMARY_WELL_GROUP, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"GGPI", {A::SUMMARY_WELL_GROUP, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GGPI2", {A::SUMMARY_WELL_GROUP, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"GGPGR", {A::SUMMARY_WELL_GROUP, "Gas Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"GGIGR", {A::SUMMARY_WELL_GROUP, "Gas Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"GSGR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"GGSR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"GSGT", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"GGST", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"GSMF", {A::SUMMARY_WELL_GROUP, "Sales Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GFGR", {A::SUMMARY_WELL_GROUP, "Fuel Gas Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"GFGT", {A::SUMMARY_WELL_GROUP, "Fuel Gas cumulative Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"GFMF", {A::SUMMARY_WELL_GROUP, "Fuel Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GGCR", {A::SUMMARY_WELL_GROUP, "Gas Consumption Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"GGCT", {A::SUMMARY_WELL_GROUP, "Gas Consumption Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"GGIMR", {A::SUMMARY_WELL_GROUP, "Gas Import Rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"GGIMT", {A::SUMMARY_WELL_GROUP, "Gas Import Total, at and below this group"}} );
    m_summaryToDescMap.insert( {"GGLIR", {A::SUMMARY_WELL_GROUP, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"GWGPR", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"GWGPT", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"GWGPRH", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"GWGIR", {A::SUMMARY_WELL_GROUP, "Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"GWGIT", {A::SUMMARY_WELL_GROUP, "Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"GEGR", {A::SUMMARY_WELL_GROUP, "Export Gas Rate"}} );
    m_summaryToDescMap.insert( {"GEGT", {A::SUMMARY_WELL_GROUP, "Export Gas Total"}} );
    m_summaryToDescMap.insert( {"GEMF", {A::SUMMARY_WELL_GROUP, "Export Gas Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GEXGR", {A::SUMMARY_WELL_GROUP, "Excess Gas Rate"}} );
    m_summaryToDescMap.insert( {"GEXGT", {A::SUMMARY_WELL_GROUP, "Excess Gas Total"}} );
    m_summaryToDescMap.insert( {"GRGR", {A::SUMMARY_WELL_GROUP, "Re-injection Gas Rate"}} );
    m_summaryToDescMap.insert( {"GRGT", {A::SUMMARY_WELL_GROUP, "Re-injection Gas Total"}} );
    m_summaryToDescMap.insert( {"GGnPR", {A::SUMMARY_WELL_GROUP, "nth separator stage gas rate"}} );
    m_summaryToDescMap.insert( {"GGnPT", {A::SUMMARY_WELL_GROUP, "nth separator stage gas total"}} );
    m_summaryToDescMap.insert( {"GGVPR", {A::SUMMARY_WELL_GROUP, "Gas Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"GGVPT", {A::SUMMARY_WELL_GROUP, "Gas Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"GGVIR", {A::SUMMARY_WELL_GROUP, "Gas Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"GGVIT", {A::SUMMARY_WELL_GROUP, "Gas Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"GGQ", {A::SUMMARY_WELL_GROUP, "Gas Quality"}} );
    m_summaryToDescMap.insert( {"GLPR", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate"}} );
    m_summaryToDescMap.insert( {"GLPRH", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate History"}} );
    m_summaryToDescMap.insert( {"GLPRT", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GLPRL", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GLPT", {A::SUMMARY_WELL_GROUP, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"GLPTH", {A::SUMMARY_WELL_GROUP, "Liquid Production Total History"}} );
    m_summaryToDescMap.insert( {"GVPR", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate"}} );
    m_summaryToDescMap.insert( {"GVPRT", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GVPRL", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GVPT", {A::SUMMARY_WELL_GROUP, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"GVPGR", {A::SUMMARY_WELL_GROUP, "Res Volume Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"GVIR", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate"}} );
    m_summaryToDescMap.insert( {"GVIRT", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GVIRL", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"GVIT", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"GWCT", {A::SUMMARY_WELL_GROUP, "Water Cut"}} );
    m_summaryToDescMap.insert( {"GWCTH", {A::SUMMARY_WELL_GROUP, "Water Cut History"}} );
    m_summaryToDescMap.insert( {"GGOR", {A::SUMMARY_WELL_GROUP, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"GGORH", {A::SUMMARY_WELL_GROUP, "Gas-Oil Ratio History"}} );
    m_summaryToDescMap.insert( {"GOGR", {A::SUMMARY_WELL_GROUP, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"GOGRH", {A::SUMMARY_WELL_GROUP, "Oil-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"GWGR", {A::SUMMARY_WELL_GROUP, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"GWGRH", {A::SUMMARY_WELL_GROUP, "Water-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"GGLR", {A::SUMMARY_WELL_GROUP, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"GGLRH", {A::SUMMARY_WELL_GROUP, "Gas-Liquid Ratio History"}} );
    m_summaryToDescMap.insert( {"GMCTP", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Production"}} );
    m_summaryToDescMap.insert( {"GMCTW", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Water Injection"}} );
    m_summaryToDescMap.insert( {"GMCTG", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Gas Injection"}} );
    m_summaryToDescMap.insert( {"GMWPT", {A::SUMMARY_WELL_GROUP, "Total number of production wells"}} );
    m_summaryToDescMap.insert( {"GMWPR", {A::SUMMARY_WELL_GROUP, "Number of production wells currently flowing"}} );
    m_summaryToDescMap.insert( {"GMWPA", {A::SUMMARY_WELL_GROUP, "Number of abandoned production wells"}} );
    m_summaryToDescMap.insert( {"GMWPU", {A::SUMMARY_WELL_GROUP, "Number of unused production wells"}} );
    m_summaryToDescMap.insert( {"GMWPG", {A::SUMMARY_WELL_GROUP, "Number of producers on group control"}} );
    m_summaryToDescMap.insert(
        {"GMWPO", {A::SUMMARY_WELL_GROUP, "Number of producers controlled by own oil rate limit"}} );
    m_summaryToDescMap.insert(
        {"GMWPS", {A::SUMMARY_WELL_GROUP, "Number of producers on own surface rate limit control"}} );
    m_summaryToDescMap.insert(
        {"GMWPV", {A::SUMMARY_WELL_GROUP, "Number of producers on own reservoir volume rate limit control"}} );
    m_summaryToDescMap.insert( {"GMWPP", {A::SUMMARY_WELL_GROUP, "Number of producers on pressure control"}} );
    m_summaryToDescMap.insert( {"GMWPL", {A::SUMMARY_WELL_GROUP, "Number of producers using artificial lift"}} );
    m_summaryToDescMap.insert( {"GMWIT", {A::SUMMARY_WELL_GROUP, "Total number of injection wells"}} );
    m_summaryToDescMap.insert( {"GMWIN", {A::SUMMARY_WELL_GROUP, "Number of injection wells currently flowing"}} );
    m_summaryToDescMap.insert( {"GMWIA", {A::SUMMARY_WELL_GROUP, "Number of abandoned injection wells"}} );
    m_summaryToDescMap.insert( {"GMWIU", {A::SUMMARY_WELL_GROUP, "Number of unused injection wells"}} );
    m_summaryToDescMap.insert( {"GMWIG", {A::SUMMARY_WELL_GROUP, "Number of injectors on group control"}} );
    m_summaryToDescMap.insert(
        {"GMWIS", {A::SUMMARY_WELL_GROUP, "Number of injectors on own surface rate limit control"}} );
    m_summaryToDescMap.insert(
        {"GMWIV", {A::SUMMARY_WELL_GROUP, "Number of injectors on own reservoir volume rate limit control"}} );
    m_summaryToDescMap.insert( {"GMWIP", {A::SUMMARY_WELL_GROUP, "Number of injectors on pressure control"}} );
    m_summaryToDescMap.insert( {"GMWDR", {A::SUMMARY_WELL_GROUP, "Number of drilling events this timestep"}} );
    m_summaryToDescMap.insert( {"GMWDT", {A::SUMMARY_WELL_GROUP, "Number of drilling events in total"}} );
    m_summaryToDescMap.insert( {"GMWWO", {A::SUMMARY_WELL_GROUP, "Number of workover events this timestep"}} );
    m_summaryToDescMap.insert( {"GMWWT", {A::SUMMARY_WELL_GROUP, "Number of workover events in total"}} );
    m_summaryToDescMap.insert( {"GEPR", {A::SUMMARY_WELL_GROUP, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"GEPT", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"GEFF", {A::SUMMARY_WELL_GROUP, "Efficiency Factor"}} );
    m_summaryToDescMap.insert( {"GNLPR", {A::SUMMARY_WELL_GROUP, "NGL Production Rate"}} );
    m_summaryToDescMap.insert( {"GNLPT", {A::SUMMARY_WELL_GROUP, "NGL Production Total"}} );
    m_summaryToDescMap.insert( {"GNLPRH", {A::SUMMARY_WELL_GROUP, "NGL Production Rate History"}} );
    m_summaryToDescMap.insert( {"GNLPTH", {A::SUMMARY_WELL_GROUP, "NGL Production Total History"}} );
    m_summaryToDescMap.insert(
        {"GAMF", {A::SUMMARY_WELL_GROUP, "Component aqueous mole fraction, from producing completions"}} );
    m_summaryToDescMap.insert( {"GXMF", {A::SUMMARY_WELL_GROUP, "Liquid Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GYMF", {A::SUMMARY_WELL_GROUP, "Vapor Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GXMFn", {A::SUMMARY_WELL_GROUP, "Liquid Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"GYMFn", {A::SUMMARY_WELL_GROUP, "Vapor Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"GZMF", {A::SUMMARY_WELL_GROUP, "Total Mole Fraction"}} );
    m_summaryToDescMap.insert( {"GCMPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Production Rates"}} );
    m_summaryToDescMap.insert( {"GCMPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"GCMIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Injection Rates"}} );
    m_summaryToDescMap.insert( {"GCMIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Injection Totals"}} );
    m_summaryToDescMap.insert( {"GHMIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Injection Rate"}} );
    m_summaryToDescMap.insert( {"GHMIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Injection Total"}} );
    m_summaryToDescMap.insert( {"GHMPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Production Rate"}} );
    m_summaryToDescMap.insert( {"GHMPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Production Total"}} );
    m_summaryToDescMap.insert( {"GCHMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"GCHMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"GCWGPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"GCWGPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"GCWGIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"GCWGIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"GCGMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCGMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCOMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCOMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCNMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the NGL phase"}} );
    m_summaryToDescMap.insert( {"GCNWR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component mass rates in the NGL phase"}} );
    m_summaryToDescMap.insert(
        {"GCGMRn",
         {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert(
        {"GCGRn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert( {"GCOMRn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCORn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GMUF", {A::SUMMARY_WELL_GROUP, "Make-up fraction"}} );
    m_summaryToDescMap.insert( {"GAMR", {A::SUMMARY_WELL_GROUP, "Make-up gas rate"}} );
    m_summaryToDescMap.insert( {"GAMT", {A::SUMMARY_WELL_GROUP, "Make-up gas total"}} );
    m_summaryToDescMap.insert(
        {"GGSPR", {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert( {"GGSRL",
                                {A::SUMMARY_WELL_GROUP,
                                 "Maximum tested rate sustained for the test period during the most recent sustainable "
                                 "capacity test for gas"}} );
    m_summaryToDescMap.insert(
        {"GGSRU",
         {A::SUMMARY_WELL_GROUP,
          "Minimum tested rate not sustained for the test period during the most recent sustainable capacity test for "
          "gas"}} );
    m_summaryToDescMap.insert(
        {"GGSSP",
         {A::SUMMARY_WELL_GROUP,
          "Period for which target sustainable rate could be maintained for the most recent sustainable capacity test "
          "for gas"}} );
    m_summaryToDescMap.insert(
        {"GGSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for gas"}} );
    m_summaryToDescMap.insert(
        {"GOSPR", {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert( {"GOSRL",
                                {A::SUMMARY_WELL_GROUP,
                                 "Maximum tested rate sustained for the test period during the most recent sustainable "
                                 "capacity test for oil"}} );
    m_summaryToDescMap.insert(
        {"GOSRU",
         {A::SUMMARY_WELL_GROUP,
          "Minimum tested rate not sustained for the test period during the most recent sustainable capacity test for "
          "oil"}} );
    m_summaryToDescMap.insert(
        {"GOSSP",
         {A::SUMMARY_WELL_GROUP,
          "Period for which target sustainable rate could be maintained for the most recent sustainable capacity test "
          "for oil"}} );
    m_summaryToDescMap.insert(
        {"GOSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for oil"}} );
    m_summaryToDescMap.insert(
        {"GWSPR",
         {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for water"}} );
    m_summaryToDescMap.insert( {"GWSRL",
                                {A::SUMMARY_WELL_GROUP,
                                 "Maximum tested rate sustained for the test period during the most recent sustainable "
                                 "capacity test for water"}} );
    m_summaryToDescMap.insert(
        {"GWSRU",
         {A::SUMMARY_WELL_GROUP,
          "Minimum tested rate not sustained for the test period during the most recent sustainable capacity test for "
          "water"}} );
    m_summaryToDescMap.insert(
        {"GWSSP",
         {A::SUMMARY_WELL_GROUP,
          "Period for which target sustainable rate could be maintained for the most recent sustainable capacity test "
          "for water"}} );
    m_summaryToDescMap.insert(
        {"GWSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for water"}} );
    m_summaryToDescMap.insert( {"GGPRG", {A::SUMMARY_WELL_GROUP, "Gas production rate"}} );
    m_summaryToDescMap.insert( {"GOPRG", {A::SUMMARY_WELL_GROUP, "Oil production rate"}} );
    m_summaryToDescMap.insert( {"GNLPRG", {A::SUMMARY_WELL_GROUP, "NGL production rate"}} );
    m_summaryToDescMap.insert( {"GXMFG", {A::SUMMARY_WELL_GROUP, "Liquid mole fraction"}} );
    m_summaryToDescMap.insert( {"GYMFG", {A::SUMMARY_WELL_GROUP, "Vapor mole fraction"}} );
    m_summaryToDescMap.insert( {"GCOMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"GCGMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase"}} );
    m_summaryToDescMap.insert( {"GCNMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the NGL phase"}} );
    m_summaryToDescMap.insert( {"GTPR", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPT", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"GTPC", {A::SUMMARY_WELL_GROUP, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"GTIR", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTIT", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"GTIC", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"GTMR", {A::SUMMARY_WELL_GROUP, "Traced mass Rate"}} );
    m_summaryToDescMap.insert( {"GTMT", {A::SUMMARY_WELL_GROUP, "Traced mass Total"}} );
    m_summaryToDescMap.insert( {"GTQR", {A::SUMMARY_WELL_GROUP, "Traced molar Rate"}} );
    m_summaryToDescMap.insert( {"GTCM", {A::SUMMARY_WELL_GROUP, "Tracer Carrier molar Rate"}} );
    m_summaryToDescMap.insert( {"GTMF", {A::SUMMARY_WELL_GROUP, "Traced molar fraction"}} );
    m_summaryToDescMap.insert( {"GTVL", {A::SUMMARY_WELL_GROUP, "Traced liquid volume rate"}} );
    m_summaryToDescMap.insert( {"GTVV", {A::SUMMARY_WELL_GROUP, "Traced vapor volume rate"}} );
    m_summaryToDescMap.insert( {"GTTL", {A::SUMMARY_WELL_GROUP, "Traced liquid volume total"}} );
    m_summaryToDescMap.insert( {"GTTV", {A::SUMMARY_WELL_GROUP, "Traced vapor volume total"}} );
    m_summaryToDescMap.insert( {"GTML", {A::SUMMARY_WELL_GROUP, "Traced mass liquid rate"}} );
    m_summaryToDescMap.insert( {"GTMV", {A::SUMMARY_WELL_GROUP, "Traced mass vapor rate"}} );
    m_summaryToDescMap.insert( {"GTLM", {A::SUMMARY_WELL_GROUP, "Traced mass liquid total"}} );
    m_summaryToDescMap.insert( {"GTVM", {A::SUMMARY_WELL_GROUP, "Traced mass vapor total"}} );
    m_summaryToDescMap.insert( {"GAPI", {A::SUMMARY_WELL_GROUP, "Oil API"}} );
    m_summaryToDescMap.insert( {"GSPR", {A::SUMMARY_WELL_GROUP, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"GSPT", {A::SUMMARY_WELL_GROUP, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"GSIR", {A::SUMMARY_WELL_GROUP, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"GSIT", {A::SUMMARY_WELL_GROUP, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"GSPC", {A::SUMMARY_WELL_GROUP, "Salt Production Concentration"}} );
    m_summaryToDescMap.insert( {"GSIC", {A::SUMMARY_WELL_GROUP, "Salt Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTPRANI", {A::SUMMARY_WELL_GROUP, "Anion Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPTANI", {A::SUMMARY_WELL_GROUP, "Anion Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRANI", {A::SUMMARY_WELL_GROUP, "Anion Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTITANI", {A::SUMMARY_WELL_GROUP, "Anion Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPRCAT", {A::SUMMARY_WELL_GROUP, "Cation Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPTCAT", {A::SUMMARY_WELL_GROUP, "Cation Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRCAT", {A::SUMMARY_WELL_GROUP, "Cation Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTITCAT", {A::SUMMARY_WELL_GROUP, "Cation Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPCHEA", {A::SUMMARY_WELL_GROUP, "Production Temperature"}} );
    m_summaryToDescMap.insert( {"GTICHEA", {A::SUMMARY_WELL_GROUP, "Injection Temperature"}} );
    m_summaryToDescMap.insert( {"GTPRHEA", {A::SUMMARY_WELL_GROUP, "Energy flows"}} );
    m_summaryToDescMap.insert( {"GTPTHEA", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRHEA", {A::SUMMARY_WELL_GROUP, "Energy flows"}} );
    m_summaryToDescMap.insert( {"GTITHEA", {A::SUMMARY_WELL_GROUP, "Energy Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPR", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPT", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"GTPC", {A::SUMMARY_WELL_GROUP, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"GTIR", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTIT", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"GTIC", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"GTIRF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTIRS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTPRF", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPRS", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"GTITF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"GTITS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPTF", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"GTPTS", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"GTICF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"GTICS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"GTPCF", {A::SUMMARY_WELL_GROUP, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"GTPCS", {A::SUMMARY_WELL_GROUP, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"GMPR", {A::SUMMARY_WELL_GROUP, "Methane Production Rate"}} );
    m_summaryToDescMap.insert( {"GMPT", {A::SUMMARY_WELL_GROUP, "Methane Production Total"}} );
    m_summaryToDescMap.insert( {"GMIR", {A::SUMMARY_WELL_GROUP, "Methane Injection Rate"}} );
    m_summaryToDescMap.insert( {"GMIT", {A::SUMMARY_WELL_GROUP, "Methane Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPRFOA", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPTFOA", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRFOA", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTITFOA", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    m_summaryToDescMap.insert( {"GSGR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"GGSR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    m_summaryToDescMap.insert( {"GSGT", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"GGST", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    m_summaryToDescMap.insert( {"GGDC", {A::SUMMARY_WELL_GROUP, "Gas Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"GGDCQ", {A::SUMMARY_WELL_GROUP, "Field/Group Gas DCQ"}} );
    m_summaryToDescMap.insert( {"GMCPL", {A::SUMMARY_WELL_GROUP, "Group Multi-level Compressor Level"}} );
    m_summaryToDescMap.insert( {"GPR", {A::SUMMARY_WELL_GROUP, "Group nodal Pressure in network"}} );
    m_summaryToDescMap.insert( {"GPRDC", {A::SUMMARY_WELL_GROUP, "Group Pressure at Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"GGCR", {A::SUMMARY_WELL_GROUP, "Gas consumption rate, at and below this group"}} );
    m_summaryToDescMap.insert(
        {"GGCT", {A::SUMMARY_WELL_GROUP, "Gas consumption cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert( {"GFGR", {A::SUMMARY_WELL_GROUP, "Fuel Gas rate, at and below this group"}} );
    m_summaryToDescMap.insert( {"GFGT", {A::SUMMARY_WELL_GROUP, "Fuel Gas cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert( {"GGIMR", {A::SUMMARY_WELL_GROUP, "Gas import rate, at and below this group"}} );
    m_summaryToDescMap.insert(
        {"GGIMT", {A::SUMMARY_WELL_GROUP, "Gas import cumulative total, at and below this group"}} );
    m_summaryToDescMap.insert(
        {"GPRFP", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in network from end of First Pass"}} );
    m_summaryToDescMap.insert(
        {"GGPRNBFP",
         {A::SUMMARY_WELL_GROUP,
          "Gas flow rate along Group’s or node’s outlet branch in network, from end of First Pass"}} );
    m_summaryToDescMap.insert( {"GGLIR", {A::SUMMARY_WELL_GROUP, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"GGCV", {A::SUMMARY_WELL_GROUP, "Gas Calorific Value"}} );
    m_summaryToDescMap.insert( {"GGQ", {A::SUMMARY_WELL_GROUP, "Gas molar Quality"}} );
    m_summaryToDescMap.insert( {"GEPR", {A::SUMMARY_WELL_GROUP, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"GEPT", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"GESR", {A::SUMMARY_WELL_GROUP, "Energy Sales Rate"}} );
    m_summaryToDescMap.insert( {"GEST", {A::SUMMARY_WELL_GROUP, "Energy Sales Total"}} );
    m_summaryToDescMap.insert( {"GEDC", {A::SUMMARY_WELL_GROUP, "Energy Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"GEDCQ", {A::SUMMARY_WELL_GROUP, "Energy DCQ"}} );
    m_summaryToDescMap.insert( {"GPR", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the production network"}} );
    m_summaryToDescMap.insert( {"GPRG", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the gas injection network"}} );
    m_summaryToDescMap.insert(
        {"GPRW", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the water injection network"}} );
    m_summaryToDescMap.insert(
        {"GPRB",
         {A::SUMMARY_WELL_GROUP, "Pressure drop along the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GPRBG",
         {A::SUMMARY_WELL_GROUP, "Pressure drop along the group’s or node’s inlet branch in the gas injection network"}} );
    m_summaryToDescMap.insert(
        {"GPRBW",
         {A::SUMMARY_WELL_GROUP,
          "Pressure drop along the group’s or node’s inlet branch in the water injection network"}} );
    m_summaryToDescMap.insert(
        {"GALQ", {A::SUMMARY_WELL_GROUP, "ALQ in the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GOPRNB",
         {A::SUMMARY_WELL_GROUP, "Oil flow rate along the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GWPRNB",
         {A::SUMMARY_WELL_GROUP, "Water flow rate along the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GGPRNB",
         {A::SUMMARY_WELL_GROUP, "Gas flow rate along the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GLPRNB",
         {A::SUMMARY_WELL_GROUP, "Liquid flow rate along the group’s or node’s outlet branch in the production network"}} );
    m_summaryToDescMap.insert(
        {"GWIRNB",
         {A::SUMMARY_WELL_GROUP,
          "Water flow rate along the group’s or node’s inlet branch in the water injection network"}} );
    m_summaryToDescMap.insert(
        {"GGIRNB",
         {A::SUMMARY_WELL_GROUP, "Gas flow rate along the group’s or node’s inlet branch in the gas injection network"}} );
    m_summaryToDescMap.insert(
        {"GOMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum oil rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GGMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum gas rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GWMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum water rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GLMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum liquid rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GOMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum oil rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GGMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum gas rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GWMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum water rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GLMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum liquid rate as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GMNP",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum pressure as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GMXP",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum pressure as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GPRINC",
         {A::SUMMARY_WELL_GROUP, "Group or node pressure increment as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert(
        {"GPRDEC",
         {A::SUMMARY_WELL_GROUP, "Group or node pressure decrement as specified with GNETDP in the production network"}} );
    m_summaryToDescMap.insert( {"GCPR", {A::SUMMARY_WELL_GROUP, "Polymer Production Rate"}} );
    m_summaryToDescMap.insert( {"GCPC", {A::SUMMARY_WELL_GROUP, "Polymer Production Concentration"}} );
    m_summaryToDescMap.insert( {"GCPT", {A::SUMMARY_WELL_GROUP, "Polymer Production Total"}} );
    m_summaryToDescMap.insert( {"GCIR", {A::SUMMARY_WELL_GROUP, "Polymer Injection Rate"}} );
    m_summaryToDescMap.insert( {"GCIC", {A::SUMMARY_WELL_GROUP, "Polymer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"GCIT", {A::SUMMARY_WELL_GROUP, "Polymer Injection Total"}} );
    m_summaryToDescMap.insert( {"GSPR", {A::SUMMARY_WELL_GROUP, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"GSPT", {A::SUMMARY_WELL_GROUP, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"GSIR", {A::SUMMARY_WELL_GROUP, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"GSIT", {A::SUMMARY_WELL_GROUP, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"GOPRL", {A::SUMMARY_WELL_GROUP, "Group Oil Production Rate Target"}} );
    m_summaryToDescMap.insert( {"GOIRL", {A::SUMMARY_WELL_GROUP, "Group Oil Injection Rate Target"}} );
    m_summaryToDescMap.insert( {"GWPRL", {A::SUMMARY_WELL_GROUP, "Group Water Production Rate Target"}} );
    m_summaryToDescMap.insert( {"GWIRL", {A::SUMMARY_WELL_GROUP, "Group Water Injection Rate Target"}} );
    m_summaryToDescMap.insert( {"GGPRL", {A::SUMMARY_WELL_GROUP, "Group Gas Production Rate Target"}} );
    m_summaryToDescMap.insert( {"GGIRL", {A::SUMMARY_WELL_GROUP, "Group Gas Injection Rate Target"}} );
    m_summaryToDescMap.insert( {"GLPRL", {A::SUMMARY_WELL_GROUP, "Group Liquid Production Rate Target"}} );
    m_summaryToDescMap.insert( {"GVPRL", {A::SUMMARY_WELL_GROUP, "Group reservoir Volume Production Rate Target"}} );
    m_summaryToDescMap.insert( {"GVIRL", {A::SUMMARY_WELL_GROUP, "Group reservoir Volume Injection Rate Target"}} );
    m_summaryToDescMap.insert( {"GNPR", {A::SUMMARY_WELL_GROUP, "Solvent Production Rate"}} );
    m_summaryToDescMap.insert( {"GNPT", {A::SUMMARY_WELL_GROUP, "Solvent Production Total"}} );
    m_summaryToDescMap.insert( {"GNIR", {A::SUMMARY_WELL_GROUP, "Solvent Injection Rate"}} );
    m_summaryToDescMap.insert( {"GNIT", {A::SUMMARY_WELL_GROUP, "Solvent Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPRSUR", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPTSUR", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRSUR", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTITSUR", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    m_summaryToDescMap.insert( {"GTPRALK", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    m_summaryToDescMap.insert( {"GTPTALK", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    m_summaryToDescMap.insert( {"GTIRALK", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"GTITALK", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    m_summaryToDescMap.insert( {"GU", {A::SUMMARY_WELL_GROUP, "User-defined group quantity"}} );

    m_summaryToDescMap.insert( {"WOPR", {A::SUMMARY_WELL, "Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"WOPRA", {A::SUMMARY_WELL, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"WOPRB", {A::SUMMARY_WELL, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"WOPTA", {A::SUMMARY_WELL, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"WOPTB", {A::SUMMARY_WELL, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"WOPR1", {A::SUMMARY_WELL, "Oil Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"WOPR2", {A::SUMMARY_WELL, "Oil Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"WOPT1", {A::SUMMARY_WELL, "Oil Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"WOPT2", {A::SUMMARY_WELL, "Oil Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"WOMR", {A::SUMMARY_WELL, "Oil Mass Rate"}} );
    m_summaryToDescMap.insert( {"WOMT", {A::SUMMARY_WELL, "Oil Mass Total"}} );
    m_summaryToDescMap.insert( {"WODN", {A::SUMMARY_WELL, "Oil Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"WOPRH", {A::SUMMARY_WELL, "Oil Production Rate History"}} );
    m_summaryToDescMap.insert( {"WOPRT", {A::SUMMARY_WELL, "Oil Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WOPRF", {A::SUMMARY_WELL, "Free Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"WOPRS", {A::SUMMARY_WELL, "Solution Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"WOPT", {A::SUMMARY_WELL, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"WOPTH", {A::SUMMARY_WELL, "Oil Production Total History"}} );
    m_summaryToDescMap.insert( {"WOPTF", {A::SUMMARY_WELL, "Free Oil Production Total"}} );
    m_summaryToDescMap.insert( {"WOPTS", {A::SUMMARY_WELL, "Solution Oil Production Total"}} );
    m_summaryToDescMap.insert( {"WOIR", {A::SUMMARY_WELL, "Oil Injection Rate"}} );
    m_summaryToDescMap.insert( {"WOIRH", {A::SUMMARY_WELL, "Oil Injection Rate History"}} );
    m_summaryToDescMap.insert( {"WOIRT", {A::SUMMARY_WELL, "Oil Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WOIT", {A::SUMMARY_WELL, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"WOITH", {A::SUMMARY_WELL, "Oil Injection Total History"}} );
    m_summaryToDescMap.insert( {"WOPP", {A::SUMMARY_WELL, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WOPP2", {A::SUMMARY_WELL, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WOPI", {A::SUMMARY_WELL, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WOPI2", {A::SUMMARY_WELL, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WOPGR", {A::SUMMARY_WELL, "Oil Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"WOIGR", {A::SUMMARY_WELL, "Oil Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"WOVPR", {A::SUMMARY_WELL, "Oil Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"WOVPT", {A::SUMMARY_WELL, "Oil Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"WOVIR", {A::SUMMARY_WELL, "Oil Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"WOVIT", {A::SUMMARY_WELL, "Oil Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"WOnPR", {A::SUMMARY_WELL, "nth separator stage oil rate"}} );
    m_summaryToDescMap.insert( {"WOnPT", {A::SUMMARY_WELL, "nth separator stage oil total"}} );
    m_summaryToDescMap.insert( {"WWPR", {A::SUMMARY_WELL, "Water Production Rate"}} );
    m_summaryToDescMap.insert( {"WWMR", {A::SUMMARY_WELL, "Water Mass Rate"}} );
    m_summaryToDescMap.insert( {"WWMT", {A::SUMMARY_WELL, "Water Mass Total"}} );
    m_summaryToDescMap.insert( {"WWPRH", {A::SUMMARY_WELL, "Water Production Rate History"}} );
    m_summaryToDescMap.insert( {"WWPRT", {A::SUMMARY_WELL, "Water Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WWPT", {A::SUMMARY_WELL, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"WWPTH", {A::SUMMARY_WELL, "Water Production Total History"}} );
    m_summaryToDescMap.insert( {"WWIR", {A::SUMMARY_WELL, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"WWIRH", {A::SUMMARY_WELL, "Water Injection Rate History"}} );
    m_summaryToDescMap.insert( {"WWIRT", {A::SUMMARY_WELL, "Water Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WWIT", {A::SUMMARY_WELL, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"WWITH", {A::SUMMARY_WELL, "Water Injection Total History"}} );
    m_summaryToDescMap.insert( {"WWPP", {A::SUMMARY_WELL, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WWPP2", {A::SUMMARY_WELL, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WWPI", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WWIP", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WWPI2", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WWIP2", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WWPGR", {A::SUMMARY_WELL, "Water Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"WWIGR", {A::SUMMARY_WELL, "Water Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"WWVPR", {A::SUMMARY_WELL, "Water Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"WWVPT", {A::SUMMARY_WELL, "Water Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"WWVIR", {A::SUMMARY_WELL, "Water Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"WWVIT", {A::SUMMARY_WELL, "Water Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"WWPIR", {A::SUMMARY_WELL, "Ratio of produced water to injected water (percentage)"}} );
    m_summaryToDescMap.insert( {"WWMPR", {A::SUMMARY_WELL, "Water component molar production rate"}} );
    m_summaryToDescMap.insert( {"WWMPT", {A::SUMMARY_WELL, "Water component molar production total"}} );
    m_summaryToDescMap.insert( {"WWMIR", {A::SUMMARY_WELL, "Water component molar injection rate"}} );
    m_summaryToDescMap.insert( {"WWMIT", {A::SUMMARY_WELL, "Water component molar injection total"}} );
    m_summaryToDescMap.insert( {"WGPR", {A::SUMMARY_WELL, "Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WGPRA", {A::SUMMARY_WELL, "Gas Production Rate above"}} );
    m_summaryToDescMap.insert( {"WGPRB", {A::SUMMARY_WELL, "Gas Production Rate below"}} );
    m_summaryToDescMap.insert( {"WGPTA", {A::SUMMARY_WELL, "Gas Production Total above"}} );
    m_summaryToDescMap.insert( {"WGPTB", {A::SUMMARY_WELL, "Gas Production Total below"}} );
    m_summaryToDescMap.insert( {"WGPR1", {A::SUMMARY_WELL, "Gas Production Rate above GOC"}} );
    m_summaryToDescMap.insert( {"WGPR2", {A::SUMMARY_WELL, "Gas Production Rate below GOC"}} );
    m_summaryToDescMap.insert( {"WGPT1", {A::SUMMARY_WELL, "Gas Production Total above GOC"}} );
    m_summaryToDescMap.insert( {"WGPT2", {A::SUMMARY_WELL, "Gas Production Total below GOC"}} );
    m_summaryToDescMap.insert( {"WGMR", {A::SUMMARY_WELL, "Gas Mass Rate"}} );
    m_summaryToDescMap.insert( {"WGMT", {A::SUMMARY_WELL, "Gas Mass Total"}} );
    m_summaryToDescMap.insert( {"WGDN", {A::SUMMARY_WELL, "Gas Density at Surface Conditions"}} );
    m_summaryToDescMap.insert( {"WGPRH", {A::SUMMARY_WELL, "Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"WGPRT", {A::SUMMARY_WELL, "Gas Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WGPRF", {A::SUMMARY_WELL, "Free Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WGPRS", {A::SUMMARY_WELL, "Solution Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WGPT", {A::SUMMARY_WELL, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WGPTH", {A::SUMMARY_WELL, "Gas Production Total History"}} );
    m_summaryToDescMap.insert( {"WGPTF", {A::SUMMARY_WELL, "Free Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WGPTS", {A::SUMMARY_WELL, "Solution Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WGIR", {A::SUMMARY_WELL, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"WGIRH", {A::SUMMARY_WELL, "Gas Injection Rate History"}} );
    m_summaryToDescMap.insert( {"WGIRT", {A::SUMMARY_WELL, "Gas Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WGIT", {A::SUMMARY_WELL, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"WGITH", {A::SUMMARY_WELL, "Gas Injection Total History"}} );
    m_summaryToDescMap.insert( {"WGPP", {A::SUMMARY_WELL, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WGPP2", {A::SUMMARY_WELL, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WGPPS", {A::SUMMARY_WELL, "Solution"}} );
    m_summaryToDescMap.insert( {"WGPPS2", {A::SUMMARY_WELL, "Solution"}} );
    m_summaryToDescMap.insert( {"WGPPF", {A::SUMMARY_WELL, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WGPPF2", {A::SUMMARY_WELL, "Free Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"WGPI", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WGIP", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WGPI2", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WGIP2", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"WGPGR", {A::SUMMARY_WELL, "Gas Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"WGIGR", {A::SUMMARY_WELL, "Gas Injection Guide Rate"}} );
    m_summaryToDescMap.insert( {"WGLIR", {A::SUMMARY_WELL, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"WWGPR", {A::SUMMARY_WELL, "Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WWGPT", {A::SUMMARY_WELL, "Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WWGPRH", {A::SUMMARY_WELL, "Wet Gas Production Rate History"}} );
    m_summaryToDescMap.insert( {"WWGIR", {A::SUMMARY_WELL, "Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"WWGIT", {A::SUMMARY_WELL, "Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"WGnPR", {A::SUMMARY_WELL, "nth separator stage gas rate"}} );
    m_summaryToDescMap.insert( {"WGnPT", {A::SUMMARY_WELL, "nth separator stage gas total"}} );
    m_summaryToDescMap.insert( {"WGVPR", {A::SUMMARY_WELL, "Gas Voidage Production Rate"}} );
    m_summaryToDescMap.insert( {"WGVPT", {A::SUMMARY_WELL, "Gas Voidage Production Total"}} );
    m_summaryToDescMap.insert( {"WGVIR", {A::SUMMARY_WELL, "Gas Voidage Injection Rate"}} );
    m_summaryToDescMap.insert( {"WGVIT", {A::SUMMARY_WELL, "Gas Voidage Injection Total"}} );
    m_summaryToDescMap.insert( {"WGQ", {A::SUMMARY_WELL, "Gas Quality"}} );
    m_summaryToDescMap.insert( {"WLPR", {A::SUMMARY_WELL, "Liquid Production Rate"}} );
    m_summaryToDescMap.insert( {"WLPRH", {A::SUMMARY_WELL, "Liquid Production Rate History"}} );
    m_summaryToDescMap.insert( {"WLPRT", {A::SUMMARY_WELL, "Liquid Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WLPT", {A::SUMMARY_WELL, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"WLPTH", {A::SUMMARY_WELL, "Liquid Production Total History"}} );
    m_summaryToDescMap.insert( {"WVPR", {A::SUMMARY_WELL, "Res Volume Production Rate"}} );
    m_summaryToDescMap.insert( {"WVPRT", {A::SUMMARY_WELL, "Res Volume Production Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WVPT", {A::SUMMARY_WELL, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"WVPGR", {A::SUMMARY_WELL, "Res Volume Production Guide Rate"}} );
    m_summaryToDescMap.insert( {"WVIR", {A::SUMMARY_WELL, "Res Volume Injection Rate"}} );
    m_summaryToDescMap.insert( {"WVIRT", {A::SUMMARY_WELL, "Res Volume Injection Rate Target/Limit"}} );
    m_summaryToDescMap.insert( {"WVIT", {A::SUMMARY_WELL, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"WWCT", {A::SUMMARY_WELL, "Water Cut"}} );
    m_summaryToDescMap.insert( {"WWCTH", {A::SUMMARY_WELL, "Water Cut History"}} );
    m_summaryToDescMap.insert( {"WGOR", {A::SUMMARY_WELL, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"WGORH", {A::SUMMARY_WELL, "Gas-Oil Ratio History"}} );
    m_summaryToDescMap.insert( {"WOGR", {A::SUMMARY_WELL, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"WOGRH", {A::SUMMARY_WELL, "Oil-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"WWGR", {A::SUMMARY_WELL, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"WWGRH", {A::SUMMARY_WELL, "Water-Gas Ratio History"}} );
    m_summaryToDescMap.insert( {"WGLR", {A::SUMMARY_WELL, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"WGLRH", {A::SUMMARY_WELL, "Gas-Liquid Ratio History"}} );
    m_summaryToDescMap.insert( {"WBGLR", {A::SUMMARY_WELL, "Bottom hole Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"WBHP", {A::SUMMARY_WELL, "Bottom Hole Pressure"}} );
    m_summaryToDescMap.insert( {"WBHPH", {A::SUMMARY_WELL, "Bottom Hole Pressure History,"}} );
    m_summaryToDescMap.insert( {"WBHPT", {A::SUMMARY_WELL, "Bottom Hole Pressure Target/Limit"}} );
    m_summaryToDescMap.insert( {"WTHP", {A::SUMMARY_WELL, "Tubing Head Pressure"}} );
    m_summaryToDescMap.insert( {"WTHPH", {A::SUMMARY_WELL, "Tubing Head Pressure History,"}} );
    m_summaryToDescMap.insert( {"WPI", {A::SUMMARY_WELL, "Productivity Index of well’s preferred phase"}} );
    m_summaryToDescMap.insert( {"WPIO", {A::SUMMARY_WELL, "Oil phase PI"}} );
    m_summaryToDescMap.insert( {"WPIG", {A::SUMMARY_WELL, "Gas phase PI"}} );
    m_summaryToDescMap.insert( {"WPIW", {A::SUMMARY_WELL, "Water phase PI"}} );
    m_summaryToDescMap.insert( {"WPIL", {A::SUMMARY_WELL, "Liquid phase PI"}} );
    m_summaryToDescMap.insert( {"WBP", {A::SUMMARY_WELL, "One-point Pressure Average"}} );
    m_summaryToDescMap.insert( {"WBP4", {A::SUMMARY_WELL, "Four-point Pressure Average"}} );
    m_summaryToDescMap.insert( {"WBP5", {A::SUMMARY_WELL, "Five-point Pressure Average"}} );
    m_summaryToDescMap.insert( {"WBP9", {A::SUMMARY_WELL, "Nine-point Pressure Average"}} );
    m_summaryToDescMap.insert( {"WPI1", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP"}} );
    m_summaryToDescMap.insert( {"WPI4", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP4"}} );
    m_summaryToDescMap.insert( {"WPI5", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP5"}} );
    m_summaryToDescMap.insert( {"WPI9", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP9"}} );
    m_summaryToDescMap.insert(
        {"WHD",
         {A::SUMMARY_WELL,
          "Hydraulic head in well based on the reference depth given in HYDRHEAD and the well’s reference depth"}} );
    m_summaryToDescMap.insert(
        {"WHDF",
         {A::SUMMARY_WELL,
          "Hydraulic head in well based on the reference depth given in HYDRHEAD and the well’s reference depth "
          "calculated at freshwater conditions"}} );
    m_summaryToDescMap.insert( {"WSTAT", {A::SUMMARY_WELL, "Well State Indicator"}} );
    m_summaryToDescMap.insert( {"WMCTL", {A::SUMMARY_WELL, "Mode of Control"}} );
    m_summaryToDescMap.insert( {"WMCON", {A::SUMMARY_WELL, "The number of connections capable of flowing in the well"}} );
    m_summaryToDescMap.insert( {"WEPR", {A::SUMMARY_WELL, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"WEPT", {A::SUMMARY_WELL, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"WEFF", {A::SUMMARY_WELL, "Efficiency Factor"}} );
    m_summaryToDescMap.insert(
        {"WEFFG", {A::SUMMARY_WELL, "Product of efficiency factors of the well and all its superior groups"}} );
    m_summaryToDescMap.insert( {"WALQ", {A::SUMMARY_WELL, "Well Artificial Lift Quantity"}} );
    m_summaryToDescMap.insert( {"WMVFP", {A::SUMMARY_WELL, "VFP table number used by the well"}} );
    m_summaryToDescMap.insert( {"WNLPR", {A::SUMMARY_WELL, "NGL Production Rate"}} );
    m_summaryToDescMap.insert( {"WNLPT", {A::SUMMARY_WELL, "NGL Production Total"}} );
    m_summaryToDescMap.insert( {"WNLPRH", {A::SUMMARY_WELL, "NGL Production Rate History"}} );
    m_summaryToDescMap.insert( {"WNLPTH", {A::SUMMARY_WELL, "NGL Production Total History"}} );
    m_summaryToDescMap.insert( {"WNLPRT", {A::SUMMARY_WELL, "NGL Production Rate Target"}} );
    m_summaryToDescMap.insert(
        {"WAMF", {A::SUMMARY_WELL, "Component aqueous mole fraction, from producing completions"}} );
    m_summaryToDescMap.insert( {"WXMF", {A::SUMMARY_WELL, "Liquid Mole Fraction"}} );
    m_summaryToDescMap.insert( {"WYMF", {A::SUMMARY_WELL, "Vapor Mole Fraction"}} );
    m_summaryToDescMap.insert( {"WXMFn", {A::SUMMARY_WELL, "Liquid Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"WYMFn", {A::SUMMARY_WELL, "Vapor Mole Fraction for nth separator stage"}} );
    m_summaryToDescMap.insert( {"WZMF", {A::SUMMARY_WELL, "Total Mole Fraction"}} );
    m_summaryToDescMap.insert( {"WCMPR", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Production Rates"}} );
    m_summaryToDescMap.insert( {"WCMPT", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"WCMIR", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Injection Rates"}} );
    m_summaryToDescMap.insert( {"WCMIT", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Injection Totals"}} );
    m_summaryToDescMap.insert( {"WCGIR", {A::SUMMARY_WELL, "Hydrocarbon Component Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"WCGPR", {A::SUMMARY_WELL, "Hydrocarbon Component Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WCOPR", {A::SUMMARY_WELL, "Hydrocarbon Component Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"WHMIR", {A::SUMMARY_WELL, "Hydrocarbon Molar Injection Rate"}} );
    m_summaryToDescMap.insert( {"WHMIT", {A::SUMMARY_WELL, "Hydrocarbon Molar Injection Total"}} );
    m_summaryToDescMap.insert( {"WHMPR", {A::SUMMARY_WELL, "Hydrocarbon Molar Production Rate"}} );
    m_summaryToDescMap.insert( {"WHMPT", {A::SUMMARY_WELL, "Hydrocarbon Molar Production Total"}} );
    m_summaryToDescMap.insert( {"WCHMR", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"WCHMT", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"WCWGPR", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Production Rate"}} );
    m_summaryToDescMap.insert( {"WCWGPT", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WCWGIR", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"WCWGIT", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"WCGMR", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WCGMT", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WCOMR", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WCOMT", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WCNMR", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the NGL phase"}} );
    m_summaryToDescMap.insert( {"WCNWR", {A::SUMMARY_WELL, "Hydrocarbon component mass rates in the NGL phase"}} );
    m_summaryToDescMap.insert(
        {"WCGMRn", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert(
        {"WCGRn", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    m_summaryToDescMap.insert( {"WCOMRn", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WCORn", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    m_summaryToDescMap.insert( {"WMUF", {A::SUMMARY_WELL, "Make-up fraction"}} );
    m_summaryToDescMap.insert( {"WTHT", {A::SUMMARY_WELL, "Tubing Head Temperature"}} );
    m_summaryToDescMap.insert( {"WMMW", {A::SUMMARY_WELL, "Mean molecular weight of wellstream"}} );
    m_summaryToDescMap.insert( {"WPWE0", {A::SUMMARY_WELL, "Well drilled indicator"}} );
    m_summaryToDescMap.insert( {"WPWE1", {A::SUMMARY_WELL, "Connections opened indicator"}} );
    m_summaryToDescMap.insert( {"WPWE2", {A::SUMMARY_WELL, "Connections closed indicator"}} );
    m_summaryToDescMap.insert( {"WPWE3", {A::SUMMARY_WELL, "Connections closed to bottom indicator"}} );
    m_summaryToDescMap.insert( {"WPWE4", {A::SUMMARY_WELL, "Well stopped indicator"}} );
    m_summaryToDescMap.insert( {"WPWE5", {A::SUMMARY_WELL, "Injector to producer indicator"}} );
    m_summaryToDescMap.insert( {"WPWE6", {A::SUMMARY_WELL, "Producer to injector indicator"}} );
    m_summaryToDescMap.insert( {"WPWE7", {A::SUMMARY_WELL, "Well shut indicator"}} );
    m_summaryToDescMap.insert( {"WPWEM", {A::SUMMARY_WELL, "WELEVNT output mnemonic"}} );
    m_summaryToDescMap.insert( {"WDRPR", {A::SUMMARY_WELL, "Well drilling priority"}} );
    m_summaryToDescMap.insert( {"WBHWCn", {A::SUMMARY_WELL, "Derivative of well BHP with respect to parameter n"}} );
    m_summaryToDescMap.insert(
        {"WGFWCn", {A::SUMMARY_WELL, "Derivative of well gas flow rate with respect to parameter n"}} );
    m_summaryToDescMap.insert(
        {"WOFWCn", {A::SUMMARY_WELL, "Derivative of well oil flow rate with respect to parameter n"}} );
    m_summaryToDescMap.insert(
        {"WWFWCn", {A::SUMMARY_WELL, "Derivative of water flow rate with respect to parameter n"}} );
    m_summaryToDescMap.insert( {"WTPR", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPT", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"WTPC", {A::SUMMARY_WELL, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"WTIR", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTIT", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"WTIC", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTMR", {A::SUMMARY_WELL, "Traced mass Rate"}} );
    m_summaryToDescMap.insert( {"WTMT", {A::SUMMARY_WELL, "Traced mass Total"}} );
    m_summaryToDescMap.insert( {"WTQR", {A::SUMMARY_WELL, "Traced molar Rate"}} );
    m_summaryToDescMap.insert( {"WTCM", {A::SUMMARY_WELL, "Tracer Carrier molar Rate"}} );
    m_summaryToDescMap.insert( {"WTMF", {A::SUMMARY_WELL, "Traced molar fraction"}} );
    m_summaryToDescMap.insert( {"WTVL", {A::SUMMARY_WELL, "Traced liquid volume rate"}} );
    m_summaryToDescMap.insert( {"WTVV", {A::SUMMARY_WELL, "Traced vapor volume rate"}} );
    m_summaryToDescMap.insert( {"WTTL", {A::SUMMARY_WELL, "Traced liquid volume total"}} );
    m_summaryToDescMap.insert( {"WTTV", {A::SUMMARY_WELL, "Traced vapor volume total"}} );
    m_summaryToDescMap.insert( {"WTML", {A::SUMMARY_WELL, "Traced mass liquid rate"}} );
    m_summaryToDescMap.insert( {"WTMV", {A::SUMMARY_WELL, "Traced mass vapor rate"}} );
    m_summaryToDescMap.insert( {"WTLM", {A::SUMMARY_WELL, "Traced mass liquid total"}} );
    m_summaryToDescMap.insert( {"WTVM", {A::SUMMARY_WELL, "Traced mass vapor total"}} );
    m_summaryToDescMap.insert( {"WAPI", {A::SUMMARY_WELL, "Oil API"}} );
    m_summaryToDescMap.insert( {"WSPR", {A::SUMMARY_WELL, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"WSPT", {A::SUMMARY_WELL, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"WSIR", {A::SUMMARY_WELL, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"WSIT", {A::SUMMARY_WELL, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"WSPC", {A::SUMMARY_WELL, "Salt Production Concentration"}} );
    m_summaryToDescMap.insert( {"WSIC", {A::SUMMARY_WELL, "Salt Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTPCHEA", {A::SUMMARY_WELL, "Production Temperature"}} );
    m_summaryToDescMap.insert( {"WTICHEA", {A::SUMMARY_WELL, "Injection Temperature"}} );
    m_summaryToDescMap.insert( {"WTPRHEA", {A::SUMMARY_WELL, "Energy flows"}} );
    m_summaryToDescMap.insert( {"WTPTHEA", {A::SUMMARY_WELL, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRHEA", {A::SUMMARY_WELL, "Energy flows"}} );
    m_summaryToDescMap.insert( {"WTITHEA", {A::SUMMARY_WELL, "Energy Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPR", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPT", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"WTPC", {A::SUMMARY_WELL, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"WTIR", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTIT", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"WTIC", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTIRF", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTIRS", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTPRF", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPRS", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"WTITF", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"WTITS", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPTF", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"WTPTS", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"WTICF", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTICS", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WTPCF", {A::SUMMARY_WELL, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"WTPCS", {A::SUMMARY_WELL, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"WMPR", {A::SUMMARY_WELL, "Methane Production Rate"}} );
    m_summaryToDescMap.insert( {"WMPT", {A::SUMMARY_WELL, "Methane Production Total"}} );
    m_summaryToDescMap.insert( {"WMIR", {A::SUMMARY_WELL, "Methane Injection Rate"}} );
    m_summaryToDescMap.insert( {"WMIT", {A::SUMMARY_WELL, "Methane Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPRFOA", {A::SUMMARY_WELL, "Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPTFOA", {A::SUMMARY_WELL, "Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRFOA", {A::SUMMARY_WELL, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTITFOA", {A::SUMMARY_WELL, "Injection Total"}} );
    m_summaryToDescMap.insert( {"WGDC", {A::SUMMARY_WELL, "Gas Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"NGOPAS", {A::SUMMARY_WELL, "Number of iterations to converge DCQ in first pass"}} );
    m_summaryToDescMap.insert( {"WGPRFP", {A::SUMMARY_WELL, "Well Gas Production Rate from end of First Pass"}} );
    m_summaryToDescMap.insert( {"WTHPFP", {A::SUMMARY_WELL, "Well Tubing Head Pressure from end of First Pass"}} );
    m_summaryToDescMap.insert( {"WBHPFP", {A::SUMMARY_WELL, "Well Bottom Hole Pressure from end of First Pass"}} );
    m_summaryToDescMap.insert( {"WGLIR", {A::SUMMARY_WELL, "Gas Lift Injection Rate"}} );
    m_summaryToDescMap.insert( {"WOGLR", {A::SUMMARY_WELL, "Well Oil Gas Lift Ratio"}} );
    m_summaryToDescMap.insert( {"WGCV", {A::SUMMARY_WELL, "Gas Calorific Value"}} );
    m_summaryToDescMap.insert( {"WGQ", {A::SUMMARY_WELL, "Gas molar Quality"}} );
    m_summaryToDescMap.insert( {"WEPR", {A::SUMMARY_WELL, "Energy Production Rate"}} );
    m_summaryToDescMap.insert( {"WEPT", {A::SUMMARY_WELL, "Energy Production Total"}} );
    m_summaryToDescMap.insert( {"WEDC", {A::SUMMARY_WELL, "Energy Delivery Capacity"}} );
    m_summaryToDescMap.insert( {"WCPR", {A::SUMMARY_WELL, "Polymer Production Rate"}} );
    m_summaryToDescMap.insert( {"WCPC", {A::SUMMARY_WELL, "Polymer Production Concentration"}} );
    m_summaryToDescMap.insert( {"WCPT", {A::SUMMARY_WELL, "Polymer Production Total"}} );
    m_summaryToDescMap.insert( {"WCIR", {A::SUMMARY_WELL, "Polymer Injection Rate"}} );
    m_summaryToDescMap.insert( {"WCIC", {A::SUMMARY_WELL, "Polymer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"WCIT", {A::SUMMARY_WELL, "Polymer Injection Total"}} );
    m_summaryToDescMap.insert( {"WSPR", {A::SUMMARY_WELL, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"WSPT", {A::SUMMARY_WELL, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"WSIR", {A::SUMMARY_WELL, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"WSIT", {A::SUMMARY_WELL, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"WNPR", {A::SUMMARY_WELL, "Solvent Production Rate"}} );
    m_summaryToDescMap.insert( {"WNPT", {A::SUMMARY_WELL, "Solvent Production Total"}} );
    m_summaryToDescMap.insert( {"WNIR", {A::SUMMARY_WELL, "Solvent Injection Rate"}} );
    m_summaryToDescMap.insert( {"WNIT", {A::SUMMARY_WELL, "Solvent Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPRSUR", {A::SUMMARY_WELL, "Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPTSUR", {A::SUMMARY_WELL, "Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRSUR", {A::SUMMARY_WELL, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTITSUR", {A::SUMMARY_WELL, "Injection Total"}} );
    m_summaryToDescMap.insert( {"WTPRALK", {A::SUMMARY_WELL, "Production Rate"}} );
    m_summaryToDescMap.insert( {"WTPTALK", {A::SUMMARY_WELL, "Production Total"}} );
    m_summaryToDescMap.insert( {"WTIRALK", {A::SUMMARY_WELL, "Injection Rate"}} );
    m_summaryToDescMap.insert( {"WTITALK", {A::SUMMARY_WELL, "Injection Total"}} );
    m_summaryToDescMap.insert( {"WU", {A::SUMMARY_WELL, "User-defined well quantity"}} );

    // Future CONNECTION vectors
    m_summaryToDescMap.insert( {"COFR", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"COFRF", {A::SUMMARY_WELL_COMPLETION, "Free Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"COFRS", {A::SUMMARY_WELL_COMPLETION, "Solution oil flow rate"}} );
    m_summaryToDescMap.insert(
        {"COFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection oil flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert( {"COPR", {A::SUMMARY_WELL_COMPLETION, "Oil Production Rate"}} );
    m_summaryToDescMap.insert( {"COPT", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"COPTF", {A::SUMMARY_WELL_COMPLETION, "Free Oil Production Total"}} );
    m_summaryToDescMap.insert( {"COPTS", {A::SUMMARY_WELL_COMPLETION, "Solution Oil Production Total"}} );
    m_summaryToDescMap.insert( {"COIT", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"COPP", {A::SUMMARY_WELL_COMPLETION, "Oil Potential Production rate"}} );
    m_summaryToDescMap.insert( {"COPI", {A::SUMMARY_WELL_COMPLETION, "Oil Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"CWFR", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"CWFRU",
                                {A::SUMMARY_WELL_COMPLETION,
                                 "Sum of connection water flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert( {"CWPR", {A::SUMMARY_WELL_COMPLETION, "Water Production Rate"}} );
    m_summaryToDescMap.insert( {"CWPT", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"CWIR", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"CWIT", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"CWPP", {A::SUMMARY_WELL_COMPLETION, "Water Potential Production rate"}} );
    m_summaryToDescMap.insert( {"CWPI", {A::SUMMARY_WELL_COMPLETION, "Water Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"CGFR", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"CGFRF", {A::SUMMARY_WELL_COMPLETION, "Free Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"CGFRS", {A::SUMMARY_WELL_COMPLETION, "Solution Gas Flow Rate"}} );
    m_summaryToDescMap.insert(
        {"CGFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection gas flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert( {"CGPR", {A::SUMMARY_WELL_COMPLETION, "Gas Production Rate "}} );
    m_summaryToDescMap.insert( {"CGPT", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"CGPTF", {A::SUMMARY_WELL_COMPLETION, "Free Gas Production Total"}} );
    m_summaryToDescMap.insert( {"CGPTS", {A::SUMMARY_WELL_COMPLETION, "Solution Gas Production Total"}} );
    m_summaryToDescMap.insert( {"CGIR", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"CGIT", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"CGPP", {A::SUMMARY_WELL_COMPLETION, "Gas Potential Production rate"}} );
    m_summaryToDescMap.insert( {"CGPI", {A::SUMMARY_WELL_COMPLETION, "Gas Potential Injection rate"}} );
    m_summaryToDescMap.insert( {"CGQ", {A::SUMMARY_WELL_COMPLETION, "Gas Quality"}} );
    m_summaryToDescMap.insert( {"CLFR", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    m_summaryToDescMap.insert( {"CLPT", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"CVFR", {A::SUMMARY_WELL_COMPLETION, "Reservoir"}} );
    m_summaryToDescMap.insert( {"CVPR", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Rate"}} );
    m_summaryToDescMap.insert( {"CVPT", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"CVIR", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Rate"}} );
    m_summaryToDescMap.insert( {"CVIT", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"CWCT", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    m_summaryToDescMap.insert( {"CGOR", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"COGR", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"CWGR", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"CGLR", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"CPR", {A::SUMMARY_WELL_COMPLETION, "Connection Pressure"}} );
    m_summaryToDescMap.insert( {"CPI", {A::SUMMARY_WELL_COMPLETION, "Productivity Index of well’s preferred phase"}} );
    m_summaryToDescMap.insert( {"CTFAC", {A::SUMMARY_WELL_COMPLETION, "Connection Transmissibility Factor"}} );
    m_summaryToDescMap.insert(
        {"CDBF", {A::SUMMARY_WELL_COMPLETION, "Blocking factor for generalized pseudo-pressure method"}} );
    m_summaryToDescMap.insert(
        {"CGPPTN", {A::SUMMARY_WELL_COMPLETION, "Generalized pseudo-pressure table update counter"}} );
    m_summaryToDescMap.insert(
        {"CGPPTS", {A::SUMMARY_WELL_COMPLETION, "Generalized pseudo-pressure table update status"}} );
    m_summaryToDescMap.insert( {"CDSM", {A::SUMMARY_WELL_COMPLETION, "Current mass of scale deposited"}} );
    m_summaryToDescMap.insert(
        {"CDSML", {A::SUMMARY_WELL_COMPLETION, "Current mass of scale deposited per unit perforation length"}} );
    m_summaryToDescMap.insert( {"CDSF", {A::SUMMARY_WELL_COMPLETION, "PI multiplicative factor due to scale damage"}} );
    m_summaryToDescMap.insert(
        {"CAMF", {A::SUMMARY_WELL_COMPLETION, "Component aqueous mole fraction, from producing completions"}} );
    m_summaryToDescMap.insert( {"CZMF", {A::SUMMARY_WELL_COMPLETION, "Total Mole Fraction"}} );
    m_summaryToDescMap.insert( {"CKFR", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"CKFT", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"CDFAC", {A::SUMMARY_WELL_COMPLETION, "D-factor for flow dependent skin factor"}} );
    m_summaryToDescMap.insert( {"CTFR", {A::SUMMARY_WELL_COMPLETION, "Tracer Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPR", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"CTPT", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"CTPC", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"CTIR", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"CTIT", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"CTIC", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CAPI", {A::SUMMARY_WELL_COMPLETION, "Oil API"}} );
    m_summaryToDescMap.insert( {"CSFR", {A::SUMMARY_WELL_COMPLETION, "Salt Flow Rate"}} );
    m_summaryToDescMap.insert( {"CSPR", {A::SUMMARY_WELL_COMPLETION, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"CSPT", {A::SUMMARY_WELL_COMPLETION, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"CSIR", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"CSIT", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"CSPC", {A::SUMMARY_WELL_COMPLETION, "Salt Production Concentration"}} );
    m_summaryToDescMap.insert( {"CSIC", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CTFRANI", {A::SUMMARY_WELL_COMPLETION, "Anion Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPTANI", {A::SUMMARY_WELL_COMPLETION, "Anion Production Total"}} );
    m_summaryToDescMap.insert( {"CTITANI", {A::SUMMARY_WELL_COMPLETION, "Anion Injection Total"}} );
    m_summaryToDescMap.insert( {"CTFRCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPTCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Production Total"}} );
    m_summaryToDescMap.insert( {"CTITCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Injection Total"}} );
    m_summaryToDescMap.insert( {"CTFR", {A::SUMMARY_WELL_COMPLETION, "Tracer Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPR", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"CTPT", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"CTPC", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Concentration"}} );
    m_summaryToDescMap.insert( {"CTIR", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"CTIT", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"CTIC", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CTIRF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"CTIRS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    m_summaryToDescMap.insert( {"CTPRF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"CTPRS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    m_summaryToDescMap.insert( {"CTITF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"CTITS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    m_summaryToDescMap.insert( {"CTPTF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"CTPTS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    m_summaryToDescMap.insert( {"CTICF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CTICS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CTPCF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"CTPCS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production"}} );
    m_summaryToDescMap.insert( {"CTFRFOA", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPTFOA", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    m_summaryToDescMap.insert( {"CTITFOA", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    m_summaryToDescMap.insert( {"CRREXCH", {A::SUMMARY_WELL_COMPLETION, "Exchange flux at current time"}} );
    m_summaryToDescMap.insert( {"CRRPROT", {A::SUMMARY_WELL_COMPLETION, "Connection cumulative water production"}} );
    m_summaryToDescMap.insert( {"CRRINJT", {A::SUMMARY_WELL_COMPLETION, "Connection cumulative water injection"}} );
    m_summaryToDescMap.insert( {"CCFR", {A::SUMMARY_WELL_COMPLETION, "Polymer Flow Rate"}} );
    m_summaryToDescMap.insert( {"CCPR", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Rate"}} );
    m_summaryToDescMap.insert( {"CCPC", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Concentration"}} );
    m_summaryToDescMap.insert( {"CCPT", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Total"}} );
    m_summaryToDescMap.insert( {"CCIR", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Rate"}} );
    m_summaryToDescMap.insert( {"CCIC", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Concentration"}} );
    m_summaryToDescMap.insert( {"CCIT", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Total"}} );
    m_summaryToDescMap.insert( {"CSFR", {A::SUMMARY_WELL_COMPLETION, "Salt Flow Rate"}} );
    m_summaryToDescMap.insert( {"CSPR", {A::SUMMARY_WELL_COMPLETION, "Salt Production Rate"}} );
    m_summaryToDescMap.insert( {"CSPT", {A::SUMMARY_WELL_COMPLETION, "Salt Production Total"}} );
    m_summaryToDescMap.insert( {"CSIR", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Rate"}} );
    m_summaryToDescMap.insert( {"CSIT", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Total"}} );
    m_summaryToDescMap.insert( {"CNFR", {A::SUMMARY_WELL_COMPLETION, "Solvent Flow Rate"}} );
    m_summaryToDescMap.insert( {"CNPT", {A::SUMMARY_WELL_COMPLETION, "Solvent Production Total"}} );
    m_summaryToDescMap.insert( {"CNIT", {A::SUMMARY_WELL_COMPLETION, "Solvent Injection Total"}} );
    m_summaryToDescMap.insert( {"CTFRSUR", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPTSUR", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    m_summaryToDescMap.insert( {"CTITSUR", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    m_summaryToDescMap.insert( {"CTFRALK", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    m_summaryToDescMap.insert( {"CTPTALK", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    m_summaryToDescMap.insert( {"CTITALK", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    m_summaryToDescMap.insert(
        {"COFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection oil flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert( {"CWFRU",
                                {A::SUMMARY_WELL_COMPLETION,
                                 "Sum of connection water flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert(
        {"CGFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection gas flow rates upstream of, and including, this connection"}} );
    m_summaryToDescMap.insert( {"LCOFRU", {A::SUMMARY_WELL_COMPLETION, "As COFRU but for local grids"}} );
    m_summaryToDescMap.insert( {"LCWFRU", {A::SUMMARY_WELL_COMPLETION, "As CWFRU but for local grids"}} );
    m_summaryToDescMap.insert( {"LCGFRU", {A::SUMMARY_WELL_COMPLETION, "As CGFRU but for local grids"}} );
    m_summaryToDescMap.insert( {"CU", {A::SUMMARY_WELL_COMPLETION, "User-defined connection quantity"}} );

    m_summaryToDescMap.insert( {"COFRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"WOFRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"COPRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"WOPRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"COPTL", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"WOPTL", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    m_summaryToDescMap.insert( {"COITL", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"WOITL", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    m_summaryToDescMap.insert( {"CWFRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"WWFRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"CWPRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"WWPRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"CWPTL", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"WWPTL", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    m_summaryToDescMap.insert( {"CWIRL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"WWIRL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    m_summaryToDescMap.insert( {"CWITL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"WWITL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    m_summaryToDescMap.insert( {"CGFRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"WGFRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"CGPRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"WGPRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"CGPTL", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"WGPTL", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    m_summaryToDescMap.insert( {"CGIRL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"WGIRL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    m_summaryToDescMap.insert( {"CGITL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"WGITL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    m_summaryToDescMap.insert( {"CLFRL", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    m_summaryToDescMap.insert( {"WLFRL", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    m_summaryToDescMap.insert( {"CLPTL", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"WLPTL", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    m_summaryToDescMap.insert( {"CVFRL", {A::SUMMARY_WELL_COMPLETION, "Reservoir"}} );
    m_summaryToDescMap.insert( {"WVFRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Flow Rate"}} );
    m_summaryToDescMap.insert( {"CVPRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Flow Rate"}} );
    m_summaryToDescMap.insert( {"WVPRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Flow Rate"}} );
    m_summaryToDescMap.insert( {"CVIRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Flow Rate"}} );
    m_summaryToDescMap.insert( {"WVIRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Flow Rate"}} );
    m_summaryToDescMap.insert( {"CVPTL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"WVPTL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    m_summaryToDescMap.insert( {"CVITL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"WVITL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    m_summaryToDescMap.insert( {"CWCTL", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    m_summaryToDescMap.insert( {"WWCTL", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    m_summaryToDescMap.insert( {"CGORL", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"WGORL", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    m_summaryToDescMap.insert( {"COGRL", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"WOGRL", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"CWGRL", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"WWGRL", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    m_summaryToDescMap.insert( {"CGLRL", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"WGLRL", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    m_summaryToDescMap.insert( {"CPRL", {A::SUMMARY_WELL_COMPLETION, "Average Connection Pressure in completion"}} );
    m_summaryToDescMap.insert( {"CKFRL", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    m_summaryToDescMap.insert( {"CKFTL", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );

    m_summaryToDescMap.insert( {"RPR", {A::SUMMARY_REGION, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"RPRH", {A::SUMMARY_REGION, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"RPRP", {A::SUMMARY_REGION, "Pressure average value"}} );
    m_summaryToDescMap.insert( {"RPRGZ", {A::SUMMARY_REGION, "P/Z"}} );
    m_summaryToDescMap.insert( {"RRS", {A::SUMMARY_REGION, "Gas-oil ratio"}} );
    m_summaryToDescMap.insert( {"RRV", {A::SUMMARY_REGION, "Oil-gas ratio"}} );
    m_summaryToDescMap.insert( {"RPPC", {A::SUMMARY_REGION, "Initial Contact Corrected Potential"}} );
    m_summaryToDescMap.insert( {"RRPV", {A::SUMMARY_REGION, "Pore Volume at Reservoir conditions"}} );
    m_summaryToDescMap.insert( {"ROPV", {A::SUMMARY_REGION, "Pore Volume containing Oil"}} );
    m_summaryToDescMap.insert( {"RWPV", {A::SUMMARY_REGION, "Pore Volume containing Water"}} );
    m_summaryToDescMap.insert( {"RGPV", {A::SUMMARY_REGION, "Pore Volume containing Gas"}} );
    m_summaryToDescMap.insert( {"RHPV", {A::SUMMARY_REGION, "Pore Volume containing Hydrocarbon"}} );
    m_summaryToDescMap.insert(
        {"RRTM", {A::SUMMARY_REGION, "Transmissibility Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert( {"ROE", {A::SUMMARY_REGION, "(OIP(initial - OIP(now) / OIP(initial)"}} );
    m_summaryToDescMap.insert( {"ROEW", {A::SUMMARY_REGION, "Oil Production from Wells / OIP(initial)"}} );
    m_summaryToDescMap.insert(
        {"ROEIW", {A::SUMMARY_REGION, "(OIP(initial - OIP(now) / Initial Mobile Oil with respect to Water"}} );
    m_summaryToDescMap.insert(
        {"ROEWW", {A::SUMMARY_REGION, "Oil Production from Wells / Initial Mobile Oil with respect to Water"}} );
    m_summaryToDescMap.insert(
        {"ROEIG", {A::SUMMARY_REGION, "(OIP(initial - OIP(now) / Initial Mobile Oil with respect to Gas"}} );
    m_summaryToDescMap.insert(
        {"ROEWG", {A::SUMMARY_REGION, "Oil Production from Wells / Initial Mobile Oil with respect to Gas"}} );
    m_summaryToDescMap.insert( {"RORMR", {A::SUMMARY_REGION, "Total stock tank oil produced by rock compaction"}} );
    m_summaryToDescMap.insert( {"RORMW", {A::SUMMARY_REGION, "Total stock tank oil produced by water influx"}} );
    m_summaryToDescMap.insert( {"RORMG", {A::SUMMARY_REGION, "Total stock tank oil produced by gas influx"}} );
    m_summaryToDescMap.insert( {"RORME", {A::SUMMARY_REGION, "Total stock tank oil produced by oil expansion"}} );
    m_summaryToDescMap.insert( {"RORMS", {A::SUMMARY_REGION, "Total stock tank oil produced by solution gas"}} );
    m_summaryToDescMap.insert( {"RORMF", {A::SUMMARY_REGION, "Total stock tank oil produced by free gas influx"}} );
    m_summaryToDescMap.insert( {"RORMX", {A::SUMMARY_REGION, "Total stock tank oil produced by 'traced' water influx"}} );
    m_summaryToDescMap.insert( {"RORMY", {A::SUMMARY_REGION, "Total stock tank oil produced by other water influx"}} );
    m_summaryToDescMap.insert( {"RORFR", {A::SUMMARY_REGION, "Fraction of total oil produced by rock compaction"}} );
    m_summaryToDescMap.insert( {"RORFW", {A::SUMMARY_REGION, "Fraction of total oil produced by water influx"}} );
    m_summaryToDescMap.insert( {"RORFG", {A::SUMMARY_REGION, "Fraction of total oil produced by gas influx"}} );
    m_summaryToDescMap.insert( {"RORFE", {A::SUMMARY_REGION, "Fraction of total oil produced by oil expansion"}} );
    m_summaryToDescMap.insert( {"RORFS", {A::SUMMARY_REGION, "Fraction of total oil produced by solution gas"}} );
    m_summaryToDescMap.insert( {"RORFF", {A::SUMMARY_REGION, "Fraction of total oil produced by free gas influx"}} );
    m_summaryToDescMap.insert( {"RORFX", {A::SUMMARY_REGION, "Fraction of total oil produced by 'traced' water influx"}} );
    m_summaryToDescMap.insert( {"RORFY", {A::SUMMARY_REGION, "Fraction of total oil produced by other water influx"}} );
    m_summaryToDescMap.insert( {"RTIPT", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RTIPF", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RTIPS", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RAPI", {A::SUMMARY_REGION, "Oil API"}} );
    m_summaryToDescMap.insert( {"RSIP", {A::SUMMARY_REGION, "Salt In Place"}} );
    m_summaryToDescMap.insert(
        {"RTIPTHEA", {A::SUMMARY_REGION, "Difference in Energy in place between current and initial time"}} );
    m_summaryToDescMap.insert( {"RTIPT", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RTIPF", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RTIPS", {A::SUMMARY_REGION, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"RTIP#", {A::SUMMARY_REGION, "Tracer In Place in phase # (1,2,3,...)"}} );
    m_summaryToDescMap.insert( {"RTADS", {A::SUMMARY_REGION, "Tracer Adsorption total"}} );
    m_summaryToDescMap.insert( {"RTDCY", {A::SUMMARY_REGION, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"RCGC", {A::SUMMARY_REGION, "Bulk Coal Gas Concentration"}} );
    m_summaryToDescMap.insert( {"RCSC", {A::SUMMARY_REGION, "Bulk Coal Solvent Concentration"}} );
    m_summaryToDescMap.insert( {"RTIPTFOA", {A::SUMMARY_REGION, "In Solution"}} );
    m_summaryToDescMap.insert( {"RTADSFOA", {A::SUMMARY_REGION, "Adsorption total"}} );
    m_summaryToDescMap.insert( {"RTDCYFOA", {A::SUMMARY_REGION, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"RTMOBFOA", {A::SUMMARY_REGION, "Gas mobility factor"}} );
    m_summaryToDescMap.insert( {"RCIP", {A::SUMMARY_REGION, "Polymer In Solution"}} );
    m_summaryToDescMap.insert( {"RCAD", {A::SUMMARY_REGION, "Polymer Adsorption total"}} );
    m_summaryToDescMap.insert( {"RSIP", {A::SUMMARY_REGION, "Salt In Place"}} );
    m_summaryToDescMap.insert( {"RNIP", {A::SUMMARY_REGION, "Solvent In Place"}} );
    m_summaryToDescMap.insert( {"RTIPTSUR", {A::SUMMARY_REGION, "In Solution"}} );
    m_summaryToDescMap.insert( {"RTADSUR", {A::SUMMARY_REGION, "Adsorption total"}} );
    m_summaryToDescMap.insert( {"RU", {A::SUMMARY_REGION, "User-defined region quantity"}} );

    m_summaryToDescMap.insert( {"ROFR", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    m_summaryToDescMap.insert( {"ROFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    m_summaryToDescMap.insert( {"ROFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    m_summaryToDescMap.insert( {"ROFT", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    m_summaryToDescMap.insert( {"ROFT+", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    m_summaryToDescMap.insert( {"ROFT-", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    m_summaryToDescMap.insert( {"ROFTL", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    m_summaryToDescMap.insert( {"ROFTG", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    m_summaryToDescMap.insert( {"RGFR", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    m_summaryToDescMap.insert( {"RGFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    m_summaryToDescMap.insert( {"RGFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    m_summaryToDescMap.insert( {"RGFT", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total)"}} );
    m_summaryToDescMap.insert( {"RGFT+", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    m_summaryToDescMap.insert( {"RGFT-", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    m_summaryToDescMap.insert( {"RGFTL", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    m_summaryToDescMap.insert( {"RGFTG", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    m_summaryToDescMap.insert( {"RWFR", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    m_summaryToDescMap.insert( {"RWFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    m_summaryToDescMap.insert( {"RWFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    m_summaryToDescMap.insert( {"RWFT", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow total"}} );
    m_summaryToDescMap.insert( {"RTFTF", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RTFTS", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RTFTT", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RSFT", {A::SUMMARY_REGION_2_REGION, "Salt inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RTFTT", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    m_summaryToDescMap.insert( {"RTFTF", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    m_summaryToDescMap.insert( {"RTFTS", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    m_summaryToDescMap.insert(
        {"RTFT#", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow in phase # (1,2,3,...)"}} );
    m_summaryToDescMap.insert( {"RTFTTFOA", {A::SUMMARY_REGION_2_REGION, "Inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RCFT", {A::SUMMARY_REGION_2_REGION, "Polymer inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RSFT", {A::SUMMARY_REGION_2_REGION, "Salt inter-region Flow Total"}} );
    m_summaryToDescMap.insert( {"RNFT", {A::SUMMARY_REGION_2_REGION, "Solvent inter-region Flow"}} );
    m_summaryToDescMap.insert( {"RTFTTSUR", {A::SUMMARY_REGION_2_REGION, "Inter-region Flow Total"}} );

    m_summaryToDescMap.insert( {"BPR", {A::SUMMARY_BLOCK, "Oil phase Pressure"}} );
    m_summaryToDescMap.insert( {"BPRESSUR", {A::SUMMARY_BLOCK, "Oil phase Pressure"}} );
    m_summaryToDescMap.insert( {"BWPR", {A::SUMMARY_BLOCK, "Water phase Pressure"}} );
    m_summaryToDescMap.insert( {"BGPR", {A::SUMMARY_BLOCK, "Gas phase Pressure"}} );
    m_summaryToDescMap.insert( {"BRS", {A::SUMMARY_BLOCK, "Gas-oil ratio"}} );
    m_summaryToDescMap.insert( {"BRV", {A::SUMMARY_BLOCK, "Oil-gas ratio"}} );
    m_summaryToDescMap.insert( {"BPBUB", {A::SUMMARY_BLOCK, "Bubble point pressure"}} );
    m_summaryToDescMap.insert( {"BPDEW", {A::SUMMARY_BLOCK, "Dew point pressure"}} );
    m_summaryToDescMap.insert( {"BRSSAT", {A::SUMMARY_BLOCK, "Saturated gas-oil ratio"}} );
    m_summaryToDescMap.insert( {"BRVSAT", {A::SUMMARY_BLOCK, "Saturated oil-gas ratio"}} );
    m_summaryToDescMap.insert( {"BSTATE", {A::SUMMARY_BLOCK, "Gas-oil state indicator"}} );
    m_summaryToDescMap.insert( {"BPPC", {A::SUMMARY_BLOCK, "Initial Contact Corrected Potential"}} );
    m_summaryToDescMap.insert( {"BOKR", {A::SUMMARY_BLOCK, "Oil relative permeability"}} );
    m_summaryToDescMap.insert( {"BWKR", {A::SUMMARY_BLOCK, "Water relative permeability"}} );
    m_summaryToDescMap.insert( {"BGKR", {A::SUMMARY_BLOCK, "Gas relative permeability"}} );
    m_summaryToDescMap.insert( {"BKRO", {A::SUMMARY_BLOCK, "Oil relative permeability"}} );
    m_summaryToDescMap.insert( {"BKROG", {A::SUMMARY_BLOCK, "Two-phase oil relative permeability to gas"}} );
    m_summaryToDescMap.insert( {"BKROW", {A::SUMMARY_BLOCK, "Two-phase oil relative permeability to water"}} );
    m_summaryToDescMap.insert( {"BKRG", {A::SUMMARY_BLOCK, "Gas relative permeability"}} );
    m_summaryToDescMap.insert( {"BKRGO", {A::SUMMARY_BLOCK, "Two-phase gas relative permeability to oil "}} );
    m_summaryToDescMap.insert( {"BKRGW", {A::SUMMARY_BLOCK, "Two-phase gas relative permeability to water"}} );
    m_summaryToDescMap.insert( {"BKRW", {A::SUMMARY_BLOCK, "Water relative permeability"}} );
    m_summaryToDescMap.insert( {"BKRWG", {A::SUMMARY_BLOCK, "Two-phase water relative permeability to gas"}} );
    m_summaryToDescMap.insert( {"BKRWO", {A::SUMMARY_BLOCK, "Two-phase water relative permeability to oil"}} );
    m_summaryToDescMap.insert(
        {"BRK", {A::SUMMARY_BLOCK, "Water relative permeability reduction factor due to polymer"}} );
    m_summaryToDescMap.insert( {"BEWKR", {A::SUMMARY_BLOCK, "Water effective relative permeability due to polymer"}} );
    m_summaryToDescMap.insert( {"BWPC", {A::SUMMARY_BLOCK, "Water-Oil capillary pressure"}} );
    m_summaryToDescMap.insert( {"BGPC", {A::SUMMARY_BLOCK, "Gas-Oil capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCO", {A::SUMMARY_BLOCK, "Oil Capillary Pressures"}} );
    m_summaryToDescMap.insert( {"BPCG", {A::SUMMARY_BLOCK, "Gas Capillary Pressures"}} );
    m_summaryToDescMap.insert( {"BPCW", {A::SUMMARY_BLOCK, "Water Capillary Pressures"}} );
    m_summaryToDescMap.insert( {"BGTRP", {A::SUMMARY_BLOCK, "Trapped gas saturation"}} );
    m_summaryToDescMap.insert( {"BGTPD", {A::SUMMARY_BLOCK, "Dynamic trapped gas saturation"}} );
    m_summaryToDescMap.insert(
        {"BGSHY",
         {A::SUMMARY_BLOCK, "Departure saturation from drainage to imbibition for gas capillary pressure hysteresis"}} );
    m_summaryToDescMap.insert(
        {"BGSTRP", {A::SUMMARY_BLOCK, "Trapped gas critical saturation for gas capillary pressure hysteresis"}} );
    m_summaryToDescMap.insert(
        {"BWSHY",
         {A::SUMMARY_BLOCK, "Departure saturation from drainage to imbibition for water capillary pressure hysteresis"}} );
    m_summaryToDescMap.insert(
        {"BWSMA", {A::SUMMARY_BLOCK, "Maximum wetting saturation for water capillary pressure hysteresis"}} );
    m_summaryToDescMap.insert( {"BMLSC", {A::SUMMARY_BLOCK, "Hydrocarbon molar density"}} );
    m_summaryToDescMap.insert( {"BMLST", {A::SUMMARY_BLOCK, "Total hydrocarbon molar density"}} );
    m_summaryToDescMap.insert( {"BMWAT", {A::SUMMARY_BLOCK, "Water molar density"}} );
    m_summaryToDescMap.insert( {"BROMLS", {A::SUMMARY_BLOCK, "Residual oil moles/ reservoir volume"}} );
    m_summaryToDescMap.insert( {"BJV", {A::SUMMARY_BLOCK, "In"}} );
    m_summaryToDescMap.insert( {"BVMF", {A::SUMMARY_BLOCK, "Vapor mole fraction"}} );
    m_summaryToDescMap.insert( {"BPSAT", {A::SUMMARY_BLOCK, "Saturation Pressures"}} );
    m_summaryToDescMap.insert( {"BAMF", {A::SUMMARY_BLOCK, "Component aqueous mole fraction"}} );
    m_summaryToDescMap.insert( {"BXMF", {A::SUMMARY_BLOCK, "Liquid hydrocarbon component mole fraction"}} );
    m_summaryToDescMap.insert( {"BYMF", {A::SUMMARY_BLOCK, "Vapor hydrocarbon component mole fraction / vapor steam"}} );
    m_summaryToDescMap.insert(
        {"BSMF", {A::SUMMARY_BLOCK, "CO2STORE with SOLID option only Solid hydrocarbon component mole fraction"}} );
    m_summaryToDescMap.insert( {"BSTEN", {A::SUMMARY_BLOCK, "Surface Tension"}} );
    m_summaryToDescMap.insert( {"BFMISC", {A::SUMMARY_BLOCK, "Miscibility Factor"}} );
    m_summaryToDescMap.insert(
        {"BREAC", {A::SUMMARY_BLOCK, "Reaction rate. The reaction number is given as a component index"}} );
    m_summaryToDescMap.insert( {"BHD", {A::SUMMARY_BLOCK, "Hydraulic head"}} );
    m_summaryToDescMap.insert( {"BHDF", {A::SUMMARY_BLOCK, "Hydraulic head at fresh water conditions"}} );
    m_summaryToDescMap.insert( {"BPR_X", {A::SUMMARY_BLOCK, "Pressure interpolated at a defined coordinate"}} );
    m_summaryToDescMap.insert( {"BHD_X", {A::SUMMARY_BLOCK, "Hydraulic head interpolated at a defined coordinate"}} );
    m_summaryToDescMap.insert(
        {"BHDF_X", {A::SUMMARY_BLOCK, "Hydraulic head at fresh water conditions interpolated at a defined coordinate"}} );
    m_summaryToDescMap.insert(
        {"BSCN_X", {A::SUMMARY_BLOCK, "Brine concentration interpolated at a defined coordinate"}} );
    m_summaryToDescMap.insert(
        {"BCTRA_X", {A::SUMMARY_BLOCK, "Tracer concentration interpolated at a defined coordinate"}} );
    m_summaryToDescMap.insert(
        {"LBPR_X", {A::SUMMARY_BLOCK, "Pressure interpolated at a defined coordinate within a local grid"}} );
    m_summaryToDescMap.insert(
        {"LBHD_X", {A::SUMMARY_BLOCK, "Hydraulic head interpolated at a defined coordinate within a local grid"}} );
    m_summaryToDescMap.insert(
        {"LBHDF_X",
         {A::SUMMARY_BLOCK,
          "Hydraulic head at freshwater conditions interpolated at a defined coordinate within a local grid"}} );
    m_summaryToDescMap.insert(
        {"LBSCN_X", {A::SUMMARY_BLOCK, "Brine concentration interpolated at a defined coordinate within a local grid"}} );
    m_summaryToDescMap.insert(
        {"LBCTRA_X",
         {A::SUMMARY_BLOCK, "Tracer concentration interpolated at a defined coordinate within a local grid"}} );
    m_summaryToDescMap.insert( {"BOKRX", {A::SUMMARY_BLOCK, "Oil relative permeability in the X direction"}} );
    m_summaryToDescMap.insert( {"BOKRX", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -X direction"}} );
    m_summaryToDescMap.insert( {"BOKRY", {A::SUMMARY_BLOCK, "Oil relative permeability in the Y direction"}} );
    m_summaryToDescMap.insert( {"BOKRY", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -Y direction"}} );
    m_summaryToDescMap.insert( {"BOKRZ", {A::SUMMARY_BLOCK, "Oil relative permeability in the Z direction"}} );
    m_summaryToDescMap.insert( {"BOKRZ", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -Z direction"}} );
    m_summaryToDescMap.insert( {"BWKRX", {A::SUMMARY_BLOCK, "Water relative permeability in the X direction"}} );
    m_summaryToDescMap.insert( {"BWKRX", {A::SUMMARY_BLOCK, "- Water relative permeability in the -X direction"}} );
    m_summaryToDescMap.insert( {"BWKRY", {A::SUMMARY_BLOCK, "Water relative permeability in the Y direction"}} );
    m_summaryToDescMap.insert( {"BWKRY", {A::SUMMARY_BLOCK, "- Water relative permeability in the -Y direction"}} );
    m_summaryToDescMap.insert( {"BWKRZ", {A::SUMMARY_BLOCK, "Water relative permeability in the Z direction"}} );
    m_summaryToDescMap.insert( {"BWKRZ", {A::SUMMARY_BLOCK, "- Water relative permeability in the -Z direction"}} );
    m_summaryToDescMap.insert( {"BGKRX", {A::SUMMARY_BLOCK, "Gas relative permeability in the X direction"}} );
    m_summaryToDescMap.insert( {"BGKRX", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -X direction"}} );
    m_summaryToDescMap.insert( {"BGKRY", {A::SUMMARY_BLOCK, "Gas relative permeability in the Y direction"}} );
    m_summaryToDescMap.insert( {"BGKRY", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -Y direction"}} );
    m_summaryToDescMap.insert( {"BGKRZ", {A::SUMMARY_BLOCK, "Gas relative permeability in the Z direction"}} );
    m_summaryToDescMap.insert( {"BGKRZ", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -Z direction"}} );
    m_summaryToDescMap.insert( {"BOKRI", {A::SUMMARY_BLOCK, "Oil relative permeability in the I direction"}} );
    m_summaryToDescMap.insert( {"BOKRI", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -I direction"}} );
    m_summaryToDescMap.insert( {"BOKRJ", {A::SUMMARY_BLOCK, "Oil relative permeability in the J direction"}} );
    m_summaryToDescMap.insert( {"BOKRJ", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -J direction"}} );
    m_summaryToDescMap.insert( {"BOKRK", {A::SUMMARY_BLOCK, "Oil relative permeability in the K direction"}} );
    m_summaryToDescMap.insert( {"BOKRK", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -K direction"}} );
    m_summaryToDescMap.insert( {"BWKRI", {A::SUMMARY_BLOCK, "Water relative permeability in the I direction"}} );
    m_summaryToDescMap.insert( {"BWKRI", {A::SUMMARY_BLOCK, "- Water relative permeability in the -I direction"}} );
    m_summaryToDescMap.insert( {"BWKRJ", {A::SUMMARY_BLOCK, "Water relative permeability in the J direction"}} );
    m_summaryToDescMap.insert( {"BWKRJ", {A::SUMMARY_BLOCK, "- Water relative permeability in the -J direction"}} );
    m_summaryToDescMap.insert( {"BWKRK", {A::SUMMARY_BLOCK, "Water relative permeability in the K direction"}} );
    m_summaryToDescMap.insert( {"BWKRK", {A::SUMMARY_BLOCK, "- Water relative permeability in the -K direction"}} );
    m_summaryToDescMap.insert( {"BGKRI", {A::SUMMARY_BLOCK, "Gas relative permeability in the I direction"}} );
    m_summaryToDescMap.insert( {"BGKRI", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -I direction"}} );
    m_summaryToDescMap.insert( {"BGKRJ", {A::SUMMARY_BLOCK, "Gas relative permeability in the J direction"}} );
    m_summaryToDescMap.insert( {"BGKRJ", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -J direction"}} );
    m_summaryToDescMap.insert( {"BGKRK", {A::SUMMARY_BLOCK, "Gas relative permeability in the K direction"}} );
    m_summaryToDescMap.insert( {"BGKRK", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -K direction"}} );
    m_summaryToDescMap.insert( {"BOKRR", {A::SUMMARY_BLOCK, "Oil relative permeability in the R"}} );
    m_summaryToDescMap.insert( {"BOKRR", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -R"}} );
    m_summaryToDescMap.insert( {"BOKRT", {A::SUMMARY_BLOCK, "Oil relative permeability in the T"}} );
    m_summaryToDescMap.insert( {"BOKRT", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -T"}} );
    m_summaryToDescMap.insert( {"BWKRR", {A::SUMMARY_BLOCK, "Water relative permeability in the R"}} );
    m_summaryToDescMap.insert( {"BWKRR", {A::SUMMARY_BLOCK, "- Water relative permeability in the -R"}} );
    m_summaryToDescMap.insert( {"BWKRT", {A::SUMMARY_BLOCK, "Water relative permeability in the T"}} );
    m_summaryToDescMap.insert( {"BWKRT", {A::SUMMARY_BLOCK, "- Water relative permeability in the -T"}} );
    m_summaryToDescMap.insert( {"BGKRR", {A::SUMMARY_BLOCK, "Gas relative permeability in the R"}} );
    m_summaryToDescMap.insert( {"BGKRR", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -R"}} );
    m_summaryToDescMap.insert( {"BGKRT", {A::SUMMARY_BLOCK, "Gas relative permeability in the T"}} );
    m_summaryToDescMap.insert( {"BGKRT", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -T"}} );
    m_summaryToDescMap.insert( {"BRPV", {A::SUMMARY_BLOCK, "Pore Volume at Reservoir conditions"}} );
    m_summaryToDescMap.insert( {"BPORV", {A::SUMMARY_BLOCK, "Cell Pore Volumes at Reference conditions"}} );
    m_summaryToDescMap.insert( {"BOPV", {A::SUMMARY_BLOCK, "Pore Volume containing Oil"}} );
    m_summaryToDescMap.insert( {"BWPV", {A::SUMMARY_BLOCK, "Pore Volume containing Water"}} );
    m_summaryToDescMap.insert( {"BGPV", {A::SUMMARY_BLOCK, "Pore Volume containing Gas"}} );
    m_summaryToDescMap.insert( {"BHPV", {A::SUMMARY_BLOCK, "Pore Volume containing Hydrocarbon"}} );
    m_summaryToDescMap.insert(
        {"BRTM", {A::SUMMARY_BLOCK, "Transmissibility Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BPERMMOD", {A::SUMMARY_BLOCK, "Transmissibility Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BPERMMDX",
         {A::SUMMARY_BLOCK,
          "Directional Transmissibility Multipliers in the X direction, associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BPERMMDY",
         {A::SUMMARY_BLOCK,
          "Directional Transmissibility Multipliers in the Y direction, associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BPERMMDZ",
         {A::SUMMARY_BLOCK,
          "Directional Transmissibility Multipliers in the Z direction, associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BPORVMOD", {A::SUMMARY_BLOCK, "Pore Volume Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert(
        {"BSIGMMOD", {A::SUMMARY_BLOCK, "Dual Porosity Sigma Multiplier associated with rock compaction"}} );
    m_summaryToDescMap.insert( {"BTCNF", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    m_summaryToDescMap.insert( {"BTCNS", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    m_summaryToDescMap.insert( {"BTCN", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    m_summaryToDescMap.insert( {"BTIPT", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BTIPF", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BTIPS", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BAPI", {A::SUMMARY_BLOCK, "Oil API"}} );
    m_summaryToDescMap.insert( {"BSCN", {A::SUMMARY_BLOCK, "Salt Cell Concentration"}} );
    m_summaryToDescMap.insert( {"BSIP", {A::SUMMARY_BLOCK, "Salt In Place"}} );
    m_summaryToDescMap.insert( {"BEWV_SAL", {A::SUMMARY_BLOCK, "Effective water viscosity due to salt concentration"}} );
    m_summaryToDescMap.insert( {"BTCNFANI", {A::SUMMARY_BLOCK, "Anion Flowing Concentration"}} );
    m_summaryToDescMap.insert( {"BTCNFCAT", {A::SUMMARY_BLOCK, "Cation Flowing Concentration"}} );
    m_summaryToDescMap.insert( {"BTRADCAT", {A::SUMMARY_BLOCK, "Cation Rock Associated Concentration"}} );
    m_summaryToDescMap.insert( {"BTSADCAT", {A::SUMMARY_BLOCK, "Cation Surfactant Associated Concentration"}} );
    m_summaryToDescMap.insert( {"BESALSUR", {A::SUMMARY_BLOCK, "Effective Salinity with respect to Surfactant"}} );
    m_summaryToDescMap.insert( {"BESALPLY", {A::SUMMARY_BLOCK, "Effective Salinity with respect to Polymer"}} );
    m_summaryToDescMap.insert( {"BTCNFHEA", {A::SUMMARY_BLOCK, "Block Temperature"}} );
    m_summaryToDescMap.insert(
        {"BTIPTHEA", {A::SUMMARY_BLOCK, "Difference in Energy in place between current and initial time"}} );
    m_summaryToDescMap.insert( {"BTCNF", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    m_summaryToDescMap.insert( {"BTCNS", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    m_summaryToDescMap.insert( {"BTCN#", {A::SUMMARY_BLOCK, "Tracer concentration in phase # (1,2,3,...)"}} );
    m_summaryToDescMap.insert( {"BTIPT", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BTIPF", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BTIPS", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    m_summaryToDescMap.insert( {"BTIP#", {A::SUMMARY_BLOCK, "Tracer In Place in phase # (1,2,3,...)"}} );
    m_summaryToDescMap.insert( {"BTADS", {A::SUMMARY_BLOCK, "Tracer Adsorption"}} );
    m_summaryToDescMap.insert( {"BTDCY", {A::SUMMARY_BLOCK, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"BCGC", {A::SUMMARY_BLOCK, "Bulk Coal Gas Concentration"}} );
    m_summaryToDescMap.insert( {"BCSC", {A::SUMMARY_BLOCK, "Bulk Coal Solvent Concentration"}} );
    m_summaryToDescMap.insert( {"BTCNFFOA", {A::SUMMARY_BLOCK, "Concentration"}} );
    m_summaryToDescMap.insert( {"BFOAM", {A::SUMMARY_BLOCK, "Surfactant concentration"}} );
    m_summaryToDescMap.insert( {"BTCNMFOA", {A::SUMMARY_BLOCK, "Capillary number"}} );
    m_summaryToDescMap.insert( {"BFOAMCNM", {A::SUMMARY_BLOCK, "Capillary number"}} );
    m_summaryToDescMap.insert( {"BTIPTFOA", {A::SUMMARY_BLOCK, "In Solution"}} );
    m_summaryToDescMap.insert( {"BTADSFOA", {A::SUMMARY_BLOCK, "Adsorption"}} );
    m_summaryToDescMap.insert( {"BTDCYFOA", {A::SUMMARY_BLOCK, "Decayed tracer"}} );
    m_summaryToDescMap.insert( {"BTMOBFOA", {A::SUMMARY_BLOCK, "Gas mobility factor"}} );
    m_summaryToDescMap.insert( {"BFOAMMOB", {A::SUMMARY_BLOCK, "Gas mobility factor"}} );
    m_summaryToDescMap.insert( {"BTHLFFOA", {A::SUMMARY_BLOCK, "Decay Half life"}} );
    m_summaryToDescMap.insert( {"BGI", {A::SUMMARY_BLOCK, "Block Gi value"}} );
    m_summaryToDescMap.insert( {"BCCN", {A::SUMMARY_BLOCK, "Polymer Concentration"}} );
    m_summaryToDescMap.insert( {"BCIP", {A::SUMMARY_BLOCK, "Polymer In Solution"}} );
    m_summaryToDescMap.insert( {"BEPVIS", {A::SUMMARY_BLOCK, "Effective polymer solution viscosity"}} );
    m_summaryToDescMap.insert( {"BVPOLY", {A::SUMMARY_BLOCK, "Effective polymer solution viscosity"}} );
    m_summaryToDescMap.insert( {"BEMVIS", {A::SUMMARY_BLOCK, "Effective mixture"}} );
    m_summaryToDescMap.insert( {"BEWV_POL", {A::SUMMARY_BLOCK, "Effective water viscosity"}} );
    m_summaryToDescMap.insert( {"BCAD", {A::SUMMARY_BLOCK, "Polymer Adsorption concentration"}} );
    m_summaryToDescMap.insert(
        {"BCDCS", {A::SUMMARY_BLOCK, "Polymer thermal degradation - total mass degraded in previous timestep"}} );
    m_summaryToDescMap.insert( {"BCDCR", {A::SUMMARY_BLOCK, "Polymer thermal degradation - total degradation rate"}} );
    m_summaryToDescMap.insert( {"BCDCP", {A::SUMMARY_BLOCK, "Polymer thermal degradation solution degradation rate"}} );
    m_summaryToDescMap.insert( {"BCDCA", {A::SUMMARY_BLOCK, "Polymer thermal degradation adsorbed degradation rate"}} );
    m_summaryToDescMap.insert(
        {"BCABnnn", {A::SUMMARY_BLOCK, "Adsorbed polymer by highest temperature band at which RRF was calculated"}} );
    m_summaryToDescMap.insert( {"BSCN", {A::SUMMARY_BLOCK, "Salt Cell Concentration"}} );
    m_summaryToDescMap.insert( {"BSIP", {A::SUMMARY_BLOCK, "Salt In Place"}} );
    m_summaryToDescMap.insert(
        {"BFLOW0I",
         {A::SUMMARY_BLOCK, "Inter-block water flow rate in the positive I direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BFLOW0J",
         {A::SUMMARY_BLOCK, "Inter-block water flow rate in the positive J direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BFLOW0K",
         {A::SUMMARY_BLOCK, "Inter-block water flow rate in the positive K direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BVELW0I",
         {A::SUMMARY_BLOCK,
          "Water velocity in the positive I direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BVELW0J",
         {A::SUMMARY_BLOCK,
          "Water velocity in the positive J direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BVELW0K",
         {A::SUMMARY_BLOCK,
          "Water velocity in the positive K direction multiplied by the corresponding shear multiplier"}} );
    m_summaryToDescMap.insert(
        {"BPSHLZI", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive I direction"}} );
    m_summaryToDescMap.insert(
        {"BPSHLZJ", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive J direction"}} );
    m_summaryToDescMap.insert(
        {"BPSHLZK", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive K direction"}} );
    m_summaryToDescMap.insert(
        {"BSRTW0I", {A::SUMMARY_BLOCK, "Water shear rate in the positive I direction prior to shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSRTW0J", {A::SUMMARY_BLOCK, "Water shear rate in the positive J direction prior to shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSRTW0K", {A::SUMMARY_BLOCK, "Water shear rate in the positive K direction prior to shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSRTWI", {A::SUMMARY_BLOCK, "Water shear rate in the positive I direction following shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSRTWJ", {A::SUMMARY_BLOCK, "Water shear rate in the positive J direction following shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSRTWK", {A::SUMMARY_BLOCK, "Water shear rate in the positive K direction following shear effects"}} );
    m_summaryToDescMap.insert(
        {"BSHWVISI",
         {A::SUMMARY_BLOCK, "Shear viscosity of the water/polymer solution due to shear thinning/thickening in the positive I direction"}} );
    m_summaryToDescMap.insert(
        {"BSHWVISJ",
         {A::SUMMARY_BLOCK, "Shear viscosity of the water/polymer solution due to shear thinning/thickening in the positive J direction"}} );
    m_summaryToDescMap.insert(
        {"BSHWVISK",
         {A::SUMMARY_BLOCK, "Shear viscosity of the water/polymer solution due to shear thinning/thickening in the positive K direction"}} );
    m_summaryToDescMap.insert( {"BNSAT", {A::SUMMARY_BLOCK, "Solvent SATuration"}} );
    m_summaryToDescMap.insert( {"BNIP", {A::SUMMARY_BLOCK, "Solvent In Place"}} );
    m_summaryToDescMap.insert( {"BNKR", {A::SUMMARY_BLOCK, "Solvent relative permeability"}} );
    m_summaryToDescMap.insert( {"BTCNFSUR", {A::SUMMARY_BLOCK, "Concentration"}} );
    m_summaryToDescMap.insert( {"BSURF", {A::SUMMARY_BLOCK, "Concentration in solution"}} );
    m_summaryToDescMap.insert( {"BTIPTSUR", {A::SUMMARY_BLOCK, "In Solution"}} );
    m_summaryToDescMap.insert( {"BTADSUR", {A::SUMMARY_BLOCK, "Adsorption"}} );
    m_summaryToDescMap.insert( {"BTCASUR", {A::SUMMARY_BLOCK, "Log"}} );
    m_summaryToDescMap.insert( {"BSURFCNM", {A::SUMMARY_BLOCK, "Log"}} );
    m_summaryToDescMap.insert( {"BTSTSUR", {A::SUMMARY_BLOCK, "Surface tension"}} );
    m_summaryToDescMap.insert( {"BSURFST", {A::SUMMARY_BLOCK, "Surface tension"}} );
    m_summaryToDescMap.insert(
        {"BEWV_SUR", {A::SUMMARY_BLOCK, "Effective water viscosity due to surfactant concentration"}} );
    m_summaryToDescMap.insert(
        {"BESVIS", {A::SUMMARY_BLOCK, "Effective water viscosity due to surfactant concentration"}} );
    m_summaryToDescMap.insert( {"BTCNFALK", {A::SUMMARY_BLOCK, "Concentration"}} );
    m_summaryToDescMap.insert( {"BTADSALK", {A::SUMMARY_BLOCK, "Adsorption"}} );
    m_summaryToDescMap.insert( {"BTSTMALK", {A::SUMMARY_BLOCK, "Surface tension multiplier"}} );
    m_summaryToDescMap.insert( {"BTSADALK", {A::SUMMARY_BLOCK, "Surfactant adsorption multiplier"}} );
    m_summaryToDescMap.insert( {"BTPADALK", {A::SUMMARY_BLOCK, "Polymer adsorption multiplier"}} );
    m_summaryToDescMap.insert(
        {"BKRGOE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to gas for gas-oil system"}} );
    m_summaryToDescMap.insert(
        {"BKRGWE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to gas for gas-water system"}} );
    m_summaryToDescMap.insert(
        {"BKRWGE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to water for water-gas system"}} );
    m_summaryToDescMap.insert(
        {"BKROWT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to oil for oil-water system"}} );
    m_summaryToDescMap.insert(
        {"BKRWOT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to water for water-oil system"}} );
    m_summaryToDescMap.insert(
        {"BKROGT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to oil for oil-gas system"}} );
    m_summaryToDescMap.insert(
        {"BKRGOT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to gas for gas-oil system"}} );
    m_summaryToDescMap.insert(
        {"BKRGWT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to gas for gas-water system"}} );
    m_summaryToDescMap.insert(
        {"BKRWGT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to water for water-gas system"}} );
    m_summaryToDescMap.insert( {"BIFTOW", {A::SUMMARY_BLOCK, "Oil-water interfacial tension"}} );
    m_summaryToDescMap.insert( {"BIFTWO", {A::SUMMARY_BLOCK, "Water-oil interfacial tension"}} );
    m_summaryToDescMap.insert( {"BIFTOG", {A::SUMMARY_BLOCK, "Oil-gas interfacial tension"}} );
    m_summaryToDescMap.insert( {"BIFTGO", {A::SUMMARY_BLOCK, "Gas-oil interfacial tension"}} );
    m_summaryToDescMap.insert( {"BIFTGW", {A::SUMMARY_BLOCK, "Gas-water interfacial tension"}} );
    m_summaryToDescMap.insert( {"BIFTWG", {A::SUMMARY_BLOCK, "Water-gas interfacial tension"}} );
    m_summaryToDescMap.insert( {"BPCOWR", {A::SUMMARY_BLOCK, "Representative oil-water capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCWOR", {A::SUMMARY_BLOCK, "Representative water-oil capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCOGR", {A::SUMMARY_BLOCK, "Representative oil-gas capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCGOR", {A::SUMMARY_BLOCK, "Representative gas-oil capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCGWR", {A::SUMMARY_BLOCK, "Representative gas-water capillary pressure"}} );
    m_summaryToDescMap.insert( {"BPCWGR", {A::SUMMARY_BLOCK, "Representative water-gas capillary pressure"}} );

    m_summaryToDescMap.insert( {"SOFR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"SOFRF", {A::SUMMARY_WELL_SEGMENT, "Segment Free Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"SOFRS", {A::SUMMARY_WELL_SEGMENT, "Segment Solution Oil Flow Rate"}} );
    m_summaryToDescMap.insert( {"SWFR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Flow Rate"}} );
    m_summaryToDescMap.insert( {"SGFR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"SGFRF", {A::SUMMARY_WELL_SEGMENT, "Segment Free Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"SGFRS", {A::SUMMARY_WELL_SEGMENT, "Segment Solution Gas Flow Rate"}} );
    m_summaryToDescMap.insert( {"SKFR", {A::SUMMARY_WELL_SEGMENT, "Segment Component Flow Rate"}} );
    m_summaryToDescMap.insert( {"SCWGFR", {A::SUMMARY_WELL_SEGMENT, "Segment Component Flow Rate as Wet Gas"}} );
    m_summaryToDescMap.insert( {"SHFR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Flow Rate"}} );
    m_summaryToDescMap.insert( {"SWCT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Cut"}} );
    m_summaryToDescMap.insert( {"SGOR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Oil Ratio"}} );
    m_summaryToDescMap.insert( {"SOGR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Gas Ratio"}} );
    m_summaryToDescMap.insert( {"SWGR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Gas Ratio"}} );
    m_summaryToDescMap.insert( {"SPR", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure"}} );
    m_summaryToDescMap.insert( {"SPRD", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop"}} );
    m_summaryToDescMap.insert( {"SPRDF", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop component due to Friction"}} );
    m_summaryToDescMap.insert(
        {"SPRDH", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop component due to Hydrostatic head"}} );
    m_summaryToDescMap.insert( {"SPRDA", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure drop due to Acceleration head"}} );
    m_summaryToDescMap.insert( {"SPRDM", {A::SUMMARY_WELL_SEGMENT, "Segment frictional Pressure Drop Multiplier"}} );
    m_summaryToDescMap.insert( {"SPPOW", {A::SUMMARY_WELL_SEGMENT, "Working power of a pull through pump"}} );
    m_summaryToDescMap.insert( {"SOFV", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Flow Velocity"}} );
    m_summaryToDescMap.insert( {"SWFV", {A::SUMMARY_WELL_SEGMENT, "Segment Water Flow Velocity"}} );
    m_summaryToDescMap.insert( {"SGFV", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Flow Velocity"}} );
    m_summaryToDescMap.insert( {"SOHF", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Holdup Fraction"}} );
    m_summaryToDescMap.insert( {"SWHF", {A::SUMMARY_WELL_SEGMENT, "Segment Water Holdup Fraction"}} );
    m_summaryToDescMap.insert( {"SGHF", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Holdup Fraction"}} );
    m_summaryToDescMap.insert( {"SDENM", {A::SUMMARY_WELL_SEGMENT, "Segment fluid mixture density"}} );
    m_summaryToDescMap.insert( {"SOVIS", {A::SUMMARY_WELL_SEGMENT, "Segment oil viscosity"}} );
    m_summaryToDescMap.insert( {"SWVIS", {A::SUMMARY_WELL_SEGMENT, "Segment water viscosity"}} );
    m_summaryToDescMap.insert( {"SGVIS", {A::SUMMARY_WELL_SEGMENT, "Segment gas viscosity"}} );
    m_summaryToDescMap.insert( {"SEMVIS", {A::SUMMARY_WELL_SEGMENT, "Segment effective mixture viscosity"}} );
    m_summaryToDescMap.insert( {"SGLPP", {A::SUMMARY_WELL_SEGMENT, "Segment Gas-Liquid Profile Parameter, C0"}} );
    m_summaryToDescMap.insert( {"SGLVD", {A::SUMMARY_WELL_SEGMENT, "Segment Gas-Liquid Drift Velocity, Vd"}} );
    m_summaryToDescMap.insert( {"SOWPP", {A::SUMMARY_WELL_SEGMENT, "Segment Oil-Water Profile Parameter, C0"}} );
    m_summaryToDescMap.insert( {"SOWVD", {A::SUMMARY_WELL_SEGMENT, "Segment Oil-Water Drift Velocity, Vd"}} );
    m_summaryToDescMap.insert( {"SOIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Import Rate"}} );
    m_summaryToDescMap.insert( {"SGIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Import Rate"}} );
    m_summaryToDescMap.insert( {"SWIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Import Rate"}} );
    m_summaryToDescMap.insert( {"SHIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Import Rate"}} );
    m_summaryToDescMap.insert( {"SORMR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Removal Rate"}} );
    m_summaryToDescMap.insert( {"SGRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Removal Rate"}} );
    m_summaryToDescMap.insert( {"SWRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Removal Rate"}} );
    m_summaryToDescMap.insert( {"SHRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Removal Rate"}} );
    m_summaryToDescMap.insert( {"SOIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Import Total"}} );
    m_summaryToDescMap.insert( {"SGIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Import Total"}} );
    m_summaryToDescMap.insert( {"SWIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Import Total"}} );
    m_summaryToDescMap.insert( {"SHIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Import Total"}} );
    m_summaryToDescMap.insert( {"SORMT", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Removal Total"}} );
    m_summaryToDescMap.insert( {"SGRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Removal Total"}} );
    m_summaryToDescMap.insert( {"SWRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Removal Total"}} );
    m_summaryToDescMap.insert( {"SHRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Removal Total"}} );
    m_summaryToDescMap.insert( {"SAPI", {A::SUMMARY_WELL_SEGMENT, "Segment API value"}} );
    m_summaryToDescMap.insert( {"SCFR", {A::SUMMARY_WELL_SEGMENT, "Segment polymer flow rate"}} );
    m_summaryToDescMap.insert( {"SCCN", {A::SUMMARY_WELL_SEGMENT, "Segment polymer concentration"}} );
    m_summaryToDescMap.insert( {"SSFR", {A::SUMMARY_WELL_SEGMENT, "Segment brine flow rate"}} );
    m_summaryToDescMap.insert( {"SSCN", {A::SUMMARY_WELL_SEGMENT, "Segment brine concentration"}} );
    m_summaryToDescMap.insert( {"STFR", {A::SUMMARY_WELL_SEGMENT, "Segment tracer flow rate"}} );
    m_summaryToDescMap.insert( {"STFC", {A::SUMMARY_WELL_SEGMENT, "Segment tracer concentration"}} );
    m_summaryToDescMap.insert(
        {"SFD", {A::SUMMARY_WELL_SEGMENT, "Segment diameter for Karst Conduit Calcite Dissolution"}} );
    m_summaryToDescMap.insert( {"SPSAT", {A::SUMMARY_WELL_SEGMENT, "Segment Psat"}} );
    m_summaryToDescMap.insert( {"STEM", {A::SUMMARY_WELL_SEGMENT, "Segment Temperature"}} );
    m_summaryToDescMap.insert( {"SENE", {A::SUMMARY_WELL_SEGMENT, "Segment Energy Density"}} );
    m_summaryToDescMap.insert( {"SSQU", {A::SUMMARY_WELL_SEGMENT, "Segment Steam Quality"}} );
    m_summaryToDescMap.insert( {"SCVPR", {A::SUMMARY_WELL_SEGMENT, "Segment Calorific Value Production Rate"}} );
    m_summaryToDescMap.insert( {"SGQ", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Quality"}} );
    m_summaryToDescMap.insert( {"SCSA", {A::SUMMARY_WELL_SEGMENT, "Segment Cross Sectional Area"}} );
    m_summaryToDescMap.insert( {"SSTR", {A::SUMMARY_WELL_SEGMENT, "Strength of ICD on segment"}} );
    m_summaryToDescMap.insert( {"SFOPN", {A::SUMMARY_WELL_SEGMENT, "Setting of segment"}} );
    m_summaryToDescMap.insert( {"SALQ", {A::SUMMARY_WELL_SEGMENT, "Artificial lift quantity for segment"}} );
    m_summaryToDescMap.insert( {"SRRQR", {A::SUMMARY_WELL_SEGMENT, "Reach flow at current time"}} );
    m_summaryToDescMap.insert( {"SRRQT", {A::SUMMARY_WELL_SEGMENT, "Reach cumulative flow"}} );
    m_summaryToDescMap.insert( {"SRBQR", {A::SUMMARY_WELL_SEGMENT, "Branch flow at current time"}} );
    m_summaryToDescMap.insert( {"SRBQT", {A::SUMMARY_WELL_SEGMENT, "Branch cumulative flow"}} );
    m_summaryToDescMap.insert( {"SRTQR", {A::SUMMARY_WELL_SEGMENT, "River total flow at current time"}} );
    m_summaryToDescMap.insert( {"SRTQT", {A::SUMMARY_WELL_SEGMENT, "River total cumulative flow"}} );
    m_summaryToDescMap.insert(
        {"SRRFLOW", {A::SUMMARY_WELL_SEGMENT, "Reach flux through cross-sectional area at current time"}} );
    m_summaryToDescMap.insert( {"SRRAREA", {A::SUMMARY_WELL_SEGMENT, "Reach area at current time"}} );
    m_summaryToDescMap.insert( {"SRRDEPTH", {A::SUMMARY_WELL_SEGMENT, "Reach depth at current time"}} );
    m_summaryToDescMap.insert( {"SRREXCH", {A::SUMMARY_WELL_SEGMENT, "Exchange flux at current time"}} );
    m_summaryToDescMap.insert( {"SRRFRODE", {A::SUMMARY_WELL_SEGMENT, "Reach Froude number at current time"}} );
    m_summaryToDescMap.insert( {"SRRHEAD", {A::SUMMARY_WELL_SEGMENT, "Reach hydraulic head at current time"}} );
    m_summaryToDescMap.insert( {"SRTFR", {A::SUMMARY_WELL_SEGMENT, "Reach tracer flow rate"}} );
    m_summaryToDescMap.insert( {"SRTFC", {A::SUMMARY_WELL_SEGMENT, "Reach tracer concentration"}} );
    m_summaryToDescMap.insert( {"SRSFR", {A::SUMMARY_WELL_SEGMENT, "Reach brine flow rate through connections"}} );
    m_summaryToDescMap.insert( {"SRSFC", {A::SUMMARY_WELL_SEGMENT, "Reach brine concentration"}} );
    m_summaryToDescMap.insert( {"SU", {A::SUMMARY_WELL_SEGMENT, "User-defined segment quantity"}} );

    m_summaryToDescMap.insert( {"AAQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"ALQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"AAQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"ALQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"AAQRG", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"ALQRG", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"AAQTG", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"ALQTG", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"AACMR", {A::SUMMARY_AQUIFER, "Aquifer component molar influx rate"}} );
    m_summaryToDescMap.insert( {"AACMT", {A::SUMMARY_AQUIFER, "Aquifer component molar influx totals"}} );
    m_summaryToDescMap.insert( {"AAQP", {A::SUMMARY_AQUIFER, "Aquifer pressure"}} );
    m_summaryToDescMap.insert( {"AAQER", {A::SUMMARY_AQUIFER, "Aquifer thermal energy influx rate"}} );
    m_summaryToDescMap.insert( {"AAQET", {A::SUMMARY_AQUIFER, "Cumulative aquifer thermal energy influx"}} );
    m_summaryToDescMap.insert( {"AAQTEMP", {A::SUMMARY_AQUIFER, "Aquifer temperature"}} );
    m_summaryToDescMap.insert( {"AAQENTH", {A::SUMMARY_AQUIFER, "Aquifer molar enthalpy"}} );
    m_summaryToDescMap.insert( {"AAQTD", {A::SUMMARY_AQUIFER, "Aquifer dimensionless time"}} );
    m_summaryToDescMap.insert( {"AAQPD", {A::SUMMARY_AQUIFER, "Aquifer dimensionless pressure"}} );
    m_summaryToDescMap.insert( {"ANQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    m_summaryToDescMap.insert( {"ANQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    m_summaryToDescMap.insert( {"ANQP", {A::SUMMARY_AQUIFER, "Aquifer pressure"}} );

    m_summaryToDescMap.insert( {"CPU", {A::SUMMARY_MISC, "CPU"}} );
    m_summaryToDescMap.insert( {"DATE", {A::SUMMARY_MISC, "Date"}} );
    m_summaryToDescMap.insert( {"DAY", {A::SUMMARY_MISC, "Day"}} );
    m_summaryToDescMap.insert( {"ELAPSED", {A::SUMMARY_MISC, "Elapsed time in seconds"}} );
    m_summaryToDescMap.insert( {"MLINEARS", {A::SUMMARY_MISC, "Number linear iterations for each timestep"}} );
    m_summaryToDescMap.insert( {"MONTH", {A::SUMMARY_MISC, "Month"}} );
    m_summaryToDescMap.insert(
        {"MSUMLINS", {A::SUMMARY_MISC, "Total number of linear iterations since the start of the run"}} );
    m_summaryToDescMap.insert(
        {"MSUMNEWT", {A::SUMMARY_MISC, "Total number of Newton iterations since the start of the run"}} );
    m_summaryToDescMap.insert( {"NEWTON", {A::SUMMARY_MISC, "Number of Newton iterations used for each timestep"}} );
    m_summaryToDescMap.insert( {"STEPTYPE", {A::SUMMARY_MISC, "Step type"}} );
    m_summaryToDescMap.insert( {"TCPU", {A::SUMMARY_MISC, "TCPU"}} );
    m_summaryToDescMap.insert( {"TCPUDAY", {A::SUMMARY_MISC, "TCPUDAY"}} );
    m_summaryToDescMap.insert( {"TCPUTS", {A::SUMMARY_MISC, "TCPUTS"}} );
    m_summaryToDescMap.insert( {"TELAPLIN", {A::SUMMARY_MISC, "TELAPLIN"}} );
    m_summaryToDescMap.insert( {"TIME", {A::SUMMARY_MISC, "Time"}} );
    m_summaryToDescMap.insert( {"TIMESTEP", {A::SUMMARY_MISC, "Time step"}} );
    m_summaryToDescMap.insert( {"TIMESTRY", {A::SUMMARY_MISC, "TIMESTRY"}} );
    m_summaryToDescMap.insert( {"YEAR", {A::SUMMARY_MISC, "Year"}} );
    m_summaryToDescMap.insert( {"YEARS", {A::SUMMARY_MISC, "Years"}} );
}
