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

#include "RiuSummaryQuantityNameInfoProvider.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider* RiuSummaryQuantityNameInfoProvider::instance()
{
    static RiuSummaryQuantityNameInfoProvider* singleton = new RiuSummaryQuantityNameInfoProvider;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory
    RiuSummaryQuantityNameInfoProvider::categoryFromQuantityName( const std::string& quantity ) const
{
    auto info = quantityInfo( quantity );

    return info.category;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityInfo
    RiuSummaryQuantityNameInfoProvider::quantityInfo( const std::string& quantity ) const
{
    auto it = m_summaryToDescMap.find( quantity );

    if ( it != m_summaryToDescMap.end() )
    {
        return it->second;
    }
    else if ( quantity.size() > 1 && quantity[1] == 'U' )
    {
        // User defined vector name

        std::string key = quantity.substr( 0, 2 );

        it = m_summaryToDescMap.find( key );

        if ( it != m_summaryToDescMap.end() )
        {
            return it->second;
        }
    }
    else if ( quantity.size() > 5 )
    {
        // Check for custom vector naming

        std::string baseName = quantity.substr( 0, 5 );
        while ( baseName.back() == '_' )
            baseName.pop_back();

        it = m_summaryToDescMap.find( baseName );

        if ( it != m_summaryToDescMap.end() )
        {
            return it->second;
        }
    }

    return RiuSummaryQuantityInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiuSummaryQuantityNameInfoProvider::longNameFromQuantityName( const std::string& vectorName,
                                                                          bool returnVectorNameIfNotFound ) const
{
    auto info = quantityInfo( vectorName );
    return info.category != RifEclipseSummaryAddress::SUMMARY_INVALID || !returnVectorNameIfNotFound ? info.longName
                                                                                                     : vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityNameInfoProvider()
{
    m_summaryToDescMap = createInfoForEclipseKeywords();

    auto infoFor6x = createInfoFor6xKeywords();
    for ( const auto& other : infoFor6x )
    {
        if ( m_summaryToDescMap.count( other.first ) == 0 )
        {
            m_summaryToDescMap.insert( other );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityInfo>
    RiuSummaryQuantityNameInfoProvider::createInfoForEclipseKeywords()
{
    using A = RifEclipseSummaryAddress;

    std::map<std::string, RiuSummaryQuantityInfo> info;

    info.insert( {"FOPR", {A::SUMMARY_FIELD, "Oil Production Rate"}} );
    info.insert( {"FOPRA", {A::SUMMARY_FIELD, "Oil Production Rate above GOC"}} );
    info.insert( {"FOPRB", {A::SUMMARY_FIELD, "Oil Production Rate below GOC"}} );
    info.insert( {"FOPTA", {A::SUMMARY_FIELD, "Oil Production Total above GOC"}} );
    info.insert( {"FOPTB", {A::SUMMARY_FIELD, "Oil Production Total below GOC"}} );
    info.insert( {"FOPR1", {A::SUMMARY_FIELD, "Oil Production Rate above GOC"}} );
    info.insert( {"FOPR2", {A::SUMMARY_FIELD, "Oil Production Rate below GOC"}} );
    info.insert( {"FOPT1", {A::SUMMARY_FIELD, "Oil Production Total above GOC"}} );
    info.insert( {"FOPT2", {A::SUMMARY_FIELD, "Oil Production Total below GOC"}} );
    info.insert( {"FOMR", {A::SUMMARY_FIELD, "Oil Mass Rate"}} );
    info.insert( {"FOMT", {A::SUMMARY_FIELD, "Oil Mass Total"}} );
    info.insert( {"FODN", {A::SUMMARY_FIELD, "Oil Density at Surface Conditions"}} );
    info.insert( {"FOPRH", {A::SUMMARY_FIELD, "Oil Production Rate History"}} );
    info.insert( {"FOPRT", {A::SUMMARY_FIELD, "Oil Production Rate Target/Limit"}} );
    info.insert( {"FOPRF", {A::SUMMARY_FIELD, "Free Oil Production Rate"}} );
    info.insert( {"FOPRS", {A::SUMMARY_FIELD, "Solution Oil Production Rate"}} );
    info.insert( {"FOPT", {A::SUMMARY_FIELD, "Oil Production Total"}} );
    info.insert( {"FOPTH", {A::SUMMARY_FIELD, "Oil Production Total History"}} );
    info.insert( {"FOPTF", {A::SUMMARY_FIELD, "Free Oil Production Total"}} );
    info.insert( {"FOPTS", {A::SUMMARY_FIELD, "Solution Oil Production Total"}} );
    info.insert( {"FOIR", {A::SUMMARY_FIELD, "Oil Injection Rate"}} );
    info.insert( {"FOIRH", {A::SUMMARY_FIELD, "Oil Injection Rate History"}} );
    info.insert( {"FOIRT", {A::SUMMARY_FIELD, "Oil Injection Rate Target/Limit"}} );
    info.insert( {"FOIT", {A::SUMMARY_FIELD, "Oil Injection Total"}} );
    info.insert( {"FOITH", {A::SUMMARY_FIELD, "Oil Injection Total History"}} );
    info.insert( {"FOPP", {A::SUMMARY_FIELD, "Oil Potential Production rate"}} );
    info.insert( {"FOPP2", {A::SUMMARY_FIELD, "Oil Potential Production rate"}} );
    info.insert( {"FOPI", {A::SUMMARY_FIELD, "Oil Potential Injection rate"}} );
    info.insert( {"FOPI2", {A::SUMMARY_FIELD, "Oil Potential Injection rate"}} );
    info.insert( {"FOVPR", {A::SUMMARY_FIELD, "Oil Voidage Production Rate"}} );
    info.insert( {"FOVPT", {A::SUMMARY_FIELD, "Oil Voidage Production Total"}} );
    info.insert( {"FOVIR", {A::SUMMARY_FIELD, "Oil Voidage Injection Rate"}} );
    info.insert( {"FOVIT", {A::SUMMARY_FIELD, "Oil Voidage Injection Total"}} );
    info.insert( {"FOnPR", {A::SUMMARY_FIELD, "nth separator stage oil rate"}} );
    info.insert( {"FOnPT", {A::SUMMARY_FIELD, "nth separator stage oil total"}} );
    info.insert( {"FEOR", {A::SUMMARY_FIELD, "Export Oil Rate"}} );
    info.insert( {"FEOT", {A::SUMMARY_FIELD, "Export Oil Total"}} );
    info.insert( {"FEOMF", {A::SUMMARY_FIELD, "Export Oil Mole Fraction"}} );
    info.insert( {"FWPR", {A::SUMMARY_FIELD, "Water Production Rate"}} );
    info.insert( {"FWMR", {A::SUMMARY_FIELD, "Water Mass Rate"}} );
    info.insert( {"FWMT", {A::SUMMARY_FIELD, "Water Mass Total"}} );
    info.insert( {"FWPRH", {A::SUMMARY_FIELD, "Water Production Rate History"}} );
    info.insert( {"FWPRT", {A::SUMMARY_FIELD, "Water Production Rate Target/Limit"}} );
    info.insert( {"FWPT", {A::SUMMARY_FIELD, "Water Production Total"}} );
    info.insert( {"FWPTH", {A::SUMMARY_FIELD, "Water Production Total History"}} );
    info.insert( {"FWIR", {A::SUMMARY_FIELD, "Water Injection Rate"}} );
    info.insert( {"FWIRH", {A::SUMMARY_FIELD, "Water Injection Rate History"}} );
    info.insert( {"FWIRT", {A::SUMMARY_FIELD, "Water Injection Rate Target/Limit"}} );
    info.insert( {"FWIT", {A::SUMMARY_FIELD, "Water Injection Total"}} );
    info.insert( {"FWITH", {A::SUMMARY_FIELD, "Water Injection Total History"}} );
    info.insert( {"FWPP", {A::SUMMARY_FIELD, "Water Potential Production rate"}} );
    info.insert( {"FWPP2", {A::SUMMARY_FIELD, "Water Potential Production rate"}} );
    info.insert( {"FWPI", {A::SUMMARY_FIELD, "Water Potential Injection rate"}} );
    info.insert( {"FWPI2", {A::SUMMARY_FIELD, "Water Potential Injection rate"}} );
    info.insert( {"FWVPR", {A::SUMMARY_FIELD, "Water Voidage Production Rate"}} );
    info.insert( {"FWVPT", {A::SUMMARY_FIELD, "Water Voidage Production Total"}} );
    info.insert( {"FWVIR", {A::SUMMARY_FIELD, "Water Voidage Injection Rate"}} );
    info.insert( {"FWVIT", {A::SUMMARY_FIELD, "Water Voidage Injection Total"}} );
    info.insert( {"FWPIR", {A::SUMMARY_FIELD, "Ratio of produced water to injected water (percentage)"}} );
    info.insert( {"FWMPR", {A::SUMMARY_FIELD, "Water component molar production rate"}} );
    info.insert( {"FWMPT", {A::SUMMARY_FIELD, "Water component molar production total"}} );
    info.insert( {"FWMIR", {A::SUMMARY_FIELD, "Water component molar injection rate"}} );
    info.insert( {"FWMIT", {A::SUMMARY_FIELD, "Water component molar injection total"}} );
    info.insert( {"FGPR", {A::SUMMARY_FIELD, "Gas Production Rate"}} );
    info.insert( {"FGPRA", {A::SUMMARY_FIELD, "Gas Production Rate above"}} );
    info.insert( {"FGPRB", {A::SUMMARY_FIELD, "Gas Production Rate below"}} );
    info.insert( {"FGPTA", {A::SUMMARY_FIELD, "Gas Production Total above"}} );
    info.insert( {"FGPTB", {A::SUMMARY_FIELD, "Gas Production Total below"}} );
    info.insert( {"FGPR1", {A::SUMMARY_FIELD, "Gas Production Rate above GOC"}} );
    info.insert( {"FGPR2", {A::SUMMARY_FIELD, "Gas Production Rate below GOC"}} );
    info.insert( {"FGPT1", {A::SUMMARY_FIELD, "Gas Production Total above GOC"}} );
    info.insert( {"FGPT2", {A::SUMMARY_FIELD, "Gas Production Total below GOC"}} );
    info.insert( {"FGMR", {A::SUMMARY_FIELD, "Gas Mass Rate"}} );
    info.insert( {"FGMT", {A::SUMMARY_FIELD, "Gas Mass Total"}} );
    info.insert( {"FGDN", {A::SUMMARY_FIELD, "Gas Density at Surface Conditions"}} );
    info.insert( {"FGPRH", {A::SUMMARY_FIELD, "Gas Production Rate History"}} );
    info.insert( {"FGPRT", {A::SUMMARY_FIELD, "Gas Production Rate Target/Limit"}} );
    info.insert( {"FGPRF", {A::SUMMARY_FIELD, "Free Gas Production Rate"}} );
    info.insert( {"FGPRS", {A::SUMMARY_FIELD, "Solution Gas Production Rate"}} );
    info.insert( {"FGPT", {A::SUMMARY_FIELD, "Gas Production Total"}} );
    info.insert( {"FGPTH", {A::SUMMARY_FIELD, "Gas Production Total History"}} );
    info.insert( {"FGPTF", {A::SUMMARY_FIELD, "Free Gas Production Total"}} );
    info.insert( {"FGPTS", {A::SUMMARY_FIELD, "Solution Gas Production Total"}} );
    info.insert( {"FGIR", {A::SUMMARY_FIELD, "Gas Injection Rate"}} );
    info.insert( {"FGIRH", {A::SUMMARY_FIELD, "Gas Injection Rate History"}} );
    info.insert( {"FGIRT", {A::SUMMARY_FIELD, "Gas Injection Rate Target/Limit"}} );
    info.insert( {"FGIT", {A::SUMMARY_FIELD, "Gas Injection Total"}} );
    info.insert( {"FGITH", {A::SUMMARY_FIELD, "Gas Injection Total History"}} );
    info.insert( {"FGPP", {A::SUMMARY_FIELD, "Gas Potential Production rate"}} );
    info.insert( {"FGPP2", {A::SUMMARY_FIELD, "Gas Potential Production rate"}} );
    info.insert( {"FGPPS", {A::SUMMARY_FIELD, "Solution"}} );
    info.insert( {"FGPPS2", {A::SUMMARY_FIELD, "Solution"}} );
    info.insert( {"FGPPF", {A::SUMMARY_FIELD, "Free Gas Potential Production rate"}} );
    info.insert( {"FGPPF2", {A::SUMMARY_FIELD, "Free Gas Potential Production rate"}} );
    info.insert( {"FGPI", {A::SUMMARY_FIELD, "Gas Potential Injection rate"}} );
    info.insert( {"FGPI2", {A::SUMMARY_FIELD, "Gas Potential Injection rate"}} );
    info.insert( {"FSGR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    info.insert( {"FGSR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    info.insert( {"FSGT", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    info.insert( {"FGST", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    info.insert( {"FSMF", {A::SUMMARY_FIELD, "Sales Gas Mole Fraction"}} );
    info.insert( {"FFGR", {A::SUMMARY_FIELD, "Fuel Gas Rate, at and below this group"}} );
    info.insert( {"FFGT", {A::SUMMARY_FIELD, "Fuel Gas cumulative Total, at and below this group"}} );
    info.insert( {"FFMF", {A::SUMMARY_FIELD, "Fuel Gas Mole Fraction"}} );
    info.insert( {"FGCR", {A::SUMMARY_FIELD, "Gas Consumption Rate, at and below this group"}} );
    info.insert( {"FGCT", {A::SUMMARY_FIELD, "Gas Consumption Total, at and below this group"}} );
    info.insert( {"FGIMR", {A::SUMMARY_FIELD, "Gas Import Rate, at and below this group"}} );
    info.insert( {"FGIMT", {A::SUMMARY_FIELD, "Gas Import Total, at and below this group"}} );
    info.insert( {"FGLIR", {A::SUMMARY_FIELD, "Gas Lift Injection Rate"}} );
    info.insert( {"FWGPR", {A::SUMMARY_FIELD, "Wet Gas Production Rate"}} );
    info.insert( {"FWGPT", {A::SUMMARY_FIELD, "Wet Gas Production Total"}} );
    info.insert( {"FWGPRH", {A::SUMMARY_FIELD, "Wet Gas Production Rate History"}} );
    info.insert( {"FWGIR", {A::SUMMARY_FIELD, "Wet Gas Injection Rate"}} );
    info.insert( {"FWGIT", {A::SUMMARY_FIELD, "Wet Gas Injection Total"}} );
    info.insert( {"FEGR", {A::SUMMARY_FIELD, "Export Gas Rate"}} );
    info.insert( {"FEGT", {A::SUMMARY_FIELD, "Export Gas Total"}} );
    info.insert( {"FEMF", {A::SUMMARY_FIELD, "Export Gas Mole Fraction"}} );
    info.insert( {"FEXGR", {A::SUMMARY_FIELD, "Excess Gas Rate"}} );
    info.insert( {"FEXGT", {A::SUMMARY_FIELD, "Excess Gas Total"}} );
    info.insert( {"FRGR", {A::SUMMARY_FIELD, "Re-injection Gas Rate"}} );
    info.insert( {"FRGT", {A::SUMMARY_FIELD, "Re-injection Gas Total"}} );
    info.insert( {"FGnPR", {A::SUMMARY_FIELD, "nth separator stage gas rate"}} );
    info.insert( {"FGnPT", {A::SUMMARY_FIELD, "nth separator stage gas total"}} );
    info.insert( {"FGVPR", {A::SUMMARY_FIELD, "Gas Voidage Production Rate"}} );
    info.insert( {"FGVPT", {A::SUMMARY_FIELD, "Gas Voidage Production Total"}} );
    info.insert( {"FGVIR", {A::SUMMARY_FIELD, "Gas Voidage Injection Rate"}} );
    info.insert( {"FGVIT", {A::SUMMARY_FIELD, "Gas Voidage Injection Total"}} );
    info.insert( {"FGQ", {A::SUMMARY_FIELD, "Gas Quality"}} );
    info.insert( {"FLPR", {A::SUMMARY_FIELD, "Liquid Production Rate"}} );
    info.insert( {"FLPRH", {A::SUMMARY_FIELD, "Liquid Production Rate History"}} );
    info.insert( {"FLPRT", {A::SUMMARY_FIELD, "Liquid Production Rate Target/Limit"}} );
    info.insert( {"FLPT", {A::SUMMARY_FIELD, "Liquid Production Total"}} );
    info.insert( {"FLPTH", {A::SUMMARY_FIELD, "Liquid Production Total History"}} );
    info.insert( {"FVPR", {A::SUMMARY_FIELD, "Res Volume Production Rate"}} );
    info.insert( {"FVPRT", {A::SUMMARY_FIELD, "Res Volume Production Rate Target/Limit"}} );
    info.insert( {"FVPT", {A::SUMMARY_FIELD, "Res Volume Production Total"}} );
    info.insert( {"FVIR", {A::SUMMARY_FIELD, "Res Volume Injection Rate"}} );
    info.insert( {"FVIRT", {A::SUMMARY_FIELD, "Res Volume Injection Rate Target/Limit"}} );
    info.insert( {"FVIT", {A::SUMMARY_FIELD, "Res Volume Injection Total"}} );
    info.insert( {"FWCT", {A::SUMMARY_FIELD, "Water Cut"}} );
    info.insert( {"FWCTH", {A::SUMMARY_FIELD, "Water Cut History"}} );
    info.insert( {"FGOR", {A::SUMMARY_FIELD, "Gas-Oil Ratio"}} );
    info.insert( {"FGORH", {A::SUMMARY_FIELD, "Gas-Oil Ratio History"}} );
    info.insert( {"FOGR", {A::SUMMARY_FIELD, "Oil-Gas Ratio"}} );
    info.insert( {"FOGRH", {A::SUMMARY_FIELD, "Oil-Gas Ratio History"}} );
    info.insert( {"FWGR", {A::SUMMARY_FIELD, "Water-Gas Ratio"}} );
    info.insert( {"FWGRH", {A::SUMMARY_FIELD, "Water-Gas Ratio History"}} );
    info.insert( {"FGLR", {A::SUMMARY_FIELD, "Gas-Liquid Ratio"}} );
    info.insert( {"FGLRH", {A::SUMMARY_FIELD, "Gas-Liquid Ratio History"}} );
    info.insert( {"FMCTP", {A::SUMMARY_FIELD, "Mode of Control for group Production"}} );
    info.insert( {"FMCTW", {A::SUMMARY_FIELD, "Mode of Control for group Water Injection"}} );
    info.insert( {"FMCTG", {A::SUMMARY_FIELD, "Mode of Control for group Gas Injection"}} );
    info.insert( {"FMWPT", {A::SUMMARY_FIELD, "Total number of production wells"}} );
    info.insert( {"FMWPR", {A::SUMMARY_FIELD, "Number of production wells currently flowing"}} );
    info.insert( {"FMWPA", {A::SUMMARY_FIELD, "Number of abandoned production wells"}} );
    info.insert( {"FMWPU", {A::SUMMARY_FIELD, "Number of unused production wells"}} );
    info.insert( {"FMWPG", {A::SUMMARY_FIELD, "Number of producers on group control"}} );
    info.insert( {"FMWPO", {A::SUMMARY_FIELD, "Number of producers controlled by own oil rate limit"}} );
    info.insert( {"FMWPS", {A::SUMMARY_FIELD, "Number of producers on own surface rate limit control"}} );
    info.insert( {"FMWPV", {A::SUMMARY_FIELD, "Number of producers on own reservoir volume rate limit control"}} );
    info.insert( {"FMWPP", {A::SUMMARY_FIELD, "Number of producers on pressure control"}} );
    info.insert( {"FMWPL", {A::SUMMARY_FIELD, "Number of producers using artificial lift"}} );
    info.insert( {"FMWIT", {A::SUMMARY_FIELD, "Total number of injection wells"}} );
    info.insert( {"FMWIN", {A::SUMMARY_FIELD, "Number of injection wells currently flowing"}} );
    info.insert( {"FMWIA", {A::SUMMARY_FIELD, "Number of abandoned injection wells"}} );
    info.insert( {"FMWIU", {A::SUMMARY_FIELD, "Number of unused injection wells"}} );
    info.insert( {"FMWIG", {A::SUMMARY_FIELD, "Number of injectors on group control"}} );
    info.insert( {"FMWIS", {A::SUMMARY_FIELD, "Number of injectors on own surface rate limit control"}} );
    info.insert( {"FMWIV", {A::SUMMARY_FIELD, "Number of injectors on own reservoir volume rate limit control"}} );
    info.insert( {"FMWIP", {A::SUMMARY_FIELD, "Number of injectors on pressure control"}} );
    info.insert( {"FMWDR", {A::SUMMARY_FIELD, "Number of drilling events this timestep"}} );
    info.insert( {"FMWDT", {A::SUMMARY_FIELD, "Number of drilling events in total"}} );
    info.insert( {"FMWWO", {A::SUMMARY_FIELD, "Number of workover events this timestep"}} );
    info.insert( {"FMWWT", {A::SUMMARY_FIELD, "Number of workover events in total"}} );
    info.insert( {"FEPR", {A::SUMMARY_FIELD, "Energy Production Rate"}} );
    info.insert( {"FEPT", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    info.insert( {"FNLPR", {A::SUMMARY_FIELD, "NGL Production Rate"}} );
    info.insert( {"FNLPT", {A::SUMMARY_FIELD, "NGL Production Total"}} );
    info.insert( {"FNLPRH", {A::SUMMARY_FIELD, "NGL Production Rate History"}} );
    info.insert( {"FNLPTH", {A::SUMMARY_FIELD, "NGL Production Total History"}} );
    info.insert( {"FAMF", {A::SUMMARY_FIELD, "Component aqueous mole fraction, from producing completions"}} );
    info.insert( {"FXMF", {A::SUMMARY_FIELD, "Liquid Mole Fraction"}} );
    info.insert( {"FYMF", {A::SUMMARY_FIELD, "Vapor Mole Fraction"}} );
    info.insert( {"FXMFn", {A::SUMMARY_FIELD, "Liquid Mole Fraction for nth separator stage"}} );
    info.insert( {"FYMFn", {A::SUMMARY_FIELD, "Vapor Mole Fraction for nth separator stage"}} );
    info.insert( {"FZMF", {A::SUMMARY_FIELD, "Total Mole Fraction"}} );
    info.insert( {"FCMPR", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Production Rates"}} );
    info.insert( {"FCMPT", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    info.insert( {"FCMIR", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Injection Rates"}} );
    info.insert( {"FCMIT", {A::SUMMARY_FIELD, "Hydrocarbon Component Molar Injection Totals"}} );
    info.insert( {"FHMIR", {A::SUMMARY_FIELD, "Hydrocarbon Molar Injection Rate"}} );
    info.insert( {"FHMIT", {A::SUMMARY_FIELD, "Hydrocarbon Molar Injection Total"}} );
    info.insert( {"FHMPR", {A::SUMMARY_FIELD, "Hydrocarbon Molar Production Rate"}} );
    info.insert( {"FHMPT", {A::SUMMARY_FIELD, "Hydrocarbon Molar Production Total"}} );
    info.insert( {"FCHMR", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    info.insert( {"FCHMT", {A::SUMMARY_FIELD, "Hydrocarbon Component"}} );
    info.insert( {"FCWGPR", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Production Rate"}} );
    info.insert( {"FCWGPT", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Production Total"}} );
    info.insert( {"FCWGIR", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    info.insert( {"FCWGIT", {A::SUMMARY_FIELD, "Hydrocarbon Component Wet Gas Injection Total"}} );
    info.insert( {"FCGMR", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCGMT", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCOMR", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCOMT", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCNMR", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the NGL phase"}} );
    info.insert( {"FCNWR", {A::SUMMARY_FIELD, "Hydrocarbon component mass rates in the NGL phase"}} );
    info.insert(
        {"FCGMRn", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert(
        {"FCGRn", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert( {"FCOMRn", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCORn", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FMUF", {A::SUMMARY_FIELD, "Make-up fraction"}} );
    info.insert( {"FAMR", {A::SUMMARY_FIELD, "Make-up gas rate"}} );
    info.insert( {"FAMT", {A::SUMMARY_FIELD, "Make-up gas total"}} );
    info.insert(
        {"FGSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for gas"}} );
    info.insert( {"FGSRL",
                  {A::SUMMARY_FIELD,
                   "Maximum tested rate sustained for the test period during the most recent "
                   "sustainable capacity test for gas"}} );
    info.insert( {"FGSRU",
                  {A::SUMMARY_FIELD,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for gas"}} );
    info.insert( {"FGSSP",
                  {A::SUMMARY_FIELD,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test for gas"}} );
    info.insert( {"FGSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for gas"}} );
    info.insert(
        {"FOSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for oil"}} );
    info.insert( {"FOSRL",
                  {A::SUMMARY_FIELD,
                   "Maximum tested rate sustained for the test period during the most recent "
                   "sustainable capacity test for oil"}} );
    info.insert( {"FOSRU",
                  {A::SUMMARY_FIELD,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for oil"}} );
    info.insert( {"FOSSP",
                  {A::SUMMARY_FIELD,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test for oil"}} );
    info.insert( {"FOSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for oil"}} );
    info.insert(
        {"FWSPR", {A::SUMMARY_FIELD, "Target sustainable rate for most recent sustainable capacity test for water"}} );
    info.insert( {"FWSRL",
                  {A::SUMMARY_FIELD,
                   "Maximum tested rate sustained for the test period during the most recent "
                   "sustainable capacity test for water"}} );
    info.insert( {"FWSRU",
                  {A::SUMMARY_FIELD,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for water"}} );
    info.insert( {"FWSSP",
                  {A::SUMMARY_FIELD,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test "
                   "for water"}} );
    info.insert( {"FWSTP", {A::SUMMARY_FIELD, "Test period for the most recent sustainable capacity test for water"}} );
    info.insert( {"FGPRG", {A::SUMMARY_FIELD, "Gas production rate"}} );
    info.insert( {"FOPRG", {A::SUMMARY_FIELD, "Oil production rate"}} );
    info.insert( {"FNLPRG", {A::SUMMARY_FIELD, "NGL production rate"}} );
    info.insert( {"FXMFG", {A::SUMMARY_FIELD, "Liquid mole fraction"}} );
    info.insert( {"FYMFG", {A::SUMMARY_FIELD, "Vapor mole fraction"}} );
    info.insert( {"FCOMRG", {A::SUMMARY_FIELD, "Hydrocarbon component"}} );
    info.insert( {"FCGMRG", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the gas phase"}} );
    info.insert( {"FCNMRG", {A::SUMMARY_FIELD, "Hydrocarbon component molar rates in the NGL phase"}} );
    info.insert( {"FPR", {A::SUMMARY_FIELD, "Pressure average value"}} );
    info.insert( {"FPRH", {A::SUMMARY_FIELD, "Pressure average value"}} );
    info.insert( {"FPRP", {A::SUMMARY_FIELD, "Pressure average value"}} );
    info.insert( {"FPRGZ", {A::SUMMARY_FIELD, "P/Z"}} );
    info.insert( {"FRS", {A::SUMMARY_FIELD, "Gas-oil ratio"}} );
    info.insert( {"FRV", {A::SUMMARY_FIELD, "Oil-gas ratio"}} );
    info.insert( {"FCHIP", {A::SUMMARY_FIELD, "Component Hydrocarbon as Wet Gas"}} );
    info.insert( {"FCMIP", {A::SUMMARY_FIELD, "Component Hydrocarbon as Moles"}} );
    info.insert( {"FPPC", {A::SUMMARY_FIELD, "Initial Contact Corrected Potential"}} );
    info.insert( {"FREAC", {A::SUMMARY_FIELD, "Reaction rate. The reaction number is given as a component index"}} );
    info.insert( {"FREAT", {A::SUMMARY_FIELD, "Reaction total. The reaction number is given as a component index"}} );
    info.insert( {"FRPV", {A::SUMMARY_FIELD, "Pore Volume at Reservoir conditions"}} );
    info.insert( {"FOPV", {A::SUMMARY_FIELD, "Pore Volume containing Oil"}} );
    info.insert( {"FWPV", {A::SUMMARY_FIELD, "Pore Volume containing Water"}} );
    info.insert( {"FGPV", {A::SUMMARY_FIELD, "Pore Volume containing Gas"}} );
    info.insert( {"FHPV", {A::SUMMARY_FIELD, "Pore Volume containing Hydrocarbon"}} );
    info.insert( {"FRTM", {A::SUMMARY_FIELD, "Transmissibility Multiplier associated with rock compaction"}} );
    info.insert( {"FOE", {A::SUMMARY_FIELD, "(OIP(initial) - OIP(now)) / OIP(initial)"}} );
    info.insert( {"FOEW", {A::SUMMARY_FIELD, "Oil Production from Wells / OIP(initial)"}} );
    info.insert( {"FOEIW", {A::SUMMARY_FIELD, "(OIP(initial) - OIP(now)) / Initial Mobile Oil with respect to Water"}} );
    info.insert( {"FOEWW", {A::SUMMARY_FIELD, "Oil Production from Wells / Initial Mobile Oil with respect to Water"}} );
    info.insert( {"FOEIG", {A::SUMMARY_FIELD, "(OIP(initial) - OIP(now)) / Initial Mobile Oil with respect to Gas"}} );
    info.insert( {"FOEWG", {A::SUMMARY_FIELD, "Oil Production from Wells / Initial Mobile Oil with respect to Gas"}} );
    info.insert( {"FORMR", {A::SUMMARY_FIELD, "Total stock tank oil produced by rock compaction"}} );
    info.insert( {"FORMW", {A::SUMMARY_FIELD, "Total stock tank oil produced by water influx"}} );
    info.insert( {"FORMG", {A::SUMMARY_FIELD, "Total stock tank oil produced by gas influx"}} );
    info.insert( {"FORME", {A::SUMMARY_FIELD, "Total stock tank oil produced by oil expansion"}} );
    info.insert( {"FORMS", {A::SUMMARY_FIELD, "Total stock tank oil produced by solution gas"}} );
    info.insert( {"FORMF", {A::SUMMARY_FIELD, "Total stock tank oil produced by free gas influx"}} );
    info.insert( {"FORMX", {A::SUMMARY_FIELD, "Total stock tank oil produced by 'traced' water influx"}} );
    info.insert( {"FORMY", {A::SUMMARY_FIELD, "Total stock tank oil produced by other water influx"}} );
    info.insert( {"FORFR", {A::SUMMARY_FIELD, "Fraction of total oil produced by rock compaction"}} );
    info.insert( {"FORFW", {A::SUMMARY_FIELD, "Fraction of total oil produced by water influx"}} );
    info.insert( {"FORFG", {A::SUMMARY_FIELD, "Fraction of total oil produced by gas influx"}} );
    info.insert( {"FORFE", {A::SUMMARY_FIELD, "Fraction of total oil produced by oil expansion"}} );
    info.insert( {"FORFS", {A::SUMMARY_FIELD, "Fraction of total oil produced by solution gas"}} );
    info.insert( {"FORFF", {A::SUMMARY_FIELD, "Fraction of total oil produced by free gas influx"}} );
    info.insert( {"FORFX", {A::SUMMARY_FIELD, "Fraction of total oil produced by 'traced' water influx"}} );
    info.insert( {"FORFY", {A::SUMMARY_FIELD, "Fraction of total oil produced by other water influx"}} );
    info.insert( {"FAQR", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    info.insert( {"FAQT", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    info.insert( {"FAQRG", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    info.insert( {"FAQTG", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    info.insert( {"FAQER", {A::SUMMARY_FIELD, "Aquifer thermal energy influx rate"}} );
    info.insert( {"FAQET", {A::SUMMARY_FIELD, "Cumulative aquifer thermal energy influx"}} );
    info.insert( {"FNQR", {A::SUMMARY_FIELD, "Aquifer influx rate"}} );
    info.insert( {"FNQT", {A::SUMMARY_FIELD, "Cumulative aquifer influx"}} );
    info.insert( {"FTPR", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    info.insert( {"FTPT", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    info.insert( {"FTPC", {A::SUMMARY_FIELD, "Tracer Production Concentration"}} );
    info.insert( {"FTIR", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    info.insert( {"FTIT", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    info.insert( {"FTIC", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    info.insert( {"FTMR", {A::SUMMARY_FIELD, "Traced mass Rate"}} );
    info.insert( {"FTMT", {A::SUMMARY_FIELD, "Traced mass Total"}} );
    info.insert( {"FTQR", {A::SUMMARY_FIELD, "Traced molar Rate"}} );
    info.insert( {"FTCM", {A::SUMMARY_FIELD, "Tracer Carrier molar Rate"}} );
    info.insert( {"FTMF", {A::SUMMARY_FIELD, "Traced molar fraction"}} );
    info.insert( {"FTVL", {A::SUMMARY_FIELD, "Traced liquid volume rate"}} );
    info.insert( {"FTVV", {A::SUMMARY_FIELD, "Traced vapor volume rate"}} );
    info.insert( {"FTTL", {A::SUMMARY_FIELD, "Traced liquid volume total"}} );
    info.insert( {"FTTV", {A::SUMMARY_FIELD, "Traced vapor volume total"}} );
    info.insert( {"FTML", {A::SUMMARY_FIELD, "Traced mass liquid rate"}} );
    info.insert( {"FTMV", {A::SUMMARY_FIELD, "Traced mass vapor rate"}} );
    info.insert( {"FTLM", {A::SUMMARY_FIELD, "Traced mass liquid total"}} );
    info.insert( {"FTVM", {A::SUMMARY_FIELD, "Traced mass vapor total"}} );
    info.insert( {"FTIPT", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FTIPF", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FTIPS", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FAPI", {A::SUMMARY_FIELD, "Oil API"}} );
    info.insert( {"FSPR", {A::SUMMARY_FIELD, "Salt Production Rate"}} );
    info.insert( {"FSPT", {A::SUMMARY_FIELD, "Salt Production Total"}} );
    info.insert( {"FSIR", {A::SUMMARY_FIELD, "Salt Injection Rate"}} );
    info.insert( {"FSIT", {A::SUMMARY_FIELD, "Salt Injection Total"}} );
    info.insert( {"FSPC", {A::SUMMARY_FIELD, "Salt Production Concentration"}} );
    info.insert( {"FSIC", {A::SUMMARY_FIELD, "Salt Injection Concentration"}} );
    info.insert( {"FSIP", {A::SUMMARY_FIELD, "Salt In Place"}} );
    info.insert( {"GTPRANI", {A::SUMMARY_FIELD, "Anion Production Rate"}} );
    info.insert( {"GTPTANI", {A::SUMMARY_FIELD, "Anion Production Total"}} );
    info.insert( {"GTIRANI", {A::SUMMARY_FIELD, "Anion Injection Rate"}} );
    info.insert( {"GTITANI", {A::SUMMARY_FIELD, "Anion Injection Total"}} );
    info.insert( {"GTPRCAT", {A::SUMMARY_FIELD, "Cation Production Rate"}} );
    info.insert( {"GTPTCAT", {A::SUMMARY_FIELD, "Cation Production Total"}} );
    info.insert( {"GTIRCAT", {A::SUMMARY_FIELD, "Cation Injection Rate"}} );
    info.insert( {"GTITCAT", {A::SUMMARY_FIELD, "Cation Injection Total"}} );
    info.insert( {"FTPCHEA", {A::SUMMARY_FIELD, "Production Temperature"}} );
    info.insert( {"FTICHEA", {A::SUMMARY_FIELD, "Injection Temperature"}} );
    info.insert( {"FTPRHEA", {A::SUMMARY_FIELD, "Energy flows"}} );
    info.insert( {"FTPTHEA", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    info.insert( {"FTIRHEA", {A::SUMMARY_FIELD, "Energy flows"}} );
    info.insert( {"FTITHEA", {A::SUMMARY_FIELD, "Energy Injection Total"}} );
    info.insert( {"FTIPTHEA", {A::SUMMARY_FIELD, "Difference in Energy in place between current and initial time"}} );
    info.insert( {"FTPR", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    info.insert( {"FTPT", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    info.insert( {"FTPC", {A::SUMMARY_FIELD, "Tracer Production Concentration"}} );
    info.insert( {"FTIR", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    info.insert( {"FTIT", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    info.insert( {"FTIC", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    info.insert( {"FTIPT", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FTIPF", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FTIPS", {A::SUMMARY_FIELD, "Tracer In Place"}} );
    info.insert( {"FTIP#", {A::SUMMARY_FIELD, " Tracer In Place in phase # (1,2,3,...)"}} );
    info.insert( {"FTADS", {A::SUMMARY_FIELD, "Tracer Adsorption total"}} );
    info.insert( {"FTDCY", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    info.insert( {"FTIRF", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    info.insert( {"FTIRS", {A::SUMMARY_FIELD, "Tracer Injection Rate"}} );
    info.insert( {"FTPRF", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    info.insert( {"FTPRS", {A::SUMMARY_FIELD, "Tracer Production Rate"}} );
    info.insert( {"FTITF", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    info.insert( {"FTITS", {A::SUMMARY_FIELD, "Tracer Injection Total"}} );
    info.insert( {"FTPTF", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    info.insert( {"FTPTS", {A::SUMMARY_FIELD, "Tracer Production Total"}} );
    info.insert( {"FTICF", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    info.insert( {"FTICS", {A::SUMMARY_FIELD, "Tracer Injection Concentration"}} );
    info.insert( {"FTPCF", {A::SUMMARY_FIELD, "Tracer Production"}} );
    info.insert( {"FTPCS", {A::SUMMARY_FIELD, "Tracer Production"}} );
    info.insert( {"FMPR", {A::SUMMARY_FIELD, "Methane Production Rate"}} );
    info.insert( {"FMPT", {A::SUMMARY_FIELD, "Methane Production Total"}} );
    info.insert( {"FMIR", {A::SUMMARY_FIELD, "Methane Injection Rate"}} );
    info.insert( {"FMIT", {A::SUMMARY_FIELD, "Methane Injection Total"}} );
    info.insert( {"FCGC", {A::SUMMARY_FIELD, "Bulk Coal Gas Concentration"}} );
    info.insert( {"FCSC", {A::SUMMARY_FIELD, "Bulk Coal Solvent Concentration"}} );
    info.insert( {"FTPRFOA", {A::SUMMARY_FIELD, "Production Rate"}} );
    info.insert( {"FTPTFOA", {A::SUMMARY_FIELD, "Production Total"}} );
    info.insert( {"FTIRFOA", {A::SUMMARY_FIELD, "Injection Rate"}} );
    info.insert( {"FTITFOA", {A::SUMMARY_FIELD, "Injection Total"}} );
    info.insert( {"FTIPTFOA", {A::SUMMARY_FIELD, "In Solution"}} );
    info.insert( {"FTADSFOA", {A::SUMMARY_FIELD, "Adsorption total"}} );
    info.insert( {"FTDCYFOA", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    info.insert( {"FTMOBFOA", {A::SUMMARY_FIELD, "Gas mobility factor"}} );
    info.insert( {"FTPRFOA", {A::SUMMARY_FIELD, "Production Rate"}} );
    info.insert( {"FTPTFOA", {A::SUMMARY_FIELD, "Production Total"}} );
    info.insert( {"FTIRFOA", {A::SUMMARY_FIELD, "Injection Rate"}} );
    info.insert( {"FTITFOA", {A::SUMMARY_FIELD, "Injection Total"}} );
    info.insert( {"FTIPTFOA", {A::SUMMARY_FIELD, "In Solution"}} );
    info.insert( {"FTADSFOA", {A::SUMMARY_FIELD, "Adsorption total"}} );
    info.insert( {"FTDCYFOA", {A::SUMMARY_FIELD, "Decayed tracer"}} );
    info.insert( {"FTMOBFOA", {A::SUMMARY_FIELD, "Gas mobility factor"}} );
    info.insert( {"FSGR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    info.insert( {"FGSR", {A::SUMMARY_FIELD, "Sales Gas Rate"}} );
    info.insert( {"FSGT", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    info.insert( {"FGST", {A::SUMMARY_FIELD, "Sales Gas Total"}} );
    info.insert( {"FGDC", {A::SUMMARY_FIELD, "Gas Delivery Capacity"}} );
    info.insert( {"FGDCQ", {A::SUMMARY_FIELD, "Field/Group Gas DCQ"}} );
    info.insert( {"FGCR", {A::SUMMARY_FIELD, "Gas consumption rate, at and below this group"}} );
    info.insert( {"FGCT", {A::SUMMARY_FIELD, "Gas consumption cumulative total, at and below this group"}} );
    info.insert( {"FFGR", {A::SUMMARY_FIELD, "Fuel Gas rate, at and below this group"}} );
    info.insert( {"FFGT", {A::SUMMARY_FIELD, "Fuel Gas cumulative total, at and below this group"}} );
    info.insert( {"FGIMR", {A::SUMMARY_FIELD, "Gas import rate, at and below this group"}} );
    info.insert( {"FGIMT", {A::SUMMARY_FIELD, "Gas import cumulative total, at and below this group"}} );
    info.insert( {"FGLIR", {A::SUMMARY_FIELD, "Gas Lift Injection Rate"}} );
    info.insert( {"FGCV", {A::SUMMARY_FIELD, "Gas Calorific Value"}} );
    info.insert( {"FGQ", {A::SUMMARY_FIELD, "Gas molar Quality"}} );
    info.insert( {"FEPR", {A::SUMMARY_FIELD, "Energy Production Rate"}} );
    info.insert( {"FEPT", {A::SUMMARY_FIELD, "Energy Production Total"}} );
    info.insert( {"FESR", {A::SUMMARY_FIELD, "Energy Sales Rate"}} );
    info.insert( {"FEST", {A::SUMMARY_FIELD, "Energy Sales Total"}} );
    info.insert( {"FEDC", {A::SUMMARY_FIELD, "Energy Delivery Capacity"}} );
    info.insert( {"FEDCQ", {A::SUMMARY_FIELD, "Energy DCQ"}} );
    info.insert( {"FCPR", {A::SUMMARY_FIELD, "Polymer Production Rate"}} );
    info.insert( {"FCPC", {A::SUMMARY_FIELD, "Polymer Production Concentration"}} );
    info.insert( {"FCPT", {A::SUMMARY_FIELD, "Polymer Production Total"}} );
    info.insert( {"FCIR", {A::SUMMARY_FIELD, "Polymer Injection Rate"}} );
    info.insert( {"FCIC", {A::SUMMARY_FIELD, "Polymer Injection Concentration"}} );
    info.insert( {"FCIT", {A::SUMMARY_FIELD, "Polymer Injection Total"}} );
    info.insert( {"FCIP", {A::SUMMARY_FIELD, "Polymer In Solution"}} );
    info.insert( {"FCAD", {A::SUMMARY_FIELD, "Polymer Adsorption total"}} );
    info.insert( {"FSPR", {A::SUMMARY_FIELD, "Salt Production Rate"}} );
    info.insert( {"FSPT", {A::SUMMARY_FIELD, "Salt Production Total"}} );
    info.insert( {"FSIR", {A::SUMMARY_FIELD, "Salt Injection Rate"}} );
    info.insert( {"FSIT", {A::SUMMARY_FIELD, "Salt Injection Total"}} );
    info.insert( {"FSIP", {A::SUMMARY_FIELD, "Salt In Place"}} );
    info.insert( {"PSSPR", {A::SUMMARY_FIELD, "Log of the pressure change per unit time"}} );
    info.insert( {"PSSSO", {A::SUMMARY_FIELD, "Log of the oil saturation change per unit time"}} );
    info.insert( {"PSSSW", {A::SUMMARY_FIELD, "Log of the water saturation change per unit time"}} );
    info.insert( {"PSSSG", {A::SUMMARY_FIELD, "Log of the gas saturation change per unit time"}} );
    info.insert( {"PSSSC", {A::SUMMARY_FIELD, "Log of the salt concentration change per unit time"}} );
    info.insert( {"FNPR", {A::SUMMARY_FIELD, "Solvent Production Rate"}} );
    info.insert( {"FNPT", {A::SUMMARY_FIELD, "Solvent Production Total"}} );
    info.insert( {"FNIR", {A::SUMMARY_FIELD, "Solvent Injection Rate"}} );
    info.insert( {"FNIT", {A::SUMMARY_FIELD, "Solvent Injection Total"}} );
    info.insert( {"FNIP", {A::SUMMARY_FIELD, "Solvent In Place"}} );
    info.insert( {"FTPRSUR", {A::SUMMARY_FIELD, "Production Rate"}} );
    info.insert( {"FTPTSUR", {A::SUMMARY_FIELD, "Production Total"}} );
    info.insert( {"FTIRSUR", {A::SUMMARY_FIELD, "Injection Rate"}} );
    info.insert( {"FTITSUR", {A::SUMMARY_FIELD, "Injection Total"}} );
    info.insert( {"FTIPTSUR", {A::SUMMARY_FIELD, "In Solution"}} );
    info.insert( {"FTADSUR", {A::SUMMARY_FIELD, "Adsorption total"}} );
    info.insert( {"FTPRALK", {A::SUMMARY_FIELD, "Production Rate"}} );
    info.insert( {"FTPTALK", {A::SUMMARY_FIELD, "Production Total"}} );
    info.insert( {"FTIRALK", {A::SUMMARY_FIELD, "Injection Rate"}} );
    info.insert( {"FTITALK", {A::SUMMARY_FIELD, "Injection Total"}} );
    info.insert( {"FU", {A::SUMMARY_FIELD, "User-defined field quantity"}} );

    info.insert( {"GOPR", {A::SUMMARY_WELL_GROUP, "Oil Production Rate"}} );
    info.insert( {"GOPRA", {A::SUMMARY_WELL_GROUP, "Oil Production Rate above GOC"}} );
    info.insert( {"GOPRB", {A::SUMMARY_WELL_GROUP, "Oil Production Rate below GOC"}} );
    info.insert( {"GOPTA", {A::SUMMARY_WELL_GROUP, "Oil Production Total above GOC"}} );
    info.insert( {"GOPTB", {A::SUMMARY_WELL_GROUP, "Oil Production Total below GOC"}} );
    info.insert( {"GOPR1", {A::SUMMARY_WELL_GROUP, "Oil Production Rate above GOC"}} );
    info.insert( {"GOPR2", {A::SUMMARY_WELL_GROUP, "Oil Production Rate below GOC"}} );
    info.insert( {"GOPT1", {A::SUMMARY_WELL_GROUP, "Oil Production Total above GOC"}} );
    info.insert( {"GOPT2", {A::SUMMARY_WELL_GROUP, "Oil Production Total below GOC"}} );
    info.insert( {"GOMR", {A::SUMMARY_WELL_GROUP, "Oil Mass Rate"}} );
    info.insert( {"GOMT", {A::SUMMARY_WELL_GROUP, "Oil Mass Total"}} );
    info.insert( {"GODN", {A::SUMMARY_WELL_GROUP, "Oil Density at Surface Conditions"}} );
    info.insert( {"GOPRH", {A::SUMMARY_WELL_GROUP, "Oil Production Rate History"}} );
    info.insert( {"GOPRT", {A::SUMMARY_WELL_GROUP, "Oil Production Rate Target/Limit"}} );
    info.insert( {"GOPRL", {A::SUMMARY_WELL_GROUP, "Oil Production Rate Target/Limit"}} );
    info.insert( {"GOPRF", {A::SUMMARY_WELL_GROUP, "Free Oil Production Rate"}} );
    info.insert( {"GOPRS", {A::SUMMARY_WELL_GROUP, "Solution Oil Production Rate"}} );
    info.insert( {"GOPT", {A::SUMMARY_WELL_GROUP, "Oil Production Total"}} );
    info.insert( {"GOPTH", {A::SUMMARY_WELL_GROUP, "Oil Production Total History"}} );
    info.insert( {"GOPTF", {A::SUMMARY_WELL_GROUP, "Free Oil Production Total"}} );
    info.insert( {"GOPTS", {A::SUMMARY_WELL_GROUP, "Solution Oil Production Total"}} );
    info.insert( {"GOIR", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate"}} );
    info.insert( {"GOIRH", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate History"}} );
    info.insert( {"GOIRT", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate Target/Limit"}} );
    info.insert( {"GOIRL", {A::SUMMARY_WELL_GROUP, "Oil Injection Rate Target/Limit"}} );
    info.insert( {"GOIT", {A::SUMMARY_WELL_GROUP, "Oil Injection Total"}} );
    info.insert( {"GOITH", {A::SUMMARY_WELL_GROUP, "Oil Injection Total History"}} );
    info.insert( {"GOPP", {A::SUMMARY_WELL_GROUP, "Oil Potential Production rate"}} );
    info.insert( {"GOPP2", {A::SUMMARY_WELL_GROUP, "Oil Potential Production rate"}} );
    info.insert( {"GOPI", {A::SUMMARY_WELL_GROUP, "Oil Potential Injection rate"}} );
    info.insert( {"GOPI2", {A::SUMMARY_WELL_GROUP, "Oil Potential Injection rate"}} );
    info.insert( {"GOPGR", {A::SUMMARY_WELL_GROUP, "Oil Production Guide Rate"}} );
    info.insert( {"GOIGR", {A::SUMMARY_WELL_GROUP, "Oil Injection Guide Rate"}} );
    info.insert( {"GOVPR", {A::SUMMARY_WELL_GROUP, "Oil Voidage Production Rate"}} );
    info.insert( {"GOVPT", {A::SUMMARY_WELL_GROUP, "Oil Voidage Production Total"}} );
    info.insert( {"GOVIR", {A::SUMMARY_WELL_GROUP, "Oil Voidage Injection Rate"}} );
    info.insert( {"GOVIT", {A::SUMMARY_WELL_GROUP, "Oil Voidage Injection Total"}} );
    info.insert( {"GOnPR", {A::SUMMARY_WELL_GROUP, "nth separator stage oil rate"}} );
    info.insert( {"GOnPT", {A::SUMMARY_WELL_GROUP, "nth separator stage oil total"}} );
    info.insert( {"GEOR", {A::SUMMARY_WELL_GROUP, "Export Oil Rate"}} );
    info.insert( {"GEOT", {A::SUMMARY_WELL_GROUP, "Export Oil Total"}} );
    info.insert( {"GEOMF", {A::SUMMARY_WELL_GROUP, "Export Oil Mole Fraction"}} );
    info.insert( {"GWPR", {A::SUMMARY_WELL_GROUP, "Water Production Rate"}} );
    info.insert( {"GWMR", {A::SUMMARY_WELL_GROUP, "Water Mass Rate"}} );
    info.insert( {"GWMT", {A::SUMMARY_WELL_GROUP, "Water Mass Total"}} );
    info.insert( {"GWPRH", {A::SUMMARY_WELL_GROUP, "Water Production Rate History"}} );
    info.insert( {"GWPRT", {A::SUMMARY_WELL_GROUP, "Water Production Rate Target/Limit"}} );
    info.insert( {"GWPRL", {A::SUMMARY_WELL_GROUP, "Water Production Rate Target/Limit"}} );
    info.insert( {"GWPT", {A::SUMMARY_WELL_GROUP, "Water Production Total"}} );
    info.insert( {"GWPTH", {A::SUMMARY_WELL_GROUP, "Water Production Total History"}} );
    info.insert( {"GWIR", {A::SUMMARY_WELL_GROUP, "Water Injection Rate"}} );
    info.insert( {"GWIRH", {A::SUMMARY_WELL_GROUP, "Water Injection Rate History"}} );
    info.insert( {"GWIRT", {A::SUMMARY_WELL_GROUP, "Water Injection Rate Target/Limit"}} );
    info.insert( {"GWIRL", {A::SUMMARY_WELL_GROUP, "Water Injection Rate Target/Limit"}} );
    info.insert( {"GWIT", {A::SUMMARY_WELL_GROUP, "Water Injection Total"}} );
    info.insert( {"GWITH", {A::SUMMARY_WELL_GROUP, "Water Injection Total History"}} );
    info.insert( {"GWPP", {A::SUMMARY_WELL_GROUP, "Water Potential Production rate"}} );
    info.insert( {"GWPP2", {A::SUMMARY_WELL_GROUP, "Water Potential Production rate"}} );
    info.insert( {"GWPI", {A::SUMMARY_WELL_GROUP, "Water Potential Injection rate"}} );
    info.insert( {"GWPI2", {A::SUMMARY_WELL_GROUP, "Water Potential Injection rate"}} );
    info.insert( {"GWPGR", {A::SUMMARY_WELL_GROUP, "Water Production Guide Rate"}} );
    info.insert( {"GWIGR", {A::SUMMARY_WELL_GROUP, "Water Injection Guide Rate"}} );
    info.insert( {"GWVPR", {A::SUMMARY_WELL_GROUP, "Water Voidage Production Rate"}} );
    info.insert( {"GWVPT", {A::SUMMARY_WELL_GROUP, "Water Voidage Production Total"}} );
    info.insert( {"GWVIR", {A::SUMMARY_WELL_GROUP, "Water Voidage Injection Rate"}} );
    info.insert( {"GWVIT", {A::SUMMARY_WELL_GROUP, "Water Voidage Injection Total"}} );
    info.insert( {"GWPIR", {A::SUMMARY_WELL_GROUP, "Ratio of produced water to injected water (percentage)"}} );
    info.insert( {"GWMPR", {A::SUMMARY_WELL_GROUP, "Water component molar production rate"}} );
    info.insert( {"GWMPT", {A::SUMMARY_WELL_GROUP, "Water component molar production total"}} );
    info.insert( {"GWMIR", {A::SUMMARY_WELL_GROUP, "Water component molar injection rate"}} );
    info.insert( {"GWMIT", {A::SUMMARY_WELL_GROUP, "Water component molar injection total"}} );
    info.insert( {"GGPR", {A::SUMMARY_WELL_GROUP, "Gas Production Rate"}} );
    info.insert( {"GGPRA", {A::SUMMARY_WELL_GROUP, "Gas Production Rate above"}} );
    info.insert( {"GGPRB", {A::SUMMARY_WELL_GROUP, "Gas Production Rate below"}} );
    info.insert( {"GGPTA", {A::SUMMARY_WELL_GROUP, "Gas Production Total above"}} );
    info.insert( {"GGPTB", {A::SUMMARY_WELL_GROUP, "Gas Production Total below"}} );
    info.insert( {"GGPR1", {A::SUMMARY_WELL_GROUP, "Gas Production Rate above GOC"}} );
    info.insert( {"GGPR2", {A::SUMMARY_WELL_GROUP, "Gas Production Rate below GOC"}} );
    info.insert( {"GGPT1", {A::SUMMARY_WELL_GROUP, "Gas Production Total above GOC"}} );
    info.insert( {"GGPT2", {A::SUMMARY_WELL_GROUP, "Gas Production Total below GOC"}} );
    info.insert( {"GGMR", {A::SUMMARY_WELL_GROUP, "Gas Mass Rate"}} );
    info.insert( {"GGMT", {A::SUMMARY_WELL_GROUP, "Gas Mass Total"}} );
    info.insert( {"GGDN", {A::SUMMARY_WELL_GROUP, "Gas Density at Surface Conditions"}} );
    info.insert( {"GGPRH", {A::SUMMARY_WELL_GROUP, "Gas Production Rate History"}} );
    info.insert( {"GGPRT", {A::SUMMARY_WELL_GROUP, "Gas Production Rate Target/Limit"}} );
    info.insert( {"GGPRL", {A::SUMMARY_WELL_GROUP, "Gas Production Rate Target/Limit"}} );
    info.insert( {"GGPRF", {A::SUMMARY_WELL_GROUP, "Free Gas Production Rate"}} );
    info.insert( {"GGPRS", {A::SUMMARY_WELL_GROUP, "Solution Gas Production Rate"}} );
    info.insert( {"GGPT", {A::SUMMARY_WELL_GROUP, "Gas Production Total"}} );
    info.insert( {"GGPTH", {A::SUMMARY_WELL_GROUP, "Gas Production Total History"}} );
    info.insert( {"GGPTF", {A::SUMMARY_WELL_GROUP, "Free Gas Production Total"}} );
    info.insert( {"GGPTS", {A::SUMMARY_WELL_GROUP, "Solution Gas Production Total"}} );
    info.insert( {"GGIR", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate"}} );
    info.insert( {"GGIRH", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate History"}} );
    info.insert( {"GGIRT", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate Target/Limit"}} );
    info.insert( {"GGIRL", {A::SUMMARY_WELL_GROUP, "Gas Injection Rate Target/Limit"}} );
    info.insert( {"GGIT", {A::SUMMARY_WELL_GROUP, "Gas Injection Total"}} );
    info.insert( {"GGITH", {A::SUMMARY_WELL_GROUP, "Gas Injection Total History"}} );
    info.insert( {"GGPP", {A::SUMMARY_WELL_GROUP, "Gas Potential Production rate"}} );
    info.insert( {"GGPP2", {A::SUMMARY_WELL_GROUP, "Gas Potential Production rate"}} );
    info.insert( {"GGPPS", {A::SUMMARY_WELL_GROUP, "Solution"}} );
    info.insert( {"GGPPS2", {A::SUMMARY_WELL_GROUP, "Solution"}} );
    info.insert( {"GGPPF", {A::SUMMARY_WELL_GROUP, "Free Gas Potential Production rate"}} );
    info.insert( {"GGPPF2", {A::SUMMARY_WELL_GROUP, "Free Gas Potential Production rate"}} );
    info.insert( {"GGPI", {A::SUMMARY_WELL_GROUP, "Gas Potential Injection rate"}} );
    info.insert( {"GGPI2", {A::SUMMARY_WELL_GROUP, "Gas Potential Injection rate"}} );
    info.insert( {"GGPGR", {A::SUMMARY_WELL_GROUP, "Gas Production Guide Rate"}} );
    info.insert( {"GGIGR", {A::SUMMARY_WELL_GROUP, "Gas Injection Guide Rate"}} );
    info.insert( {"GSGR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    info.insert( {"GGSR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    info.insert( {"GSGT", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    info.insert( {"GGST", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    info.insert( {"GSMF", {A::SUMMARY_WELL_GROUP, "Sales Gas Mole Fraction"}} );
    info.insert( {"GFGR", {A::SUMMARY_WELL_GROUP, "Fuel Gas Rate, at and below this group"}} );
    info.insert( {"GFGT", {A::SUMMARY_WELL_GROUP, "Fuel Gas cumulative Total, at and below this group"}} );
    info.insert( {"GFMF", {A::SUMMARY_WELL_GROUP, "Fuel Gas Mole Fraction"}} );
    info.insert( {"GGCR", {A::SUMMARY_WELL_GROUP, "Gas Consumption Rate, at and below this group"}} );
    info.insert( {"GGCT", {A::SUMMARY_WELL_GROUP, "Gas Consumption Total, at and below this group"}} );
    info.insert( {"GGIMR", {A::SUMMARY_WELL_GROUP, "Gas Import Rate, at and below this group"}} );
    info.insert( {"GGIMT", {A::SUMMARY_WELL_GROUP, "Gas Import Total, at and below this group"}} );
    info.insert( {"GGLIR", {A::SUMMARY_WELL_GROUP, "Gas Lift Injection Rate"}} );
    info.insert( {"GWGPR", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Rate"}} );
    info.insert( {"GWGPT", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Total"}} );
    info.insert( {"GWGPRH", {A::SUMMARY_WELL_GROUP, "Wet Gas Production Rate History"}} );
    info.insert( {"GWGIR", {A::SUMMARY_WELL_GROUP, "Wet Gas Injection Rate"}} );
    info.insert( {"GWGIT", {A::SUMMARY_WELL_GROUP, "Wet Gas Injection Total"}} );
    info.insert( {"GEGR", {A::SUMMARY_WELL_GROUP, "Export Gas Rate"}} );
    info.insert( {"GEGT", {A::SUMMARY_WELL_GROUP, "Export Gas Total"}} );
    info.insert( {"GEMF", {A::SUMMARY_WELL_GROUP, "Export Gas Mole Fraction"}} );
    info.insert( {"GEXGR", {A::SUMMARY_WELL_GROUP, "Excess Gas Rate"}} );
    info.insert( {"GEXGT", {A::SUMMARY_WELL_GROUP, "Excess Gas Total"}} );
    info.insert( {"GRGR", {A::SUMMARY_WELL_GROUP, "Re-injection Gas Rate"}} );
    info.insert( {"GRGT", {A::SUMMARY_WELL_GROUP, "Re-injection Gas Total"}} );
    info.insert( {"GGnPR", {A::SUMMARY_WELL_GROUP, "nth separator stage gas rate"}} );
    info.insert( {"GGnPT", {A::SUMMARY_WELL_GROUP, "nth separator stage gas total"}} );
    info.insert( {"GGVPR", {A::SUMMARY_WELL_GROUP, "Gas Voidage Production Rate"}} );
    info.insert( {"GGVPT", {A::SUMMARY_WELL_GROUP, "Gas Voidage Production Total"}} );
    info.insert( {"GGVIR", {A::SUMMARY_WELL_GROUP, "Gas Voidage Injection Rate"}} );
    info.insert( {"GGVIT", {A::SUMMARY_WELL_GROUP, "Gas Voidage Injection Total"}} );
    info.insert( {"GGQ", {A::SUMMARY_WELL_GROUP, "Gas Quality"}} );
    info.insert( {"GLPR", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate"}} );
    info.insert( {"GLPRH", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate History"}} );
    info.insert( {"GLPRT", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate Target/Limit"}} );
    info.insert( {"GLPRL", {A::SUMMARY_WELL_GROUP, "Liquid Production Rate Target/Limit"}} );
    info.insert( {"GLPT", {A::SUMMARY_WELL_GROUP, "Liquid Production Total"}} );
    info.insert( {"GLPTH", {A::SUMMARY_WELL_GROUP, "Liquid Production Total History"}} );
    info.insert( {"GVPR", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate"}} );
    info.insert( {"GVPRT", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate Target/Limit"}} );
    info.insert( {"GVPRL", {A::SUMMARY_WELL_GROUP, "Res Volume Production Rate Target/Limit"}} );
    info.insert( {"GVPT", {A::SUMMARY_WELL_GROUP, "Res Volume Production Total"}} );
    info.insert( {"GVPGR", {A::SUMMARY_WELL_GROUP, "Res Volume Production Guide Rate"}} );
    info.insert( {"GVIR", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate"}} );
    info.insert( {"GVIRT", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate Target/Limit"}} );
    info.insert( {"GVIRL", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Rate Target/Limit"}} );
    info.insert( {"GVIT", {A::SUMMARY_WELL_GROUP, "Res Volume Injection Total"}} );
    info.insert( {"GWCT", {A::SUMMARY_WELL_GROUP, "Water Cut"}} );
    info.insert( {"GWCTH", {A::SUMMARY_WELL_GROUP, "Water Cut History"}} );
    info.insert( {"GGOR", {A::SUMMARY_WELL_GROUP, "Gas-Oil Ratio"}} );
    info.insert( {"GGORH", {A::SUMMARY_WELL_GROUP, "Gas-Oil Ratio History"}} );
    info.insert( {"GOGR", {A::SUMMARY_WELL_GROUP, "Oil-Gas Ratio"}} );
    info.insert( {"GOGRH", {A::SUMMARY_WELL_GROUP, "Oil-Gas Ratio History"}} );
    info.insert( {"GWGR", {A::SUMMARY_WELL_GROUP, "Water-Gas Ratio"}} );
    info.insert( {"GWGRH", {A::SUMMARY_WELL_GROUP, "Water-Gas Ratio History"}} );
    info.insert( {"GGLR", {A::SUMMARY_WELL_GROUP, "Gas-Liquid Ratio"}} );
    info.insert( {"GGLRH", {A::SUMMARY_WELL_GROUP, "Gas-Liquid Ratio History"}} );
    info.insert( {"GMCTP", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Production"}} );
    info.insert( {"GMCTW", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Water Injection"}} );
    info.insert( {"GMCTG", {A::SUMMARY_WELL_GROUP, "Mode of Control for group Gas Injection"}} );
    info.insert( {"GMWPT", {A::SUMMARY_WELL_GROUP, "Total number of production wells"}} );
    info.insert( {"GMWPR", {A::SUMMARY_WELL_GROUP, "Number of production wells currently flowing"}} );
    info.insert( {"GMWPA", {A::SUMMARY_WELL_GROUP, "Number of abandoned production wells"}} );
    info.insert( {"GMWPU", {A::SUMMARY_WELL_GROUP, "Number of unused production wells"}} );
    info.insert( {"GMWPG", {A::SUMMARY_WELL_GROUP, "Number of producers on group control"}} );
    info.insert( {"GMWPO", {A::SUMMARY_WELL_GROUP, "Number of producers controlled by own oil rate limit"}} );
    info.insert( {"GMWPS", {A::SUMMARY_WELL_GROUP, "Number of producers on own surface rate limit control"}} );
    info.insert( {"GMWPV", {A::SUMMARY_WELL_GROUP, "Number of producers on own reservoir volume rate limit control"}} );
    info.insert( {"GMWPP", {A::SUMMARY_WELL_GROUP, "Number of producers on pressure control"}} );
    info.insert( {"GMWPL", {A::SUMMARY_WELL_GROUP, "Number of producers using artificial lift"}} );
    info.insert( {"GMWIT", {A::SUMMARY_WELL_GROUP, "Total number of injection wells"}} );
    info.insert( {"GMWIN", {A::SUMMARY_WELL_GROUP, "Number of injection wells currently flowing"}} );
    info.insert( {"GMWIA", {A::SUMMARY_WELL_GROUP, "Number of abandoned injection wells"}} );
    info.insert( {"GMWIU", {A::SUMMARY_WELL_GROUP, "Number of unused injection wells"}} );
    info.insert( {"GMWIG", {A::SUMMARY_WELL_GROUP, "Number of injectors on group control"}} );
    info.insert( {"GMWIS", {A::SUMMARY_WELL_GROUP, "Number of injectors on own surface rate limit control"}} );
    info.insert( {"GMWIV", {A::SUMMARY_WELL_GROUP, "Number of injectors on own reservoir volume rate limit control"}} );
    info.insert( {"GMWIP", {A::SUMMARY_WELL_GROUP, "Number of injectors on pressure control"}} );
    info.insert( {"GMWDR", {A::SUMMARY_WELL_GROUP, "Number of drilling events this timestep"}} );
    info.insert( {"GMWDT", {A::SUMMARY_WELL_GROUP, "Number of drilling events in total"}} );
    info.insert( {"GMWWO", {A::SUMMARY_WELL_GROUP, "Number of workover events this timestep"}} );
    info.insert( {"GMWWT", {A::SUMMARY_WELL_GROUP, "Number of workover events in total"}} );
    info.insert( {"GEPR", {A::SUMMARY_WELL_GROUP, "Energy Production Rate"}} );
    info.insert( {"GEPT", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    info.insert( {"GEFF", {A::SUMMARY_WELL_GROUP, "Efficiency Factor"}} );
    info.insert( {"GNLPR", {A::SUMMARY_WELL_GROUP, "NGL Production Rate"}} );
    info.insert( {"GNLPT", {A::SUMMARY_WELL_GROUP, "NGL Production Total"}} );
    info.insert( {"GNLPRH", {A::SUMMARY_WELL_GROUP, "NGL Production Rate History"}} );
    info.insert( {"GNLPTH", {A::SUMMARY_WELL_GROUP, "NGL Production Total History"}} );
    info.insert( {"GAMF", {A::SUMMARY_WELL_GROUP, "Component aqueous mole fraction, from producing completions"}} );
    info.insert( {"GXMF", {A::SUMMARY_WELL_GROUP, "Liquid Mole Fraction"}} );
    info.insert( {"GYMF", {A::SUMMARY_WELL_GROUP, "Vapor Mole Fraction"}} );
    info.insert( {"GXMFn", {A::SUMMARY_WELL_GROUP, "Liquid Mole Fraction for nth separator stage"}} );
    info.insert( {"GYMFn", {A::SUMMARY_WELL_GROUP, "Vapor Mole Fraction for nth separator stage"}} );
    info.insert( {"GZMF", {A::SUMMARY_WELL_GROUP, "Total Mole Fraction"}} );
    info.insert( {"GCMPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Production Rates"}} );
    info.insert( {"GCMPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    info.insert( {"GCMIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Injection Rates"}} );
    info.insert( {"GCMIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Molar Injection Totals"}} );
    info.insert( {"GHMIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Injection Rate"}} );
    info.insert( {"GHMIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Injection Total"}} );
    info.insert( {"GHMPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Production Rate"}} );
    info.insert( {"GHMPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Molar Production Total"}} );
    info.insert( {"GCHMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    info.insert( {"GCHMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component"}} );
    info.insert( {"GCWGPR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Production Rate"}} );
    info.insert( {"GCWGPT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Production Total"}} );
    info.insert( {"GCWGIR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    info.insert( {"GCWGIT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon Component Wet Gas Injection Total"}} );
    info.insert( {"GCGMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCGMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCOMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCOMT", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCNMR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the NGL phase"}} );
    info.insert( {"GCNWR", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component mass rates in the NGL phase"}} );
    info.insert( {"GCGMRn",
                  {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert(
        {"GCGRn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert( {"GCOMRn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCORn", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GMUF", {A::SUMMARY_WELL_GROUP, "Make-up fraction"}} );
    info.insert( {"GAMR", {A::SUMMARY_WELL_GROUP, "Make-up gas rate"}} );
    info.insert( {"GAMT", {A::SUMMARY_WELL_GROUP, "Make-up gas total"}} );
    info.insert(
        {"GGSPR", {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for gas"}} );
    info.insert( {"GGSRL",
                  {A::SUMMARY_WELL_GROUP,
                   "Maximum tested rate sustained for the test period during the most recent sustainable "
                   "capacity test for gas"}} );
    info.insert( {"GGSRU",
                  {A::SUMMARY_WELL_GROUP,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for "
                   "gas"}} );
    info.insert( {"GGSSP",
                  {A::SUMMARY_WELL_GROUP,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test "
                   "for gas"}} );
    info.insert( {"GGSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for gas"}} );
    info.insert(
        {"GOSPR", {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for oil"}} );
    info.insert( {"GOSRL",
                  {A::SUMMARY_WELL_GROUP,
                   "Maximum tested rate sustained for the test period during the most recent sustainable "
                   "capacity test for oil"}} );
    info.insert( {"GOSRU",
                  {A::SUMMARY_WELL_GROUP,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for "
                   "oil"}} );
    info.insert( {"GOSSP",
                  {A::SUMMARY_WELL_GROUP,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test "
                   "for oil"}} );
    info.insert( {"GOSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for oil"}} );
    info.insert(
        {"GWSPR", {A::SUMMARY_WELL_GROUP, "Target sustainable rate for most recent sustainable capacity test for water"}} );
    info.insert( {"GWSRL",
                  {A::SUMMARY_WELL_GROUP,
                   "Maximum tested rate sustained for the test period during the most recent sustainable "
                   "capacity test for water"}} );
    info.insert( {"GWSRU",
                  {A::SUMMARY_WELL_GROUP,
                   "Minimum tested rate not sustained for the test period during the most recent "
                   "sustainable capacity test for "
                   "water"}} );
    info.insert( {"GWSSP",
                  {A::SUMMARY_WELL_GROUP,
                   "Period for which target sustainable rate could be maintained for the most recent "
                   "sustainable capacity test "
                   "for water"}} );
    info.insert( {"GWSTP", {A::SUMMARY_WELL_GROUP, "Test period for the most recent sustainable capacity test for water"}} );
    info.insert( {"GGPRG", {A::SUMMARY_WELL_GROUP, "Gas production rate"}} );
    info.insert( {"GOPRG", {A::SUMMARY_WELL_GROUP, "Oil production rate"}} );
    info.insert( {"GNLPRG", {A::SUMMARY_WELL_GROUP, "NGL production rate"}} );
    info.insert( {"GXMFG", {A::SUMMARY_WELL_GROUP, "Liquid mole fraction"}} );
    info.insert( {"GYMFG", {A::SUMMARY_WELL_GROUP, "Vapor mole fraction"}} );
    info.insert( {"GCOMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component"}} );
    info.insert( {"GCGMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the gas phase"}} );
    info.insert( {"GCNMRG", {A::SUMMARY_WELL_GROUP, "Hydrocarbon component molar rates in the NGL phase"}} );
    info.insert( {"GTPR", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    info.insert( {"GTPT", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    info.insert( {"GTPC", {A::SUMMARY_WELL_GROUP, "Tracer Production Concentration"}} );
    info.insert( {"GTIR", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    info.insert( {"GTIT", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    info.insert( {"GTIC", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    info.insert( {"GTMR", {A::SUMMARY_WELL_GROUP, "Traced mass Rate"}} );
    info.insert( {"GTMT", {A::SUMMARY_WELL_GROUP, "Traced mass Total"}} );
    info.insert( {"GTQR", {A::SUMMARY_WELL_GROUP, "Traced molar Rate"}} );
    info.insert( {"GTCM", {A::SUMMARY_WELL_GROUP, "Tracer Carrier molar Rate"}} );
    info.insert( {"GTMF", {A::SUMMARY_WELL_GROUP, "Traced molar fraction"}} );
    info.insert( {"GTVL", {A::SUMMARY_WELL_GROUP, "Traced liquid volume rate"}} );
    info.insert( {"GTVV", {A::SUMMARY_WELL_GROUP, "Traced vapor volume rate"}} );
    info.insert( {"GTTL", {A::SUMMARY_WELL_GROUP, "Traced liquid volume total"}} );
    info.insert( {"GTTV", {A::SUMMARY_WELL_GROUP, "Traced vapor volume total"}} );
    info.insert( {"GTML", {A::SUMMARY_WELL_GROUP, "Traced mass liquid rate"}} );
    info.insert( {"GTMV", {A::SUMMARY_WELL_GROUP, "Traced mass vapor rate"}} );
    info.insert( {"GTLM", {A::SUMMARY_WELL_GROUP, "Traced mass liquid total"}} );
    info.insert( {"GTVM", {A::SUMMARY_WELL_GROUP, "Traced mass vapor total"}} );
    info.insert( {"GAPI", {A::SUMMARY_WELL_GROUP, "Oil API"}} );
    info.insert( {"GSPR", {A::SUMMARY_WELL_GROUP, "Salt Production Rate"}} );
    info.insert( {"GSPT", {A::SUMMARY_WELL_GROUP, "Salt Production Total"}} );
    info.insert( {"GSIR", {A::SUMMARY_WELL_GROUP, "Salt Injection Rate"}} );
    info.insert( {"GSIT", {A::SUMMARY_WELL_GROUP, "Salt Injection Total"}} );
    info.insert( {"GSPC", {A::SUMMARY_WELL_GROUP, "Salt Production Concentration"}} );
    info.insert( {"GSIC", {A::SUMMARY_WELL_GROUP, "Salt Injection Concentration"}} );
    info.insert( {"WTPRANI", {A::SUMMARY_WELL_GROUP, "Anion Production Rate"}} );
    info.insert( {"WTPTANI", {A::SUMMARY_WELL_GROUP, "Anion Production Total"}} );
    info.insert( {"WTIRANI", {A::SUMMARY_WELL_GROUP, "Anion Injection Rate"}} );
    info.insert( {"WTITANI", {A::SUMMARY_WELL_GROUP, "Anion Injection Total"}} );
    info.insert( {"WTPRCAT", {A::SUMMARY_WELL_GROUP, "Cation Production Rate"}} );
    info.insert( {"WTPTCAT", {A::SUMMARY_WELL_GROUP, "Cation Production Total"}} );
    info.insert( {"WTIRCAT", {A::SUMMARY_WELL_GROUP, "Cation Injection Rate"}} );
    info.insert( {"WTITCAT", {A::SUMMARY_WELL_GROUP, "Cation Injection Total"}} );
    info.insert( {"GTPCHEA", {A::SUMMARY_WELL_GROUP, "Production Temperature"}} );
    info.insert( {"GTICHEA", {A::SUMMARY_WELL_GROUP, "Injection Temperature"}} );
    info.insert( {"GTPRHEA", {A::SUMMARY_WELL_GROUP, "Energy flows"}} );
    info.insert( {"GTPTHEA", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    info.insert( {"GTIRHEA", {A::SUMMARY_WELL_GROUP, "Energy flows"}} );
    info.insert( {"GTITHEA", {A::SUMMARY_WELL_GROUP, "Energy Injection Total"}} );
    info.insert( {"GTPR", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    info.insert( {"GTPT", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    info.insert( {"GTPC", {A::SUMMARY_WELL_GROUP, "Tracer Production Concentration"}} );
    info.insert( {"GTIR", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    info.insert( {"GTIT", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    info.insert( {"GTIC", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    info.insert( {"GTIRF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    info.insert( {"GTIRS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Rate"}} );
    info.insert( {"GTPRF", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    info.insert( {"GTPRS", {A::SUMMARY_WELL_GROUP, "Tracer Production Rate"}} );
    info.insert( {"GTITF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    info.insert( {"GTITS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Total"}} );
    info.insert( {"GTPTF", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    info.insert( {"GTPTS", {A::SUMMARY_WELL_GROUP, "Tracer Production Total"}} );
    info.insert( {"GTICF", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    info.insert( {"GTICS", {A::SUMMARY_WELL_GROUP, "Tracer Injection Concentration"}} );
    info.insert( {"GTPCF", {A::SUMMARY_WELL_GROUP, "Tracer Production"}} );
    info.insert( {"GTPCS", {A::SUMMARY_WELL_GROUP, "Tracer Production"}} );
    info.insert( {"GMPR", {A::SUMMARY_WELL_GROUP, "Methane Production Rate"}} );
    info.insert( {"GMPT", {A::SUMMARY_WELL_GROUP, "Methane Production Total"}} );
    info.insert( {"GMIR", {A::SUMMARY_WELL_GROUP, "Methane Injection Rate"}} );
    info.insert( {"GMIT", {A::SUMMARY_WELL_GROUP, "Methane Injection Total"}} );
    info.insert( {"GTPRFOA", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    info.insert( {"GTPTFOA", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    info.insert( {"GTIRFOA", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    info.insert( {"GTITFOA", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    info.insert( {"GSGR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    info.insert( {"GGSR", {A::SUMMARY_WELL_GROUP, "Sales Gas Rate"}} );
    info.insert( {"GSGT", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    info.insert( {"GGST", {A::SUMMARY_WELL_GROUP, "Sales Gas Total"}} );
    info.insert( {"GGDC", {A::SUMMARY_WELL_GROUP, "Gas Delivery Capacity"}} );
    info.insert( {"GGDCQ", {A::SUMMARY_WELL_GROUP, "Field/Group Gas DCQ"}} );
    info.insert( {"GMCPL", {A::SUMMARY_WELL_GROUP, "Group Multi-level Compressor Level"}} );
    info.insert( {"GPR", {A::SUMMARY_WELL_GROUP, "Group nodal Pressure in network"}} );
    info.insert( {"GPRDC", {A::SUMMARY_WELL_GROUP, "Group Pressure at Delivery Capacity"}} );
    info.insert( {"GGCR", {A::SUMMARY_WELL_GROUP, "Gas consumption rate, at and below this group"}} );
    info.insert( {"GGCT", {A::SUMMARY_WELL_GROUP, "Gas consumption cumulative total, at and below this group"}} );
    info.insert( {"GFGR", {A::SUMMARY_WELL_GROUP, "Fuel Gas rate, at and below this group"}} );
    info.insert( {"GFGT", {A::SUMMARY_WELL_GROUP, "Fuel Gas cumulative total, at and below this group"}} );
    info.insert( {"GGIMR", {A::SUMMARY_WELL_GROUP, "Gas import rate, at and below this group"}} );
    info.insert( {"GGIMT", {A::SUMMARY_WELL_GROUP, "Gas import cumulative total, at and below this group"}} );
    info.insert( {"GPRFP", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in network from end of First Pass"}} );
    info.insert( {"GGPRNBFP",
                  {A::SUMMARY_WELL_GROUP,
                   "Gas flow rate along Group's or node's outlet branch in network, from end of First Pass"}} );
    info.insert( {"GGLIR", {A::SUMMARY_WELL_GROUP, "Gas Lift Injection Rate"}} );
    info.insert( {"GGCV", {A::SUMMARY_WELL_GROUP, "Gas Calorific Value"}} );
    info.insert( {"GGQ", {A::SUMMARY_WELL_GROUP, "Gas molar Quality"}} );
    info.insert( {"GEPR", {A::SUMMARY_WELL_GROUP, "Energy Production Rate"}} );
    info.insert( {"GEPT", {A::SUMMARY_WELL_GROUP, "Energy Production Total"}} );
    info.insert( {"GESR", {A::SUMMARY_WELL_GROUP, "Energy Sales Rate"}} );
    info.insert( {"GEST", {A::SUMMARY_WELL_GROUP, "Energy Sales Total"}} );
    info.insert( {"GEDC", {A::SUMMARY_WELL_GROUP, "Energy Delivery Capacity"}} );
    info.insert( {"GEDCQ", {A::SUMMARY_WELL_GROUP, "Energy DCQ"}} );
    info.insert( {"GPR", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the production network"}} );
    info.insert( {"GPRG", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the gas injection network"}} );
    info.insert( {"GPRW", {A::SUMMARY_WELL_GROUP, "Group or node Pressure in the water injection network"}} );
    info.insert(
        {"GPRB",
         {A::SUMMARY_WELL_GROUP, "Pressure drop along the group's or node's outlet branch in the production network"}} );
    info.insert( {"GPRBG",
                  {A::SUMMARY_WELL_GROUP,
                   "Pressure drop along the group's or node's inlet branch in the gas injection network"}} );
    info.insert( {"GPRBW",
                  {A::SUMMARY_WELL_GROUP,
                   "Pressure drop along the group's or node's inlet branch in the water injection network"}} );
    info.insert( {"GALQ", {A::SUMMARY_WELL_GROUP, "ALQ in the group's or node's outlet branch in the production network"}} );
    info.insert(
        {"GOPRNB",
         {A::SUMMARY_WELL_GROUP, "Oil flow rate along the group's or node's outlet branch in the production network"}} );
    info.insert( {"GWPRNB",
                  {A::SUMMARY_WELL_GROUP,
                   "Water flow rate along the group's or node's outlet branch in the production network"}} );
    info.insert(
        {"GGPRNB",
         {A::SUMMARY_WELL_GROUP, "Gas flow rate along the group's or node's outlet branch in the production network"}} );
    info.insert( {"GLPRNB",
                  {A::SUMMARY_WELL_GROUP,
                   "Liquid flow rate along the group's or node's outlet branch in the production network"}} );
    info.insert( {"GWIRNB",
                  {A::SUMMARY_WELL_GROUP,
                   "Water flow rate along the group's or node's inlet branch in the water injection network"}} );
    info.insert( {"GGIRNB",
                  {A::SUMMARY_WELL_GROUP,
                   "Gas flow rate along the group's or node's inlet branch in the gas injection network"}} );
    info.insert(
        {"GOMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum oil rate as specified with GNETDP in the production network"}} );
    info.insert(
        {"GGMNR",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum gas rate as specified with GNETDP in the production network"}} );
    info.insert( {"GWMNR",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node minimum water rate as specified with GNETDP in the production network"}} );
    info.insert( {"GLMNR",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node minimum liquid rate as specified with GNETDP in the production network"}} );
    info.insert(
        {"GOMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum oil rate as specified with GNETDP in the production network"}} );
    info.insert(
        {"GGMXR",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum gas rate as specified with GNETDP in the production network"}} );
    info.insert( {"GWMXR",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node maximum water rate as specified with GNETDP in the production network"}} );
    info.insert( {"GLMXR",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node maximum liquid rate as specified with GNETDP in the production network"}} );
    info.insert(
        {"GMNP",
         {A::SUMMARY_WELL_GROUP, "Group or node minimum pressure as specified with GNETDP in the production network"}} );
    info.insert(
        {"GMXP",
         {A::SUMMARY_WELL_GROUP, "Group or node maximum pressure as specified with GNETDP in the production network"}} );
    info.insert( {"GPRINC",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node pressure increment as specified with GNETDP in the production network"}} );
    info.insert( {"GPRDEC",
                  {A::SUMMARY_WELL_GROUP,
                   "Group or node pressure decrement as specified with GNETDP in the production network"}} );
    info.insert( {"GCPR", {A::SUMMARY_WELL_GROUP, "Polymer Production Rate"}} );
    info.insert( {"GCPC", {A::SUMMARY_WELL_GROUP, "Polymer Production Concentration"}} );
    info.insert( {"GCPT", {A::SUMMARY_WELL_GROUP, "Polymer Production Total"}} );
    info.insert( {"GCIR", {A::SUMMARY_WELL_GROUP, "Polymer Injection Rate"}} );
    info.insert( {"GCIC", {A::SUMMARY_WELL_GROUP, "Polymer Injection Concentration"}} );
    info.insert( {"GCIT", {A::SUMMARY_WELL_GROUP, "Polymer Injection Total"}} );
    info.insert( {"GSPR", {A::SUMMARY_WELL_GROUP, "Salt Production Rate"}} );
    info.insert( {"GSPT", {A::SUMMARY_WELL_GROUP, "Salt Production Total"}} );
    info.insert( {"GSIR", {A::SUMMARY_WELL_GROUP, "Salt Injection Rate"}} );
    info.insert( {"GSIT", {A::SUMMARY_WELL_GROUP, "Salt Injection Total"}} );
    info.insert( {"GOPRL", {A::SUMMARY_WELL_GROUP, "Group Oil Production Rate Target"}} );
    info.insert( {"GOIRL", {A::SUMMARY_WELL_GROUP, "Group Oil Injection Rate Target"}} );
    info.insert( {"GWPRL", {A::SUMMARY_WELL_GROUP, "Group Water Production Rate Target"}} );
    info.insert( {"GWIRL", {A::SUMMARY_WELL_GROUP, "Group Water Injection Rate Target"}} );
    info.insert( {"GGPRL", {A::SUMMARY_WELL_GROUP, "Group Gas Production Rate Target"}} );
    info.insert( {"GGIRL", {A::SUMMARY_WELL_GROUP, "Group Gas Injection Rate Target"}} );
    info.insert( {"GLPRL", {A::SUMMARY_WELL_GROUP, "Group Liquid Production Rate Target"}} );
    info.insert( {"GVPRL", {A::SUMMARY_WELL_GROUP, "Group reservoir Volume Production Rate Target"}} );
    info.insert( {"GVIRL", {A::SUMMARY_WELL_GROUP, "Group reservoir Volume Injection Rate Target"}} );
    info.insert( {"GNPR", {A::SUMMARY_WELL_GROUP, "Solvent Production Rate"}} );
    info.insert( {"GNPT", {A::SUMMARY_WELL_GROUP, "Solvent Production Total"}} );
    info.insert( {"GNIR", {A::SUMMARY_WELL_GROUP, "Solvent Injection Rate"}} );
    info.insert( {"GNIT", {A::SUMMARY_WELL_GROUP, "Solvent Injection Total"}} );
    info.insert( {"GTPRSUR", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    info.insert( {"GTPTSUR", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    info.insert( {"GTIRSUR", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    info.insert( {"GTITSUR", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    info.insert( {"GTPRALK", {A::SUMMARY_WELL_GROUP, "Production Rate"}} );
    info.insert( {"GTPTALK", {A::SUMMARY_WELL_GROUP, "Production Total"}} );
    info.insert( {"GTIRALK", {A::SUMMARY_WELL_GROUP, "Injection Rate"}} );
    info.insert( {"GTITALK", {A::SUMMARY_WELL_GROUP, "Injection Total"}} );
    info.insert( {"GU", {A::SUMMARY_WELL_GROUP, "User-defined group quantity"}} );

    info.insert( {"WOPR", {A::SUMMARY_WELL, "Oil Production Rate"}} );
    info.insert( {"WOPRA", {A::SUMMARY_WELL, "Oil Production Rate above GOC"}} );
    info.insert( {"WOPRB", {A::SUMMARY_WELL, "Oil Production Rate below GOC"}} );
    info.insert( {"WOPTA", {A::SUMMARY_WELL, "Oil Production Total above GOC"}} );
    info.insert( {"WOPTB", {A::SUMMARY_WELL, "Oil Production Total below GOC"}} );
    info.insert( {"WOPR1", {A::SUMMARY_WELL, "Oil Production Rate above GOC"}} );
    info.insert( {"WOPR2", {A::SUMMARY_WELL, "Oil Production Rate below GOC"}} );
    info.insert( {"WOPT1", {A::SUMMARY_WELL, "Oil Production Total above GOC"}} );
    info.insert( {"WOPT2", {A::SUMMARY_WELL, "Oil Production Total below GOC"}} );
    info.insert( {"WOMR", {A::SUMMARY_WELL, "Oil Mass Rate"}} );
    info.insert( {"WOMT", {A::SUMMARY_WELL, "Oil Mass Total"}} );
    info.insert( {"WODN", {A::SUMMARY_WELL, "Oil Density at Surface Conditions"}} );
    info.insert( {"WOPRH", {A::SUMMARY_WELL, "Oil Production Rate History"}} );
    info.insert( {"WOPRT", {A::SUMMARY_WELL, "Oil Production Rate Target/Limit"}} );
    info.insert( {"WOPRF", {A::SUMMARY_WELL, "Free Oil Production Rate"}} );
    info.insert( {"WOPRS", {A::SUMMARY_WELL, "Solution Oil Production Rate"}} );
    info.insert( {"WOPT", {A::SUMMARY_WELL, "Oil Production Total"}} );
    info.insert( {"WOPTH", {A::SUMMARY_WELL, "Oil Production Total History"}} );
    info.insert( {"WOPTF", {A::SUMMARY_WELL, "Free Oil Production Total"}} );
    info.insert( {"WOPTS", {A::SUMMARY_WELL, "Solution Oil Production Total"}} );
    info.insert( {"WOIR", {A::SUMMARY_WELL, "Oil Injection Rate"}} );
    info.insert( {"WOIRH", {A::SUMMARY_WELL, "Oil Injection Rate History"}} );
    info.insert( {"WOIRT", {A::SUMMARY_WELL, "Oil Injection Rate Target/Limit"}} );
    info.insert( {"WOIT", {A::SUMMARY_WELL, "Oil Injection Total"}} );
    info.insert( {"WOITH", {A::SUMMARY_WELL, "Oil Injection Total History"}} );
    info.insert( {"WOPP", {A::SUMMARY_WELL, "Oil Potential Production rate"}} );
    info.insert( {"WOPP2", {A::SUMMARY_WELL, "Oil Potential Production rate"}} );
    info.insert( {"WOPI", {A::SUMMARY_WELL, "Oil Potential Injection rate"}} );
    info.insert( {"WOPI2", {A::SUMMARY_WELL, "Oil Potential Injection rate"}} );
    info.insert( {"WOPGR", {A::SUMMARY_WELL, "Oil Production Guide Rate"}} );
    info.insert( {"WOIGR", {A::SUMMARY_WELL, "Oil Injection Guide Rate"}} );
    info.insert( {"WOVPR", {A::SUMMARY_WELL, "Oil Voidage Production Rate"}} );
    info.insert( {"WOVPT", {A::SUMMARY_WELL, "Oil Voidage Production Total"}} );
    info.insert( {"WOVIR", {A::SUMMARY_WELL, "Oil Voidage Injection Rate"}} );
    info.insert( {"WOVIT", {A::SUMMARY_WELL, "Oil Voidage Injection Total"}} );
    info.insert( {"WOnPR", {A::SUMMARY_WELL, "nth separator stage oil rate"}} );
    info.insert( {"WOnPT", {A::SUMMARY_WELL, "nth separator stage oil total"}} );
    info.insert( {"WWPR", {A::SUMMARY_WELL, "Water Production Rate"}} );
    info.insert( {"WWMR", {A::SUMMARY_WELL, "Water Mass Rate"}} );
    info.insert( {"WWMT", {A::SUMMARY_WELL, "Water Mass Total"}} );
    info.insert( {"WWPRH", {A::SUMMARY_WELL, "Water Production Rate History"}} );
    info.insert( {"WWPRT", {A::SUMMARY_WELL, "Water Production Rate Target/Limit"}} );
    info.insert( {"WWPT", {A::SUMMARY_WELL, "Water Production Total"}} );
    info.insert( {"WWPTH", {A::SUMMARY_WELL, "Water Production Total History"}} );
    info.insert( {"WWIR", {A::SUMMARY_WELL, "Water Injection Rate"}} );
    info.insert( {"WWIRH", {A::SUMMARY_WELL, "Water Injection Rate History"}} );
    info.insert( {"WWIRT", {A::SUMMARY_WELL, "Water Injection Rate Target/Limit"}} );
    info.insert( {"WWIT", {A::SUMMARY_WELL, "Water Injection Total"}} );
    info.insert( {"WWITH", {A::SUMMARY_WELL, "Water Injection Total History"}} );
    info.insert( {"WWPP", {A::SUMMARY_WELL, "Water Potential Production rate"}} );
    info.insert( {"WWPP2", {A::SUMMARY_WELL, "Water Potential Production rate"}} );
    info.insert( {"WWPI", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    info.insert( {"WWIP", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    info.insert( {"WWPI2", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    info.insert( {"WWIP2", {A::SUMMARY_WELL, "Water Potential Injection rate"}} );
    info.insert( {"WWPGR", {A::SUMMARY_WELL, "Water Production Guide Rate"}} );
    info.insert( {"WWIGR", {A::SUMMARY_WELL, "Water Injection Guide Rate"}} );
    info.insert( {"WWVPR", {A::SUMMARY_WELL, "Water Voidage Production Rate"}} );
    info.insert( {"WWVPT", {A::SUMMARY_WELL, "Water Voidage Production Total"}} );
    info.insert( {"WWVIR", {A::SUMMARY_WELL, "Water Voidage Injection Rate"}} );
    info.insert( {"WWVIT", {A::SUMMARY_WELL, "Water Voidage Injection Total"}} );
    info.insert( {"WWPIR", {A::SUMMARY_WELL, "Ratio of produced water to injected water (percentage)"}} );
    info.insert( {"WWMPR", {A::SUMMARY_WELL, "Water component molar production rate"}} );
    info.insert( {"WWMPT", {A::SUMMARY_WELL, "Water component molar production total"}} );
    info.insert( {"WWMIR", {A::SUMMARY_WELL, "Water component molar injection rate"}} );
    info.insert( {"WWMIT", {A::SUMMARY_WELL, "Water component molar injection total"}} );
    info.insert( {"WGPR", {A::SUMMARY_WELL, "Gas Production Rate"}} );
    info.insert( {"WGPRA", {A::SUMMARY_WELL, "Gas Production Rate above"}} );
    info.insert( {"WGPRB", {A::SUMMARY_WELL, "Gas Production Rate below"}} );
    info.insert( {"WGPTA", {A::SUMMARY_WELL, "Gas Production Total above"}} );
    info.insert( {"WGPTB", {A::SUMMARY_WELL, "Gas Production Total below"}} );
    info.insert( {"WGPR1", {A::SUMMARY_WELL, "Gas Production Rate above GOC"}} );
    info.insert( {"WGPR2", {A::SUMMARY_WELL, "Gas Production Rate below GOC"}} );
    info.insert( {"WGPT1", {A::SUMMARY_WELL, "Gas Production Total above GOC"}} );
    info.insert( {"WGPT2", {A::SUMMARY_WELL, "Gas Production Total below GOC"}} );
    info.insert( {"WGMR", {A::SUMMARY_WELL, "Gas Mass Rate"}} );
    info.insert( {"WGMT", {A::SUMMARY_WELL, "Gas Mass Total"}} );
    info.insert( {"WGDN", {A::SUMMARY_WELL, "Gas Density at Surface Conditions"}} );
    info.insert( {"WGPRH", {A::SUMMARY_WELL, "Gas Production Rate History"}} );
    info.insert( {"WGPRT", {A::SUMMARY_WELL, "Gas Production Rate Target/Limit"}} );
    info.insert( {"WGPRF", {A::SUMMARY_WELL, "Free Gas Production Rate"}} );
    info.insert( {"WGPRS", {A::SUMMARY_WELL, "Solution Gas Production Rate"}} );
    info.insert( {"WGPT", {A::SUMMARY_WELL, "Gas Production Total"}} );
    info.insert( {"WGPTH", {A::SUMMARY_WELL, "Gas Production Total History"}} );
    info.insert( {"WGPTF", {A::SUMMARY_WELL, "Free Gas Production Total"}} );
    info.insert( {"WGPTS", {A::SUMMARY_WELL, "Solution Gas Production Total"}} );
    info.insert( {"WGIR", {A::SUMMARY_WELL, "Gas Injection Rate"}} );
    info.insert( {"WGIRH", {A::SUMMARY_WELL, "Gas Injection Rate History"}} );
    info.insert( {"WGIRT", {A::SUMMARY_WELL, "Gas Injection Rate Target/Limit"}} );
    info.insert( {"WGIT", {A::SUMMARY_WELL, "Gas Injection Total"}} );
    info.insert( {"WGITH", {A::SUMMARY_WELL, "Gas Injection Total History"}} );
    info.insert( {"WGPP", {A::SUMMARY_WELL, "Gas Potential Production rate"}} );
    info.insert( {"WGPP2", {A::SUMMARY_WELL, "Gas Potential Production rate"}} );
    info.insert( {"WGPPS", {A::SUMMARY_WELL, "Solution"}} );
    info.insert( {"WGPPS2", {A::SUMMARY_WELL, "Solution"}} );
    info.insert( {"WGPPF", {A::SUMMARY_WELL, "Free Gas Potential Production rate"}} );
    info.insert( {"WGPPF2", {A::SUMMARY_WELL, "Free Gas Potential Production rate"}} );
    info.insert( {"WGPI", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    info.insert( {"WGIP", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    info.insert( {"WGPI2", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    info.insert( {"WGIP2", {A::SUMMARY_WELL, "Gas Potential Injection rate"}} );
    info.insert( {"WGPGR", {A::SUMMARY_WELL, "Gas Production Guide Rate"}} );
    info.insert( {"WGIGR", {A::SUMMARY_WELL, "Gas Injection Guide Rate"}} );
    info.insert( {"WGLIR", {A::SUMMARY_WELL, "Gas Lift Injection Rate"}} );
    info.insert( {"WWGPR", {A::SUMMARY_WELL, "Wet Gas Production Rate"}} );
    info.insert( {"WWGPT", {A::SUMMARY_WELL, "Wet Gas Production Total"}} );
    info.insert( {"WWGPRH", {A::SUMMARY_WELL, "Wet Gas Production Rate History"}} );
    info.insert( {"WWGIR", {A::SUMMARY_WELL, "Wet Gas Injection Rate"}} );
    info.insert( {"WWGIT", {A::SUMMARY_WELL, "Wet Gas Injection Total"}} );
    info.insert( {"WGnPR", {A::SUMMARY_WELL, "nth separator stage gas rate"}} );
    info.insert( {"WGnPT", {A::SUMMARY_WELL, "nth separator stage gas total"}} );
    info.insert( {"WGVPR", {A::SUMMARY_WELL, "Gas Voidage Production Rate"}} );
    info.insert( {"WGVPT", {A::SUMMARY_WELL, "Gas Voidage Production Total"}} );
    info.insert( {"WGVIR", {A::SUMMARY_WELL, "Gas Voidage Injection Rate"}} );
    info.insert( {"WGVIT", {A::SUMMARY_WELL, "Gas Voidage Injection Total"}} );
    info.insert( {"WGQ", {A::SUMMARY_WELL, "Gas Quality"}} );
    info.insert( {"WLPR", {A::SUMMARY_WELL, "Liquid Production Rate"}} );
    info.insert( {"WLPRH", {A::SUMMARY_WELL, "Liquid Production Rate History"}} );
    info.insert( {"WLPRT", {A::SUMMARY_WELL, "Liquid Production Rate Target/Limit"}} );
    info.insert( {"WLPT", {A::SUMMARY_WELL, "Liquid Production Total"}} );
    info.insert( {"WLPTH", {A::SUMMARY_WELL, "Liquid Production Total History"}} );
    info.insert( {"WVPR", {A::SUMMARY_WELL, "Res Volume Production Rate"}} );
    info.insert( {"WVPRT", {A::SUMMARY_WELL, "Res Volume Production Rate Target/Limit"}} );
    info.insert( {"WVPT", {A::SUMMARY_WELL, "Res Volume Production Total"}} );
    info.insert( {"WVPGR", {A::SUMMARY_WELL, "Res Volume Production Guide Rate"}} );
    info.insert( {"WVIR", {A::SUMMARY_WELL, "Res Volume Injection Rate"}} );
    info.insert( {"WVIRT", {A::SUMMARY_WELL, "Res Volume Injection Rate Target/Limit"}} );
    info.insert( {"WVIT", {A::SUMMARY_WELL, "Res Volume Injection Total"}} );
    info.insert( {"WWCT", {A::SUMMARY_WELL, "Water Cut"}} );
    info.insert( {"WWCTH", {A::SUMMARY_WELL, "Water Cut History"}} );
    info.insert( {"WGOR", {A::SUMMARY_WELL, "Gas-Oil Ratio"}} );
    info.insert( {"WGORH", {A::SUMMARY_WELL, "Gas-Oil Ratio History"}} );
    info.insert( {"WOGR", {A::SUMMARY_WELL, "Oil-Gas Ratio"}} );
    info.insert( {"WOGRH", {A::SUMMARY_WELL, "Oil-Gas Ratio History"}} );
    info.insert( {"WWGR", {A::SUMMARY_WELL, "Water-Gas Ratio"}} );
    info.insert( {"WWGRH", {A::SUMMARY_WELL, "Water-Gas Ratio History"}} );
    info.insert( {"WGLR", {A::SUMMARY_WELL, "Gas-Liquid Ratio"}} );
    info.insert( {"WGLRH", {A::SUMMARY_WELL, "Gas-Liquid Ratio History"}} );
    info.insert( {"WBGLR", {A::SUMMARY_WELL, "Bottom hole Gas-Liquid Ratio"}} );
    info.insert( {"WBHP", {A::SUMMARY_WELL, "Bottom Hole Pressure"}} );
    info.insert( {"WBHPH", {A::SUMMARY_WELL, "Bottom Hole Pressure History,"}} );
    info.insert( {"WBHPT", {A::SUMMARY_WELL, "Bottom Hole Pressure Target/Limit"}} );
    info.insert( {"WTHP", {A::SUMMARY_WELL, "Tubing Head Pressure"}} );
    info.insert( {"WTHPH", {A::SUMMARY_WELL, "Tubing Head Pressure History,"}} );
    info.insert( {"WPI", {A::SUMMARY_WELL, "Productivity Index of well's preferred phase"}} );
    info.insert( {"WPIO", {A::SUMMARY_WELL, "Oil phase PI"}} );
    info.insert( {"WPIG", {A::SUMMARY_WELL, "Gas phase PI"}} );
    info.insert( {"WPIW", {A::SUMMARY_WELL, "Water phase PI"}} );
    info.insert( {"WPIL", {A::SUMMARY_WELL, "Liquid phase PI"}} );
    info.insert( {"WBP", {A::SUMMARY_WELL, "One-point Pressure Average"}} );
    info.insert( {"WBP4", {A::SUMMARY_WELL, "Four-point Pressure Average"}} );
    info.insert( {"WBP5", {A::SUMMARY_WELL, "Five-point Pressure Average"}} );
    info.insert( {"WBP9", {A::SUMMARY_WELL, "Nine-point Pressure Average"}} );
    info.insert( {"WPI1", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP"}} );
    info.insert( {"WPI4", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP4"}} );
    info.insert( {"WPI5", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP5"}} );
    info.insert( {"WPI9", {A::SUMMARY_WELL, "Productivity Index based on the value of WBP9"}} );
    info.insert(
        {"WHD",
         {A::SUMMARY_WELL,
          "Hydraulic head in well based on the reference depth given in HYDRHEAD and the well's reference depth"}} );
    info.insert(
        {"WHDF",
         {A::SUMMARY_WELL,
          "Hydraulic head in well based on the reference depth given in HYDRHEAD and the well's reference depth "
          "calculated at freshwater conditions"}} );
    info.insert( {"WSTAT", {A::SUMMARY_WELL, "Well State Indicator"}} );
    info.insert( {"WMCTL", {A::SUMMARY_WELL, "Mode of Control"}} );
    info.insert( {"WMCON", {A::SUMMARY_WELL, "The number of connections capable of flowing in the well"}} );
    info.insert( {"WEPR", {A::SUMMARY_WELL, "Energy Production Rate"}} );
    info.insert( {"WEPT", {A::SUMMARY_WELL, "Energy Production Total"}} );
    info.insert( {"WEFF", {A::SUMMARY_WELL, "Efficiency Factor"}} );
    info.insert( {"WEFFG", {A::SUMMARY_WELL, "Product of efficiency factors of the well and all its superior groups"}} );
    info.insert( {"WALQ", {A::SUMMARY_WELL, "Well Artificial Lift Quantity"}} );
    info.insert( {"WMVFP", {A::SUMMARY_WELL, "VFP table number used by the well"}} );
    info.insert( {"WNLPR", {A::SUMMARY_WELL, "NGL Production Rate"}} );
    info.insert( {"WNLPT", {A::SUMMARY_WELL, "NGL Production Total"}} );
    info.insert( {"WNLPRH", {A::SUMMARY_WELL, "NGL Production Rate History"}} );
    info.insert( {"WNLPTH", {A::SUMMARY_WELL, "NGL Production Total History"}} );
    info.insert( {"WNLPRT", {A::SUMMARY_WELL, "NGL Production Rate Target"}} );
    info.insert( {"WAMF", {A::SUMMARY_WELL, "Component aqueous mole fraction, from producing completions"}} );
    info.insert( {"WXMF", {A::SUMMARY_WELL, "Liquid Mole Fraction"}} );
    info.insert( {"WYMF", {A::SUMMARY_WELL, "Vapor Mole Fraction"}} );
    info.insert( {"WXMFn", {A::SUMMARY_WELL, "Liquid Mole Fraction for nth separator stage"}} );
    info.insert( {"WYMFn", {A::SUMMARY_WELL, "Vapor Mole Fraction for nth separator stage"}} );
    info.insert( {"WZMF", {A::SUMMARY_WELL, "Total Mole Fraction"}} );
    info.insert( {"WCMPR", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Production Rates"}} );
    info.insert( {"WCMPT", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    info.insert( {"WCMIR", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Injection Rates"}} );
    info.insert( {"WCMIT", {A::SUMMARY_WELL, "Hydrocarbon Component Molar Injection Totals"}} );
    info.insert( {"WCGIR", {A::SUMMARY_WELL, "Hydrocarbon Component Gas Injection Rate"}} );
    info.insert( {"WCGPR", {A::SUMMARY_WELL, "Hydrocarbon Component Gas Production Rate"}} );
    info.insert( {"WCOPR", {A::SUMMARY_WELL, "Hydrocarbon Component Oil Production Rate"}} );
    info.insert( {"WHMIR", {A::SUMMARY_WELL, "Hydrocarbon Molar Injection Rate"}} );
    info.insert( {"WHMIT", {A::SUMMARY_WELL, "Hydrocarbon Molar Injection Total"}} );
    info.insert( {"WHMPR", {A::SUMMARY_WELL, "Hydrocarbon Molar Production Rate"}} );
    info.insert( {"WHMPT", {A::SUMMARY_WELL, "Hydrocarbon Molar Production Total"}} );
    info.insert( {"WCHMR", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    info.insert( {"WCHMT", {A::SUMMARY_WELL, "Hydrocarbon Component"}} );
    info.insert( {"WCWGPR", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Production Rate"}} );
    info.insert( {"WCWGPT", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Production Total"}} );
    info.insert( {"WCWGIR", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Injection Rate"}} );
    info.insert( {"WCWGIT", {A::SUMMARY_WELL, "Hydrocarbon Component Wet Gas Injection Total"}} );
    info.insert( {"WCGMR", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WCGMT", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WCOMR", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WCOMT", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WCNMR", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the NGL phase"}} );
    info.insert( {"WCNWR", {A::SUMMARY_WELL, "Hydrocarbon component mass rates in the NGL phase"}} );
    info.insert(
        {"WCGMRn", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert(
        {"WCGRn", {A::SUMMARY_WELL, "Hydrocarbon component molar rates in the gas phase for nth separator stage"}} );
    info.insert( {"WCOMRn", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WCORn", {A::SUMMARY_WELL, "Hydrocarbon component"}} );
    info.insert( {"WMUF", {A::SUMMARY_WELL, "Make-up fraction"}} );
    info.insert( {"WTHT", {A::SUMMARY_WELL, "Tubing Head Temperature"}} );
    info.insert( {"WMMW", {A::SUMMARY_WELL, "Mean molecular weight of wellstream"}} );
    info.insert( {"WPWE0", {A::SUMMARY_WELL, "Well drilled indicator"}} );
    info.insert( {"WPWE1", {A::SUMMARY_WELL, "Connections opened indicator"}} );
    info.insert( {"WPWE2", {A::SUMMARY_WELL, "Connections closed indicator"}} );
    info.insert( {"WPWE3", {A::SUMMARY_WELL, "Connections closed to bottom indicator"}} );
    info.insert( {"WPWE4", {A::SUMMARY_WELL, "Well stopped indicator"}} );
    info.insert( {"WPWE5", {A::SUMMARY_WELL, "Injector to producer indicator"}} );
    info.insert( {"WPWE6", {A::SUMMARY_WELL, "Producer to injector indicator"}} );
    info.insert( {"WPWE7", {A::SUMMARY_WELL, "Well shut indicator"}} );
    info.insert( {"WPWEM", {A::SUMMARY_WELL, "WELEVNT output mnemonic"}} );
    info.insert( {"WDRPR", {A::SUMMARY_WELL, "Well drilling priority"}} );
    info.insert( {"WBHWCn", {A::SUMMARY_WELL, "Derivative of well BHP with respect to parameter n"}} );
    info.insert( {"WGFWCn", {A::SUMMARY_WELL, "Derivative of well gas flow rate with respect to parameter n"}} );
    info.insert( {"WOFWCn", {A::SUMMARY_WELL, "Derivative of well oil flow rate with respect to parameter n"}} );
    info.insert( {"WWFWCn", {A::SUMMARY_WELL, "Derivative of water flow rate with respect to parameter n"}} );
    info.insert( {"WTPR", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    info.insert( {"WTPT", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    info.insert( {"WTPC", {A::SUMMARY_WELL, "Tracer Production Concentration"}} );
    info.insert( {"WTIR", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    info.insert( {"WTIT", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    info.insert( {"WTIC", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    info.insert( {"WTMR", {A::SUMMARY_WELL, "Traced mass Rate"}} );
    info.insert( {"WTMT", {A::SUMMARY_WELL, "Traced mass Total"}} );
    info.insert( {"WTQR", {A::SUMMARY_WELL, "Traced molar Rate"}} );
    info.insert( {"WTCM", {A::SUMMARY_WELL, "Tracer Carrier molar Rate"}} );
    info.insert( {"WTMF", {A::SUMMARY_WELL, "Traced molar fraction"}} );
    info.insert( {"WTVL", {A::SUMMARY_WELL, "Traced liquid volume rate"}} );
    info.insert( {"WTVV", {A::SUMMARY_WELL, "Traced vapor volume rate"}} );
    info.insert( {"WTTL", {A::SUMMARY_WELL, "Traced liquid volume total"}} );
    info.insert( {"WTTV", {A::SUMMARY_WELL, "Traced vapor volume total"}} );
    info.insert( {"WTML", {A::SUMMARY_WELL, "Traced mass liquid rate"}} );
    info.insert( {"WTMV", {A::SUMMARY_WELL, "Traced mass vapor rate"}} );
    info.insert( {"WTLM", {A::SUMMARY_WELL, "Traced mass liquid total"}} );
    info.insert( {"WTVM", {A::SUMMARY_WELL, "Traced mass vapor total"}} );
    info.insert( {"WAPI", {A::SUMMARY_WELL, "Oil API"}} );
    info.insert( {"WSPR", {A::SUMMARY_WELL, "Salt Production Rate"}} );
    info.insert( {"WSPT", {A::SUMMARY_WELL, "Salt Production Total"}} );
    info.insert( {"WSIR", {A::SUMMARY_WELL, "Salt Injection Rate"}} );
    info.insert( {"WSIT", {A::SUMMARY_WELL, "Salt Injection Total"}} );
    info.insert( {"WSPC", {A::SUMMARY_WELL, "Salt Production Concentration"}} );
    info.insert( {"WSIC", {A::SUMMARY_WELL, "Salt Injection Concentration"}} );
    info.insert( {"WTPCHEA", {A::SUMMARY_WELL, "Production Temperature"}} );
    info.insert( {"WTICHEA", {A::SUMMARY_WELL, "Injection Temperature"}} );
    info.insert( {"WTPRHEA", {A::SUMMARY_WELL, "Energy flows"}} );
    info.insert( {"WTPTHEA", {A::SUMMARY_WELL, "Energy Production Total"}} );
    info.insert( {"WTIRHEA", {A::SUMMARY_WELL, "Energy flows"}} );
    info.insert( {"WTITHEA", {A::SUMMARY_WELL, "Energy Injection Total"}} );
    info.insert( {"WTPR", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    info.insert( {"WTPT", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    info.insert( {"WTPC", {A::SUMMARY_WELL, "Tracer Production Concentration"}} );
    info.insert( {"WTIR", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    info.insert( {"WTIT", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    info.insert( {"WTIC", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    info.insert( {"WTIRF", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    info.insert( {"WTIRS", {A::SUMMARY_WELL, "Tracer Injection Rate"}} );
    info.insert( {"WTPRF", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    info.insert( {"WTPRS", {A::SUMMARY_WELL, "Tracer Production Rate"}} );
    info.insert( {"WTITF", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    info.insert( {"WTITS", {A::SUMMARY_WELL, "Tracer Injection Total"}} );
    info.insert( {"WTPTF", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    info.insert( {"WTPTS", {A::SUMMARY_WELL, "Tracer Production Total"}} );
    info.insert( {"WTICF", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    info.insert( {"WTICS", {A::SUMMARY_WELL, "Tracer Injection Concentration"}} );
    info.insert( {"WTPCF", {A::SUMMARY_WELL, "Tracer Production"}} );
    info.insert( {"WTPCS", {A::SUMMARY_WELL, "Tracer Production"}} );
    info.insert( {"WMPR", {A::SUMMARY_WELL, "Methane Production Rate"}} );
    info.insert( {"WMPT", {A::SUMMARY_WELL, "Methane Production Total"}} );
    info.insert( {"WMIR", {A::SUMMARY_WELL, "Methane Injection Rate"}} );
    info.insert( {"WMIT", {A::SUMMARY_WELL, "Methane Injection Total"}} );
    info.insert( {"WTPRFOA", {A::SUMMARY_WELL, "Production Rate"}} );
    info.insert( {"WTPTFOA", {A::SUMMARY_WELL, "Production Total"}} );
    info.insert( {"WTIRFOA", {A::SUMMARY_WELL, "Injection Rate"}} );
    info.insert( {"WTITFOA", {A::SUMMARY_WELL, "Injection Total"}} );
    info.insert( {"WGDC", {A::SUMMARY_WELL, "Gas Delivery Capacity"}} );
    info.insert( {"NGOPAS", {A::SUMMARY_WELL, "Number of iterations to converge DCQ in first pass"}} );
    info.insert( {"WGPRFP", {A::SUMMARY_WELL, "Well Gas Production Rate from end of First Pass"}} );
    info.insert( {"WTHPFP", {A::SUMMARY_WELL, "Well Tubing Head Pressure from end of First Pass"}} );
    info.insert( {"WBHPFP", {A::SUMMARY_WELL, "Well Bottom Hole Pressure from end of First Pass"}} );
    info.insert( {"WGLIR", {A::SUMMARY_WELL, "Gas Lift Injection Rate"}} );
    info.insert( {"WOGLR", {A::SUMMARY_WELL, "Well Oil Gas Lift Ratio"}} );
    info.insert( {"WGCV", {A::SUMMARY_WELL, "Gas Calorific Value"}} );
    info.insert( {"WGQ", {A::SUMMARY_WELL, "Gas molar Quality"}} );
    info.insert( {"WEPR", {A::SUMMARY_WELL, "Energy Production Rate"}} );
    info.insert( {"WEPT", {A::SUMMARY_WELL, "Energy Production Total"}} );
    info.insert( {"WEDC", {A::SUMMARY_WELL, "Energy Delivery Capacity"}} );
    info.insert( {"WCPR", {A::SUMMARY_WELL, "Polymer Production Rate"}} );
    info.insert( {"WCPC", {A::SUMMARY_WELL, "Polymer Production Concentration"}} );
    info.insert( {"WCPT", {A::SUMMARY_WELL, "Polymer Production Total"}} );
    info.insert( {"WCIR", {A::SUMMARY_WELL, "Polymer Injection Rate"}} );
    info.insert( {"WCIC", {A::SUMMARY_WELL, "Polymer Injection Concentration"}} );
    info.insert( {"WCIT", {A::SUMMARY_WELL, "Polymer Injection Total"}} );
    info.insert( {"WSPR", {A::SUMMARY_WELL, "Salt Production Rate"}} );
    info.insert( {"WSPT", {A::SUMMARY_WELL, "Salt Production Total"}} );
    info.insert( {"WSIR", {A::SUMMARY_WELL, "Salt Injection Rate"}} );
    info.insert( {"WSIT", {A::SUMMARY_WELL, "Salt Injection Total"}} );
    info.insert( {"WNPR", {A::SUMMARY_WELL, "Solvent Production Rate"}} );
    info.insert( {"WNPT", {A::SUMMARY_WELL, "Solvent Production Total"}} );
    info.insert( {"WNIR", {A::SUMMARY_WELL, "Solvent Injection Rate"}} );
    info.insert( {"WNIT", {A::SUMMARY_WELL, "Solvent Injection Total"}} );
    info.insert( {"WTPRSUR", {A::SUMMARY_WELL, "Production Rate"}} );
    info.insert( {"WTPTSUR", {A::SUMMARY_WELL, "Production Total"}} );
    info.insert( {"WTIRSUR", {A::SUMMARY_WELL, "Injection Rate"}} );
    info.insert( {"WTITSUR", {A::SUMMARY_WELL, "Injection Total"}} );
    info.insert( {"WTPRALK", {A::SUMMARY_WELL, "Production Rate"}} );
    info.insert( {"WTPTALK", {A::SUMMARY_WELL, "Production Total"}} );
    info.insert( {"WTIRALK", {A::SUMMARY_WELL, "Injection Rate"}} );
    info.insert( {"WTITALK", {A::SUMMARY_WELL, "Injection Total"}} );
    info.insert( {"WU", {A::SUMMARY_WELL, "User-defined well quantity"}} );

    // Future CONNECTION vectors
    info.insert( {"COFR", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    info.insert( {"COFRF", {A::SUMMARY_WELL_COMPLETION, "Free Oil Flow Rate"}} );
    info.insert( {"COFRS", {A::SUMMARY_WELL_COMPLETION, "Solution oil flow rate"}} );
    info.insert(
        {"COFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection oil flow rates upstream of, and including, this connection"}} );
    info.insert( {"COPR", {A::SUMMARY_WELL_COMPLETION, "Oil Production Rate"}} );
    info.insert( {"COPT", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    info.insert( {"COPTF", {A::SUMMARY_WELL_COMPLETION, "Free Oil Production Total"}} );
    info.insert( {"COPTS", {A::SUMMARY_WELL_COMPLETION, "Solution Oil Production Total"}} );
    info.insert( {"COIT", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    info.insert( {"COPP", {A::SUMMARY_WELL_COMPLETION, "Oil Potential Production rate"}} );
    info.insert( {"COPI", {A::SUMMARY_WELL_COMPLETION, "Oil Potential Injection rate"}} );
    info.insert( {"CWFR", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    info.insert( {"CWFRU",
                  {A::SUMMARY_WELL_COMPLETION,
                   "Sum of connection water flow rates upstream of, and including, this connection"}} );
    info.insert( {"CWPR", {A::SUMMARY_WELL_COMPLETION, "Water Production Rate"}} );
    info.insert( {"CWPT", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    info.insert( {"CWIR", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    info.insert( {"CWIT", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    info.insert( {"CWPP", {A::SUMMARY_WELL_COMPLETION, "Water Potential Production rate"}} );
    info.insert( {"CWPI", {A::SUMMARY_WELL_COMPLETION, "Water Potential Injection rate"}} );
    info.insert( {"CGFR", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"CGFRF", {A::SUMMARY_WELL_COMPLETION, "Free Gas Flow Rate"}} );
    info.insert( {"CGFRS", {A::SUMMARY_WELL_COMPLETION, "Solution Gas Flow Rate"}} );
    info.insert(
        {"CGFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection gas flow rates upstream of, and including, this connection"}} );
    info.insert( {"CGPR", {A::SUMMARY_WELL_COMPLETION, "Gas Production Rate "}} );
    info.insert( {"CGPT", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    info.insert( {"CGPTF", {A::SUMMARY_WELL_COMPLETION, "Free Gas Production Total"}} );
    info.insert( {"CGPTS", {A::SUMMARY_WELL_COMPLETION, "Solution Gas Production Total"}} );
    info.insert( {"CGIR", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    info.insert( {"CGIT", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    info.insert( {"CGPP", {A::SUMMARY_WELL_COMPLETION, "Gas Potential Production rate"}} );
    info.insert( {"CGPI", {A::SUMMARY_WELL_COMPLETION, "Gas Potential Injection rate"}} );
    info.insert( {"CGQ", {A::SUMMARY_WELL_COMPLETION, "Gas Quality"}} );
    info.insert( {"CLFR", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    info.insert( {"CLPT", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    info.insert( {"CVFR", {A::SUMMARY_WELL_COMPLETION, "Reservoir"}} );
    info.insert( {"CVPR", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Rate"}} );
    info.insert( {"CVPT", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    info.insert( {"CVIR", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Rate"}} );
    info.insert( {"CVIT", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    info.insert( {"CWCT", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    info.insert( {"CGOR", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    info.insert( {"COGR", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    info.insert( {"CWGR", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    info.insert( {"CGLR", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    info.insert( {"CPR", {A::SUMMARY_WELL_COMPLETION, "Connection Pressure"}} );
    info.insert( {"CPI", {A::SUMMARY_WELL_COMPLETION, "Productivity Index of well's preferred phase"}} );
    info.insert( {"CTFAC", {A::SUMMARY_WELL_COMPLETION, "Connection Transmissibility Factor"}} );
    info.insert( {"CDBF", {A::SUMMARY_WELL_COMPLETION, "Blocking factor for generalized pseudo-pressure method"}} );
    info.insert( {"CGPPTN", {A::SUMMARY_WELL_COMPLETION, "Generalized pseudo-pressure table update counter"}} );
    info.insert( {"CGPPTS", {A::SUMMARY_WELL_COMPLETION, "Generalized pseudo-pressure table update status"}} );
    info.insert( {"CDSM", {A::SUMMARY_WELL_COMPLETION, "Current mass of scale deposited"}} );
    info.insert( {"CDSML", {A::SUMMARY_WELL_COMPLETION, "Current mass of scale deposited per unit perforation length"}} );
    info.insert( {"CDSF", {A::SUMMARY_WELL_COMPLETION, "PI multiplicative factor due to scale damage"}} );
    info.insert( {"CAMF", {A::SUMMARY_WELL_COMPLETION, "Component aqueous mole fraction, from producing completions"}} );
    info.insert( {"CZMF", {A::SUMMARY_WELL_COMPLETION, "Total Mole Fraction"}} );
    info.insert( {"CKFR", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    info.insert( {"CKFT", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    info.insert( {"CDFAC", {A::SUMMARY_WELL_COMPLETION, "D-factor for flow dependent skin factor"}} );
    info.insert( {"CTFR", {A::SUMMARY_WELL_COMPLETION, "Tracer Flow Rate"}} );
    info.insert( {"CTPR", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    info.insert( {"CTPT", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    info.insert( {"CTPC", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Concentration"}} );
    info.insert( {"CTIR", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    info.insert( {"CTIT", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    info.insert( {"CTIC", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    info.insert( {"CAPI", {A::SUMMARY_WELL_COMPLETION, "Oil API"}} );
    info.insert( {"CSFR", {A::SUMMARY_WELL_COMPLETION, "Salt Flow Rate"}} );
    info.insert( {"CSPR", {A::SUMMARY_WELL_COMPLETION, "Salt Production Rate"}} );
    info.insert( {"CSPT", {A::SUMMARY_WELL_COMPLETION, "Salt Production Total"}} );
    info.insert( {"CSIR", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Rate"}} );
    info.insert( {"CSIT", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Total"}} );
    info.insert( {"CSPC", {A::SUMMARY_WELL_COMPLETION, "Salt Production Concentration"}} );
    info.insert( {"CSIC", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Concentration"}} );
    info.insert( {"CTFRANI", {A::SUMMARY_WELL_COMPLETION, "Anion Flow Rate"}} );
    info.insert( {"CTPTANI", {A::SUMMARY_WELL_COMPLETION, "Anion Production Total"}} );
    info.insert( {"CTITANI", {A::SUMMARY_WELL_COMPLETION, "Anion Injection Total"}} );
    info.insert( {"CTFRCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Flow Rate"}} );
    info.insert( {"CTPTCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Production Total"}} );
    info.insert( {"CTITCAT", {A::SUMMARY_WELL_COMPLETION, "Cation Injection Total"}} );
    info.insert( {"CTFR", {A::SUMMARY_WELL_COMPLETION, "Tracer Flow Rate"}} );
    info.insert( {"CTPR", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    info.insert( {"CTPT", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    info.insert( {"CTPC", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Concentration"}} );
    info.insert( {"CTIR", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    info.insert( {"CTIT", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    info.insert( {"CTIC", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    info.insert( {"CTIRF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    info.insert( {"CTIRS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Rate"}} );
    info.insert( {"CTPRF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    info.insert( {"CTPRS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Rate"}} );
    info.insert( {"CTITF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    info.insert( {"CTITS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Total"}} );
    info.insert( {"CTPTF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    info.insert( {"CTPTS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production Total"}} );
    info.insert( {"CTICF", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    info.insert( {"CTICS", {A::SUMMARY_WELL_COMPLETION, "Tracer Injection Concentration"}} );
    info.insert( {"CTPCF", {A::SUMMARY_WELL_COMPLETION, "Tracer Production"}} );
    info.insert( {"CTPCS", {A::SUMMARY_WELL_COMPLETION, "Tracer Production"}} );
    info.insert( {"CTFRFOA", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    info.insert( {"CTPTFOA", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    info.insert( {"CTITFOA", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    info.insert( {"CRREXCH", {A::SUMMARY_WELL_COMPLETION, "Exchange flux at current time"}} );
    info.insert( {"CRRPROT", {A::SUMMARY_WELL_COMPLETION, "Connection cumulative water production"}} );
    info.insert( {"CRRINJT", {A::SUMMARY_WELL_COMPLETION, "Connection cumulative water injection"}} );
    info.insert( {"CCFR", {A::SUMMARY_WELL_COMPLETION, "Polymer Flow Rate"}} );
    info.insert( {"CCPR", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Rate"}} );
    info.insert( {"CCPC", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Concentration"}} );
    info.insert( {"CCPT", {A::SUMMARY_WELL_COMPLETION, "Polymer Production Total"}} );
    info.insert( {"CCIR", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Rate"}} );
    info.insert( {"CCIC", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Concentration"}} );
    info.insert( {"CCIT", {A::SUMMARY_WELL_COMPLETION, "Polymer Injection Total"}} );
    info.insert( {"CSFR", {A::SUMMARY_WELL_COMPLETION, "Salt Flow Rate"}} );
    info.insert( {"CSPR", {A::SUMMARY_WELL_COMPLETION, "Salt Production Rate"}} );
    info.insert( {"CSPT", {A::SUMMARY_WELL_COMPLETION, "Salt Production Total"}} );
    info.insert( {"CSIR", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Rate"}} );
    info.insert( {"CSIT", {A::SUMMARY_WELL_COMPLETION, "Salt Injection Total"}} );
    info.insert( {"CNFR", {A::SUMMARY_WELL_COMPLETION, "Solvent Flow Rate"}} );
    info.insert( {"CNPT", {A::SUMMARY_WELL_COMPLETION, "Solvent Production Total"}} );
    info.insert( {"CNIT", {A::SUMMARY_WELL_COMPLETION, "Solvent Injection Total"}} );
    info.insert( {"CTFRSUR", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    info.insert( {"CTPTSUR", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    info.insert( {"CTITSUR", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    info.insert( {"CTFRALK", {A::SUMMARY_WELL_COMPLETION, "Flow Rate"}} );
    info.insert( {"CTPTALK", {A::SUMMARY_WELL_COMPLETION, "Production Total"}} );
    info.insert( {"CTITALK", {A::SUMMARY_WELL_COMPLETION, "Injection Total"}} );
    info.insert(
        {"COFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection oil flow rates upstream of, and including, this connection"}} );
    info.insert( {"CWFRU",
                  {A::SUMMARY_WELL_COMPLETION,
                   "Sum of connection water flow rates upstream of, and including, this connection"}} );
    info.insert(
        {"CGFRU",
         {A::SUMMARY_WELL_COMPLETION, "Sum of connection gas flow rates upstream of, and including, this connection"}} );
    info.insert( {"LCOFRU", {A::SUMMARY_WELL_COMPLETION, "As COFRU but for local grids"}} );
    info.insert( {"LCWFRU", {A::SUMMARY_WELL_COMPLETION, "As CWFRU but for local grids"}} );
    info.insert( {"LCGFRU", {A::SUMMARY_WELL_COMPLETION, "As CGFRU but for local grids"}} );
    info.insert( {"CU", {A::SUMMARY_WELL_COMPLETION, "User-defined connection quantity"}} );

    info.insert( {"COFRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    info.insert( {"WOFRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    info.insert( {"COPRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    info.insert( {"WOPRL", {A::SUMMARY_WELL_COMPLETION, "Oil Flow Rate"}} );
    info.insert( {"COPTL", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    info.insert( {"WOPTL", {A::SUMMARY_WELL_COMPLETION, "Oil Production Total"}} );
    info.insert( {"COITL", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    info.insert( {"WOITL", {A::SUMMARY_WELL_COMPLETION, "Oil Injection Total"}} );
    info.insert( {"CWFRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    info.insert( {"WWFRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    info.insert( {"CWPRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    info.insert( {"WWPRL", {A::SUMMARY_WELL_COMPLETION, "Water Flow Rate"}} );
    info.insert( {"CWPTL", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    info.insert( {"WWPTL", {A::SUMMARY_WELL_COMPLETION, "Water Production Total"}} );
    info.insert( {"CWIRL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    info.insert( {"WWIRL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Rate"}} );
    info.insert( {"CWITL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    info.insert( {"WWITL", {A::SUMMARY_WELL_COMPLETION, "Water Injection Total"}} );
    info.insert( {"CGFRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"WGFRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"CGPRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"WGPRL", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"CGPTL", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    info.insert( {"WGPTL", {A::SUMMARY_WELL_COMPLETION, "Gas Production Total"}} );
    info.insert( {"CGIRL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    info.insert( {"WGIRL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Rate"}} );
    info.insert( {"CGITL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    info.insert( {"WGITL", {A::SUMMARY_WELL_COMPLETION, "Gas Injection Total"}} );
    info.insert( {"CLFRL", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    info.insert( {"WLFRL", {A::SUMMARY_WELL_COMPLETION, "Liquid Flow Rate"}} );
    info.insert( {"CLPTL", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    info.insert( {"WLPTL", {A::SUMMARY_WELL_COMPLETION, "Liquid Production Total"}} );
    info.insert( {"CVFRL", {A::SUMMARY_WELL_COMPLETION, "Reservoir"}} );
    info.insert( {"WVFRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Flow Rate"}} );
    info.insert( {"CVPRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Flow Rate"}} );
    info.insert( {"WVPRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Flow Rate"}} );
    info.insert( {"CVIRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Flow Rate"}} );
    info.insert( {"WVIRL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Flow Rate"}} );
    info.insert( {"CVPTL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    info.insert( {"WVPTL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Production Total"}} );
    info.insert( {"CVITL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    info.insert( {"WVITL", {A::SUMMARY_WELL_COMPLETION, "Res Volume Injection Total"}} );
    info.insert( {"CWCTL", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    info.insert( {"WWCTL", {A::SUMMARY_WELL_COMPLETION, "Water Cut"}} );
    info.insert( {"CGORL", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    info.insert( {"WGORL", {A::SUMMARY_WELL_COMPLETION, "Gas-Oil Ratio"}} );
    info.insert( {"COGRL", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    info.insert( {"WOGRL", {A::SUMMARY_WELL_COMPLETION, "Oil-Gas Ratio"}} );
    info.insert( {"CWGRL", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    info.insert( {"WWGRL", {A::SUMMARY_WELL_COMPLETION, "Water-Gas Ratio"}} );
    info.insert( {"CGLRL", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    info.insert( {"WGLRL", {A::SUMMARY_WELL_COMPLETION, "Gas-Liquid Ratio"}} );
    info.insert( {"CPRL", {A::SUMMARY_WELL_COMPLETION, "Average Connection Pressure in completion"}} );
    info.insert( {"CKFRL", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );
    info.insert( {"CKFTL", {A::SUMMARY_WELL_COMPLETION, "Hydrocarbon Component"}} );

    info.insert( {"RPR", {A::SUMMARY_REGION, "Pressure average value"}} );
    info.insert( {"RPRH", {A::SUMMARY_REGION, "Pressure average value"}} );
    info.insert( {"RPRP", {A::SUMMARY_REGION, "Pressure average value"}} );
    info.insert( {"RPRGZ", {A::SUMMARY_REGION, "P/Z"}} );
    info.insert( {"RRS", {A::SUMMARY_REGION, "Gas-oil ratio"}} );
    info.insert( {"RRV", {A::SUMMARY_REGION, "Oil-gas ratio"}} );
    info.insert( {"RPPC", {A::SUMMARY_REGION, "Initial Contact Corrected Potential"}} );
    info.insert( {"RRPV", {A::SUMMARY_REGION, "Pore Volume at Reservoir conditions"}} );
    info.insert( {"ROPV", {A::SUMMARY_REGION, "Pore Volume containing Oil"}} );
    info.insert( {"RWPV", {A::SUMMARY_REGION, "Pore Volume containing Water"}} );
    info.insert( {"RGPV", {A::SUMMARY_REGION, "Pore Volume containing Gas"}} );
    info.insert( {"RHPV", {A::SUMMARY_REGION, "Pore Volume containing Hydrocarbon"}} );
    info.insert( {"RRTM", {A::SUMMARY_REGION, "Transmissibility Multiplier associated with rock compaction"}} );
    info.insert( {"ROE", {A::SUMMARY_REGION, "(OIP(initial) - OIP(now)) / OIP(initial)"}} );
    info.insert( {"ROEW", {A::SUMMARY_REGION, "Oil Production from Wells / OIP(initial)"}} );
    info.insert( {"ROEIW", {A::SUMMARY_REGION, "(OIP(initial) - OIP(now)) / Initial Mobile Oil with respect to Water"}} );
    info.insert( {"ROEWW", {A::SUMMARY_REGION, "Oil Production from Wells / Initial Mobile Oil with respect to Water"}} );
    info.insert( {"ROEIG", {A::SUMMARY_REGION, "(OIP(initial) - OIP(now)) / Initial Mobile Oil with respect to Gas"}} );
    info.insert( {"ROEWG", {A::SUMMARY_REGION, "Oil Production from Wells / Initial Mobile Oil with respect to Gas"}} );
    info.insert( {"RORMR", {A::SUMMARY_REGION, "Total stock tank oil produced by rock compaction"}} );
    info.insert( {"RORMW", {A::SUMMARY_REGION, "Total stock tank oil produced by water influx"}} );
    info.insert( {"RORMG", {A::SUMMARY_REGION, "Total stock tank oil produced by gas influx"}} );
    info.insert( {"RORME", {A::SUMMARY_REGION, "Total stock tank oil produced by oil expansion"}} );
    info.insert( {"RORMS", {A::SUMMARY_REGION, "Total stock tank oil produced by solution gas"}} );
    info.insert( {"RORMF", {A::SUMMARY_REGION, "Total stock tank oil produced by free gas influx"}} );
    info.insert( {"RORMX", {A::SUMMARY_REGION, "Total stock tank oil produced by 'traced' water influx"}} );
    info.insert( {"RORMY", {A::SUMMARY_REGION, "Total stock tank oil produced by other water influx"}} );
    info.insert( {"RORFR", {A::SUMMARY_REGION, "Fraction of total oil produced by rock compaction"}} );
    info.insert( {"RORFW", {A::SUMMARY_REGION, "Fraction of total oil produced by water influx"}} );
    info.insert( {"RORFG", {A::SUMMARY_REGION, "Fraction of total oil produced by gas influx"}} );
    info.insert( {"RORFE", {A::SUMMARY_REGION, "Fraction of total oil produced by oil expansion"}} );
    info.insert( {"RORFS", {A::SUMMARY_REGION, "Fraction of total oil produced by solution gas"}} );
    info.insert( {"RORFF", {A::SUMMARY_REGION, "Fraction of total oil produced by free gas influx"}} );
    info.insert( {"RORFX", {A::SUMMARY_REGION, "Fraction of total oil produced by 'traced' water influx"}} );
    info.insert( {"RORFY", {A::SUMMARY_REGION, "Fraction of total oil produced by other water influx"}} );
    info.insert( {"RTIPT", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RTIPF", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RTIPS", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RAPI", {A::SUMMARY_REGION, "Oil API"}} );
    info.insert( {"RSIP", {A::SUMMARY_REGION, "Salt In Place"}} );
    info.insert( {"RTIPTHEA", {A::SUMMARY_REGION, "Difference in Energy in place between current and initial time"}} );
    info.insert( {"RTIPT", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RTIPF", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RTIPS", {A::SUMMARY_REGION, "Tracer In Place"}} );
    info.insert( {"RTIP#", {A::SUMMARY_REGION, "Tracer In Place in phase # (1,2,3,...)"}} );
    info.insert( {"RTADS", {A::SUMMARY_REGION, "Tracer Adsorption total"}} );
    info.insert( {"RTDCY", {A::SUMMARY_REGION, "Decayed tracer"}} );
    info.insert( {"RCGC", {A::SUMMARY_REGION, "Bulk Coal Gas Concentration"}} );
    info.insert( {"RCSC", {A::SUMMARY_REGION, "Bulk Coal Solvent Concentration"}} );
    info.insert( {"RTIPTFOA", {A::SUMMARY_REGION, "In Solution"}} );
    info.insert( {"RTADSFOA", {A::SUMMARY_REGION, "Adsorption total"}} );
    info.insert( {"RTDCYFOA", {A::SUMMARY_REGION, "Decayed tracer"}} );
    info.insert( {"RTMOBFOA", {A::SUMMARY_REGION, "Gas mobility factor"}} );
    info.insert( {"RCIP", {A::SUMMARY_REGION, "Polymer In Solution"}} );
    info.insert( {"RCAD", {A::SUMMARY_REGION, "Polymer Adsorption total"}} );
    info.insert( {"RSIP", {A::SUMMARY_REGION, "Salt In Place"}} );
    info.insert( {"RNIP", {A::SUMMARY_REGION, "Solvent In Place"}} );
    info.insert( {"RTIPTSUR", {A::SUMMARY_REGION, "In Solution"}} );
    info.insert( {"RTADSUR", {A::SUMMARY_REGION, "Adsorption total"}} );
    info.insert( {"RU", {A::SUMMARY_REGION, "User-defined region quantity"}} );

    info.insert( {"ROFR", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    info.insert( {"ROFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    info.insert( {"ROFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow rate"}} );
    info.insert( {"ROFT", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    info.insert( {"ROFT+", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    info.insert( {"ROFT-", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    info.insert( {"ROFTL", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    info.insert( {"ROFTG", {A::SUMMARY_REGION_2_REGION, "Inter-region oil flow total"}} );
    info.insert( {"RGFR", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    info.insert( {"RGFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    info.insert( {"RGFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow rate"}} );
    info.insert( {"RGFT", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total)"}} );
    info.insert( {"RGFT+", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    info.insert( {"RGFT-", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    info.insert( {"RGFTL", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    info.insert( {"RGFTG", {A::SUMMARY_REGION_2_REGION, "Inter-region gas flow total"}} );
    info.insert( {"RWFR", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    info.insert( {"RWFR+", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    info.insert( {"RWFR-", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow rate"}} );
    info.insert( {"RWFT", {A::SUMMARY_REGION_2_REGION, "Inter-region water flow total"}} );
    info.insert( {"RTFTF", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    info.insert( {"RTFTS", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    info.insert( {"RTFTT", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow Total"}} );
    info.insert( {"RSFT", {A::SUMMARY_REGION_2_REGION, "Salt inter-region Flow Total"}} );
    info.insert( {"RTFTT", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    info.insert( {"RTFTF", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    info.insert( {"RTFTS", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow"}} );
    info.insert( {"RTFT#", {A::SUMMARY_REGION_2_REGION, "Tracer inter-region Flow in phase # (1,2,3,...)"}} );
    info.insert( {"RTFTTFOA", {A::SUMMARY_REGION_2_REGION, "Inter-region Flow Total"}} );
    info.insert( {"RCFT", {A::SUMMARY_REGION_2_REGION, "Polymer inter-region Flow Total"}} );
    info.insert( {"RSFT", {A::SUMMARY_REGION_2_REGION, "Salt inter-region Flow Total"}} );
    info.insert( {"RNFT", {A::SUMMARY_REGION_2_REGION, "Solvent inter-region Flow"}} );
    info.insert( {"RTFTTSUR", {A::SUMMARY_REGION_2_REGION, "Inter-region Flow Total"}} );

    info.insert( {"BPR", {A::SUMMARY_BLOCK, "Oil phase Pressure"}} );
    info.insert( {"BPRESSUR", {A::SUMMARY_BLOCK, "Oil phase Pressure"}} );
    info.insert( {"BWPR", {A::SUMMARY_BLOCK, "Water phase Pressure"}} );
    info.insert( {"BGPR", {A::SUMMARY_BLOCK, "Gas phase Pressure"}} );
    info.insert( {"BRS", {A::SUMMARY_BLOCK, "Gas-oil ratio"}} );
    info.insert( {"BRV", {A::SUMMARY_BLOCK, "Oil-gas ratio"}} );
    info.insert( {"BPBUB", {A::SUMMARY_BLOCK, "Bubble point pressure"}} );
    info.insert( {"BPDEW", {A::SUMMARY_BLOCK, "Dew point pressure"}} );
    info.insert( {"BRSSAT", {A::SUMMARY_BLOCK, "Saturated gas-oil ratio"}} );
    info.insert( {"BRVSAT", {A::SUMMARY_BLOCK, "Saturated oil-gas ratio"}} );
    info.insert( {"BSTATE", {A::SUMMARY_BLOCK, "Gas-oil state indicator"}} );
    info.insert( {"BPPC", {A::SUMMARY_BLOCK, "Initial Contact Corrected Potential"}} );
    info.insert( {"BOKR", {A::SUMMARY_BLOCK, "Oil relative permeability"}} );
    info.insert( {"BWKR", {A::SUMMARY_BLOCK, "Water relative permeability"}} );
    info.insert( {"BGKR", {A::SUMMARY_BLOCK, "Gas relative permeability"}} );
    info.insert( {"BKRO", {A::SUMMARY_BLOCK, "Oil relative permeability"}} );
    info.insert( {"BKROG", {A::SUMMARY_BLOCK, "Two-phase oil relative permeability to gas"}} );
    info.insert( {"BKROW", {A::SUMMARY_BLOCK, "Two-phase oil relative permeability to water"}} );
    info.insert( {"BKRG", {A::SUMMARY_BLOCK, "Gas relative permeability"}} );
    info.insert( {"BKRGO", {A::SUMMARY_BLOCK, "Two-phase gas relative permeability to oil "}} );
    info.insert( {"BKRGW", {A::SUMMARY_BLOCK, "Two-phase gas relative permeability to water"}} );
    info.insert( {"BKRW", {A::SUMMARY_BLOCK, "Water relative permeability"}} );
    info.insert( {"BKRWG", {A::SUMMARY_BLOCK, "Two-phase water relative permeability to gas"}} );
    info.insert( {"BKRWO", {A::SUMMARY_BLOCK, "Two-phase water relative permeability to oil"}} );
    info.insert( {"BRK", {A::SUMMARY_BLOCK, "Water relative permeability reduction factor due to polymer"}} );
    info.insert( {"BEWKR", {A::SUMMARY_BLOCK, "Water effective relative permeability due to polymer"}} );
    info.insert( {"BWPC", {A::SUMMARY_BLOCK, "Water-Oil capillary pressure"}} );
    info.insert( {"BGPC", {A::SUMMARY_BLOCK, "Gas-Oil capillary pressure"}} );
    info.insert( {"BPCO", {A::SUMMARY_BLOCK, "Oil Capillary Pressures"}} );
    info.insert( {"BPCG", {A::SUMMARY_BLOCK, "Gas Capillary Pressures"}} );
    info.insert( {"BPCW", {A::SUMMARY_BLOCK, "Water Capillary Pressures"}} );
    info.insert( {"BGTRP", {A::SUMMARY_BLOCK, "Trapped gas saturation"}} );
    info.insert( {"BGTPD", {A::SUMMARY_BLOCK, "Dynamic trapped gas saturation"}} );
    info.insert(
        {"BGSHY",
         {A::SUMMARY_BLOCK, "Departure saturation from drainage to imbibition for gas capillary pressure hysteresis"}} );
    info.insert( {"BGSTRP", {A::SUMMARY_BLOCK, "Trapped gas critical saturation for gas capillary pressure hysteresis"}} );
    info.insert( {"BWSHY",
                  {A::SUMMARY_BLOCK,
                   "Departure saturation from drainage to imbibition for water capillary pressure hysteresis"}} );
    info.insert( {"BWSMA", {A::SUMMARY_BLOCK, "Maximum wetting saturation for water capillary pressure hysteresis"}} );
    info.insert( {"BMLSC", {A::SUMMARY_BLOCK, "Hydrocarbon molar density"}} );
    info.insert( {"BMLST", {A::SUMMARY_BLOCK, "Total hydrocarbon molar density"}} );
    info.insert( {"BMWAT", {A::SUMMARY_BLOCK, "Water molar density"}} );
    info.insert( {"BROMLS", {A::SUMMARY_BLOCK, "Residual oil moles/ reservoir volume"}} );
    info.insert( {"BJV", {A::SUMMARY_BLOCK, "In"}} );
    info.insert( {"BVMF", {A::SUMMARY_BLOCK, "Vapor mole fraction"}} );
    info.insert( {"BPSAT", {A::SUMMARY_BLOCK, "Saturation Pressures"}} );
    info.insert( {"BAMF", {A::SUMMARY_BLOCK, "Component aqueous mole fraction"}} );
    info.insert( {"BXMF", {A::SUMMARY_BLOCK, "Liquid hydrocarbon component mole fraction"}} );
    info.insert( {"BYMF", {A::SUMMARY_BLOCK, "Vapor hydrocarbon component mole fraction / vapor steam"}} );
    info.insert( {"BSMF", {A::SUMMARY_BLOCK, "CO2STORE with SOLID option only Solid hydrocarbon component mole fraction"}} );
    info.insert( {"BSTEN", {A::SUMMARY_BLOCK, "Surface Tension"}} );
    info.insert( {"BFMISC", {A::SUMMARY_BLOCK, "Miscibility Factor"}} );
    info.insert( {"BREAC", {A::SUMMARY_BLOCK, "Reaction rate. The reaction number is given as a component index"}} );
    info.insert( {"BHD", {A::SUMMARY_BLOCK, "Hydraulic head"}} );
    info.insert( {"BHDF", {A::SUMMARY_BLOCK, "Hydraulic head at fresh water conditions"}} );
    info.insert( {"BPR_X", {A::SUMMARY_BLOCK, "Pressure interpolated at a defined coordinate"}} );
    info.insert( {"BHD_X", {A::SUMMARY_BLOCK, "Hydraulic head interpolated at a defined coordinate"}} );
    info.insert( {"BHDF_X",
                  {A::SUMMARY_BLOCK, "Hydraulic head at fresh water conditions interpolated at a defined coordinate"}} );
    info.insert( {"BSCN_X", {A::SUMMARY_BLOCK, "Brine concentration interpolated at a defined coordinate"}} );
    info.insert( {"BCTRA_X", {A::SUMMARY_BLOCK, "Tracer concentration interpolated at a defined coordinate"}} );
    info.insert( {"LBPR_X", {A::SUMMARY_BLOCK, "Pressure interpolated at a defined coordinate within a local grid"}} );
    info.insert( {"LBHD_X", {A::SUMMARY_BLOCK, "Hydraulic head interpolated at a defined coordinate within a local grid"}} );
    info.insert(
        {"LBHDF_X",
         {A::SUMMARY_BLOCK,
          "Hydraulic head at freshwater conditions interpolated at a defined coordinate within a local grid"}} );
    info.insert( {"LBSCN_X",
                  {A::SUMMARY_BLOCK, "Brine concentration interpolated at a defined coordinate within a local grid"}} );
    info.insert( {"LBCTRA_X",
                  {A::SUMMARY_BLOCK, "Tracer concentration interpolated at a defined coordinate within a local grid"}} );
    info.insert( {"BOKRX", {A::SUMMARY_BLOCK, "Oil relative permeability in the X direction"}} );
    info.insert( {"BOKRX", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -X direction"}} );
    info.insert( {"BOKRY", {A::SUMMARY_BLOCK, "Oil relative permeability in the Y direction"}} );
    info.insert( {"BOKRY", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -Y direction"}} );
    info.insert( {"BOKRZ", {A::SUMMARY_BLOCK, "Oil relative permeability in the Z direction"}} );
    info.insert( {"BOKRZ", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -Z direction"}} );
    info.insert( {"BWKRX", {A::SUMMARY_BLOCK, "Water relative permeability in the X direction"}} );
    info.insert( {"BWKRX", {A::SUMMARY_BLOCK, "- Water relative permeability in the -X direction"}} );
    info.insert( {"BWKRY", {A::SUMMARY_BLOCK, "Water relative permeability in the Y direction"}} );
    info.insert( {"BWKRY", {A::SUMMARY_BLOCK, "- Water relative permeability in the -Y direction"}} );
    info.insert( {"BWKRZ", {A::SUMMARY_BLOCK, "Water relative permeability in the Z direction"}} );
    info.insert( {"BWKRZ", {A::SUMMARY_BLOCK, "- Water relative permeability in the -Z direction"}} );
    info.insert( {"BGKRX", {A::SUMMARY_BLOCK, "Gas relative permeability in the X direction"}} );
    info.insert( {"BGKRX", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -X direction"}} );
    info.insert( {"BGKRY", {A::SUMMARY_BLOCK, "Gas relative permeability in the Y direction"}} );
    info.insert( {"BGKRY", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -Y direction"}} );
    info.insert( {"BGKRZ", {A::SUMMARY_BLOCK, "Gas relative permeability in the Z direction"}} );
    info.insert( {"BGKRZ", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -Z direction"}} );
    info.insert( {"BOKRI", {A::SUMMARY_BLOCK, "Oil relative permeability in the I direction"}} );
    info.insert( {"BOKRI", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -I direction"}} );
    info.insert( {"BOKRJ", {A::SUMMARY_BLOCK, "Oil relative permeability in the J direction"}} );
    info.insert( {"BOKRJ", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -J direction"}} );
    info.insert( {"BOKRK", {A::SUMMARY_BLOCK, "Oil relative permeability in the K direction"}} );
    info.insert( {"BOKRK", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -K direction"}} );
    info.insert( {"BWKRI", {A::SUMMARY_BLOCK, "Water relative permeability in the I direction"}} );
    info.insert( {"BWKRI", {A::SUMMARY_BLOCK, "- Water relative permeability in the -I direction"}} );
    info.insert( {"BWKRJ", {A::SUMMARY_BLOCK, "Water relative permeability in the J direction"}} );
    info.insert( {"BWKRJ", {A::SUMMARY_BLOCK, "- Water relative permeability in the -J direction"}} );
    info.insert( {"BWKRK", {A::SUMMARY_BLOCK, "Water relative permeability in the K direction"}} );
    info.insert( {"BWKRK", {A::SUMMARY_BLOCK, "- Water relative permeability in the -K direction"}} );
    info.insert( {"BGKRI", {A::SUMMARY_BLOCK, "Gas relative permeability in the I direction"}} );
    info.insert( {"BGKRI", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -I direction"}} );
    info.insert( {"BGKRJ", {A::SUMMARY_BLOCK, "Gas relative permeability in the J direction"}} );
    info.insert( {"BGKRJ", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -J direction"}} );
    info.insert( {"BGKRK", {A::SUMMARY_BLOCK, "Gas relative permeability in the K direction"}} );
    info.insert( {"BGKRK", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -K direction"}} );
    info.insert( {"BOKRR", {A::SUMMARY_BLOCK, "Oil relative permeability in the R"}} );
    info.insert( {"BOKRR", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -R"}} );
    info.insert( {"BOKRT", {A::SUMMARY_BLOCK, "Oil relative permeability in the T"}} );
    info.insert( {"BOKRT", {A::SUMMARY_BLOCK, "- Oil relative permeability in the -T"}} );
    info.insert( {"BWKRR", {A::SUMMARY_BLOCK, "Water relative permeability in the R"}} );
    info.insert( {"BWKRR", {A::SUMMARY_BLOCK, "- Water relative permeability in the -R"}} );
    info.insert( {"BWKRT", {A::SUMMARY_BLOCK, "Water relative permeability in the T"}} );
    info.insert( {"BWKRT", {A::SUMMARY_BLOCK, "- Water relative permeability in the -T"}} );
    info.insert( {"BGKRR", {A::SUMMARY_BLOCK, "Gas relative permeability in the R"}} );
    info.insert( {"BGKRR", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -R"}} );
    info.insert( {"BGKRT", {A::SUMMARY_BLOCK, "Gas relative permeability in the T"}} );
    info.insert( {"BGKRT", {A::SUMMARY_BLOCK, "- Gas relative permeability in the -T"}} );
    info.insert( {"BRPV", {A::SUMMARY_BLOCK, "Pore Volume at Reservoir conditions"}} );
    info.insert( {"BPORV", {A::SUMMARY_BLOCK, "Cell Pore Volumes at Reference conditions"}} );
    info.insert( {"BOPV", {A::SUMMARY_BLOCK, "Pore Volume containing Oil"}} );
    info.insert( {"BWPV", {A::SUMMARY_BLOCK, "Pore Volume containing Water"}} );
    info.insert( {"BGPV", {A::SUMMARY_BLOCK, "Pore Volume containing Gas"}} );
    info.insert( {"BHPV", {A::SUMMARY_BLOCK, "Pore Volume containing Hydrocarbon"}} );
    info.insert( {"BRTM", {A::SUMMARY_BLOCK, "Transmissibility Multiplier associated with rock compaction"}} );
    info.insert( {"BPERMMOD", {A::SUMMARY_BLOCK, "Transmissibility Multiplier associated with rock compaction"}} );
    info.insert( {"BPERMMDX",
                  {A::SUMMARY_BLOCK,
                   "Directional Transmissibility Multipliers in the X direction, associated with rock compaction"}} );
    info.insert( {"BPERMMDY",
                  {A::SUMMARY_BLOCK,
                   "Directional Transmissibility Multipliers in the Y direction, associated with rock compaction"}} );
    info.insert( {"BPERMMDZ",
                  {A::SUMMARY_BLOCK,
                   "Directional Transmissibility Multipliers in the Z direction, associated with rock compaction"}} );
    info.insert( {"BPORVMOD", {A::SUMMARY_BLOCK, "Pore Volume Multiplier associated with rock compaction"}} );
    info.insert( {"BSIGMMOD", {A::SUMMARY_BLOCK, "Dual Porosity Sigma Multiplier associated with rock compaction"}} );
    info.insert( {"BTCNF", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    info.insert( {"BTCNS", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    info.insert( {"BTCN", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    info.insert( {"BTIPT", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BTIPF", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BTIPS", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BAPI", {A::SUMMARY_BLOCK, "Oil API"}} );
    info.insert( {"BSCN", {A::SUMMARY_BLOCK, "Salt Cell Concentration"}} );
    info.insert( {"BSIP", {A::SUMMARY_BLOCK, "Salt In Place"}} );
    info.insert( {"BEWV_SAL", {A::SUMMARY_BLOCK, "Effective water viscosity due to salt concentration"}} );
    info.insert( {"BTCNFANI", {A::SUMMARY_BLOCK, "Anion Flowing Concentration"}} );
    info.insert( {"BTCNFCAT", {A::SUMMARY_BLOCK, "Cation Flowing Concentration"}} );
    info.insert( {"BTRADCAT", {A::SUMMARY_BLOCK, "Cation Rock Associated Concentration"}} );
    info.insert( {"BTSADCAT", {A::SUMMARY_BLOCK, "Cation Surfactant Associated Concentration"}} );
    info.insert( {"BESALSUR", {A::SUMMARY_BLOCK, "Effective Salinity with respect to Surfactant"}} );
    info.insert( {"BESALPLY", {A::SUMMARY_BLOCK, "Effective Salinity with respect to Polymer"}} );
    info.insert( {"BTCNFHEA", {A::SUMMARY_BLOCK, "Block Temperature"}} );
    info.insert( {"BTIPTHEA", {A::SUMMARY_BLOCK, "Difference in Energy in place between current and initial time"}} );
    info.insert( {"BTCNF", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    info.insert( {"BTCNS", {A::SUMMARY_BLOCK, "Tracer Concentration"}} );
    info.insert( {"BTCN#", {A::SUMMARY_BLOCK, "Tracer concentration in phase # (1,2,3,...)"}} );
    info.insert( {"BTIPT", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BTIPF", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BTIPS", {A::SUMMARY_BLOCK, "Tracer In Place"}} );
    info.insert( {"BTIP#", {A::SUMMARY_BLOCK, "Tracer In Place in phase # (1,2,3,...)"}} );
    info.insert( {"BTADS", {A::SUMMARY_BLOCK, "Tracer Adsorption"}} );
    info.insert( {"BTDCY", {A::SUMMARY_BLOCK, "Decayed tracer"}} );
    info.insert( {"BCGC", {A::SUMMARY_BLOCK, "Bulk Coal Gas Concentration"}} );
    info.insert( {"BCSC", {A::SUMMARY_BLOCK, "Bulk Coal Solvent Concentration"}} );
    info.insert( {"BTCNFFOA", {A::SUMMARY_BLOCK, "Concentration"}} );
    info.insert( {"BFOAM", {A::SUMMARY_BLOCK, "Surfactant concentration"}} );
    info.insert( {"BTCNMFOA", {A::SUMMARY_BLOCK, "Capillary number"}} );
    info.insert( {"BFOAMCNM", {A::SUMMARY_BLOCK, "Capillary number"}} );
    info.insert( {"BTIPTFOA", {A::SUMMARY_BLOCK, "In Solution"}} );
    info.insert( {"BTADSFOA", {A::SUMMARY_BLOCK, "Adsorption"}} );
    info.insert( {"BTDCYFOA", {A::SUMMARY_BLOCK, "Decayed tracer"}} );
    info.insert( {"BTMOBFOA", {A::SUMMARY_BLOCK, "Gas mobility factor"}} );
    info.insert( {"BFOAMMOB", {A::SUMMARY_BLOCK, "Gas mobility factor"}} );
    info.insert( {"BTHLFFOA", {A::SUMMARY_BLOCK, "Decay Half life"}} );
    info.insert( {"BGI", {A::SUMMARY_BLOCK, "Block Gi value"}} );
    info.insert( {"BCCN", {A::SUMMARY_BLOCK, "Polymer Concentration"}} );
    info.insert( {"BCIP", {A::SUMMARY_BLOCK, "Polymer In Solution"}} );
    info.insert( {"BEPVIS", {A::SUMMARY_BLOCK, "Effective polymer solution viscosity"}} );
    info.insert( {"BVPOLY", {A::SUMMARY_BLOCK, "Effective polymer solution viscosity"}} );
    info.insert( {"BEMVIS", {A::SUMMARY_BLOCK, "Effective mixture"}} );
    info.insert( {"BEWV_POL", {A::SUMMARY_BLOCK, "Effective water viscosity"}} );
    info.insert( {"BCAD", {A::SUMMARY_BLOCK, "Polymer Adsorption concentration"}} );
    info.insert( {"BCDCS", {A::SUMMARY_BLOCK, "Polymer thermal degradation - total mass degraded in previous timestep"}} );
    info.insert( {"BCDCR", {A::SUMMARY_BLOCK, "Polymer thermal degradation - total degradation rate"}} );
    info.insert( {"BCDCP", {A::SUMMARY_BLOCK, "Polymer thermal degradation solution degradation rate"}} );
    info.insert( {"BCDCA", {A::SUMMARY_BLOCK, "Polymer thermal degradation adsorbed degradation rate"}} );
    info.insert(
        {"BCABnnn", {A::SUMMARY_BLOCK, "Adsorbed polymer by highest temperature band at which RRF was calculated"}} );
    info.insert( {"BSCN", {A::SUMMARY_BLOCK, "Salt Cell Concentration"}} );
    info.insert( {"BSIP", {A::SUMMARY_BLOCK, "Salt In Place"}} );
    info.insert( {"BFLOW0I",
                  {A::SUMMARY_BLOCK,
                   "Inter-block water flow rate in the positive I direction multiplied by the "
                   "corresponding shear multiplier"}} );
    info.insert( {"BFLOW0J",
                  {A::SUMMARY_BLOCK,
                   "Inter-block water flow rate in the positive J direction multiplied by the "
                   "corresponding shear multiplier"}} );
    info.insert( {"BFLOW0K",
                  {A::SUMMARY_BLOCK,
                   "Inter-block water flow rate in the positive K direction multiplied by the "
                   "corresponding shear multiplier"}} );
    info.insert( {"BVELW0I",
                  {A::SUMMARY_BLOCK,
                   "Water velocity in the positive I direction multiplied by the corresponding shear multiplier"}} );
    info.insert( {"BVELW0J",
                  {A::SUMMARY_BLOCK,
                   "Water velocity in the positive J direction multiplied by the corresponding shear multiplier"}} );
    info.insert( {"BVELW0K",
                  {A::SUMMARY_BLOCK,
                   "Water velocity in the positive K direction multiplied by the corresponding shear multiplier"}} );
    info.insert(
        {"BPSHLZI", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive I direction"}} );
    info.insert(
        {"BPSHLZJ", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive J direction"}} );
    info.insert(
        {"BPSHLZK", {A::SUMMARY_BLOCK, "Viscosity multiplier due to sheared water flow in the positive K direction"}} );
    info.insert( {"BSRTW0I", {A::SUMMARY_BLOCK, "Water shear rate in the positive I direction prior to shear effects"}} );
    info.insert( {"BSRTW0J", {A::SUMMARY_BLOCK, "Water shear rate in the positive J direction prior to shear effects"}} );
    info.insert( {"BSRTW0K", {A::SUMMARY_BLOCK, "Water shear rate in the positive K direction prior to shear effects"}} );
    info.insert( {"BSRTWI", {A::SUMMARY_BLOCK, "Water shear rate in the positive I direction following shear effects"}} );
    info.insert( {"BSRTWJ", {A::SUMMARY_BLOCK, "Water shear rate in the positive J direction following shear effects"}} );
    info.insert( {"BSRTWK", {A::SUMMARY_BLOCK, "Water shear rate in the positive K direction following shear effects"}} );
    info.insert( {"BSHWVISI",
                  {A::SUMMARY_BLOCK,
                   "Shear viscosity of the water/polymer solution due to shear thinning/thickening in "
                   "the positive I direction"}} );
    info.insert( {"BSHWVISJ",
                  {A::SUMMARY_BLOCK,
                   "Shear viscosity of the water/polymer solution due to shear thinning/thickening in "
                   "the positive J direction"}} );
    info.insert( {"BSHWVISK",
                  {A::SUMMARY_BLOCK,
                   "Shear viscosity of the water/polymer solution due to shear thinning/thickening in "
                   "the positive K direction"}} );
    info.insert( {"BNSAT", {A::SUMMARY_BLOCK, "Solvent SATuration"}} );
    info.insert( {"BNIP", {A::SUMMARY_BLOCK, "Solvent In Place"}} );
    info.insert( {"BNKR", {A::SUMMARY_BLOCK, "Solvent relative permeability"}} );
    info.insert( {"BTCNFSUR", {A::SUMMARY_BLOCK, "Concentration"}} );
    info.insert( {"BSURF", {A::SUMMARY_BLOCK, "Concentration in solution"}} );
    info.insert( {"BTIPTSUR", {A::SUMMARY_BLOCK, "In Solution"}} );
    info.insert( {"BTADSUR", {A::SUMMARY_BLOCK, "Adsorption"}} );
    info.insert( {"BTCASUR", {A::SUMMARY_BLOCK, "Log"}} );
    info.insert( {"BSURFCNM", {A::SUMMARY_BLOCK, "Log"}} );
    info.insert( {"BTSTSUR", {A::SUMMARY_BLOCK, "Surface tension"}} );
    info.insert( {"BSURFST", {A::SUMMARY_BLOCK, "Surface tension"}} );
    info.insert( {"BEWV_SUR", {A::SUMMARY_BLOCK, "Effective water viscosity due to surfactant concentration"}} );
    info.insert( {"BESVIS", {A::SUMMARY_BLOCK, "Effective water viscosity due to surfactant concentration"}} );
    info.insert( {"BTCNFALK", {A::SUMMARY_BLOCK, "Concentration"}} );
    info.insert( {"BTADSALK", {A::SUMMARY_BLOCK, "Adsorption"}} );
    info.insert( {"BTSTMALK", {A::SUMMARY_BLOCK, "Surface tension multiplier"}} );
    info.insert( {"BTSADALK", {A::SUMMARY_BLOCK, "Surfactant adsorption multiplier"}} );
    info.insert( {"BTPADALK", {A::SUMMARY_BLOCK, "Polymer adsorption multiplier"}} );
    info.insert( {"BKRGOE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to gas for gas-oil system"}} );
    info.insert( {"BKRGWE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to gas for gas-water system"}} );
    info.insert( {"BKRWGE", {A::SUMMARY_BLOCK, "Equivalent relative permeability to water for water-gas system"}} );
    info.insert( {"BKROWT",
                  {A::SUMMARY_BLOCK,
                   "Opposite saturation direction turning point relative permeability to oil for oil-water system"}} );
    info.insert(
        {"BKRWOT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to water for water-oil system"}} );
    info.insert( {"BKROGT",
                  {A::SUMMARY_BLOCK,
                   "Opposite saturation direction turning point relative permeability to oil for oil-gas system"}} );
    info.insert( {"BKRGOT",
                  {A::SUMMARY_BLOCK,
                   "Opposite saturation direction turning point relative permeability to gas for gas-oil system"}} );
    info.insert( {"BKRGWT",
                  {A::SUMMARY_BLOCK,
                   "Opposite saturation direction turning point relative permeability to gas for gas-water system"}} );
    info.insert(
        {"BKRWGT",
         {A::SUMMARY_BLOCK,
          "Opposite saturation direction turning point relative permeability to water for water-gas system"}} );
    info.insert( {"BIFTOW", {A::SUMMARY_BLOCK, "Oil-water interfacial tension"}} );
    info.insert( {"BIFTWO", {A::SUMMARY_BLOCK, "Water-oil interfacial tension"}} );
    info.insert( {"BIFTOG", {A::SUMMARY_BLOCK, "Oil-gas interfacial tension"}} );
    info.insert( {"BIFTGO", {A::SUMMARY_BLOCK, "Gas-oil interfacial tension"}} );
    info.insert( {"BIFTGW", {A::SUMMARY_BLOCK, "Gas-water interfacial tension"}} );
    info.insert( {"BIFTWG", {A::SUMMARY_BLOCK, "Water-gas interfacial tension"}} );
    info.insert( {"BPCOWR", {A::SUMMARY_BLOCK, "Representative oil-water capillary pressure"}} );
    info.insert( {"BPCWOR", {A::SUMMARY_BLOCK, "Representative water-oil capillary pressure"}} );
    info.insert( {"BPCOGR", {A::SUMMARY_BLOCK, "Representative oil-gas capillary pressure"}} );
    info.insert( {"BPCGOR", {A::SUMMARY_BLOCK, "Representative gas-oil capillary pressure"}} );
    info.insert( {"BPCGWR", {A::SUMMARY_BLOCK, "Representative gas-water capillary pressure"}} );
    info.insert( {"BPCWGR", {A::SUMMARY_BLOCK, "Representative water-gas capillary pressure"}} );

    info.insert( {"SOFR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Flow Rate"}} );
    info.insert( {"SOFRF", {A::SUMMARY_WELL_SEGMENT, "Segment Free Oil Flow Rate"}} );
    info.insert( {"SOFRS", {A::SUMMARY_WELL_SEGMENT, "Segment Solution Oil Flow Rate"}} );
    info.insert( {"SWFR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Flow Rate"}} );
    info.insert( {"SGFR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Flow Rate"}} );
    info.insert( {"SGFRF", {A::SUMMARY_WELL_SEGMENT, "Segment Free Gas Flow Rate"}} );
    info.insert( {"SGFRS", {A::SUMMARY_WELL_SEGMENT, "Segment Solution Gas Flow Rate"}} );
    info.insert( {"SKFR", {A::SUMMARY_WELL_SEGMENT, "Segment Component Flow Rate"}} );
    info.insert( {"SCWGFR", {A::SUMMARY_WELL_SEGMENT, "Segment Component Flow Rate as Wet Gas"}} );
    info.insert( {"SHFR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Flow Rate"}} );
    info.insert( {"SWCT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Cut"}} );
    info.insert( {"SGOR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Oil Ratio"}} );
    info.insert( {"SOGR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Gas Ratio"}} );
    info.insert( {"SWGR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Gas Ratio"}} );
    info.insert( {"SPR", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure"}} );
    info.insert( {"SPRD", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop"}} );
    info.insert( {"SPRDF", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop component due to Friction"}} );
    info.insert( {"SPRDH", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure Drop component due to Hydrostatic head"}} );
    info.insert( {"SPRDA", {A::SUMMARY_WELL_SEGMENT, "Segment Pressure drop due to Acceleration head"}} );
    info.insert( {"SPRDM", {A::SUMMARY_WELL_SEGMENT, "Segment frictional Pressure Drop Multiplier"}} );
    info.insert( {"SPPOW", {A::SUMMARY_WELL_SEGMENT, "Working power of a pull through pump"}} );
    info.insert( {"SOFV", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Flow Velocity"}} );
    info.insert( {"SWFV", {A::SUMMARY_WELL_SEGMENT, "Segment Water Flow Velocity"}} );
    info.insert( {"SGFV", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Flow Velocity"}} );
    info.insert( {"SOHF", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Holdup Fraction"}} );
    info.insert( {"SWHF", {A::SUMMARY_WELL_SEGMENT, "Segment Water Holdup Fraction"}} );
    info.insert( {"SGHF", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Holdup Fraction"}} );
    info.insert( {"SDENM", {A::SUMMARY_WELL_SEGMENT, "Segment fluid mixture density"}} );
    info.insert( {"SOVIS", {A::SUMMARY_WELL_SEGMENT, "Segment oil viscosity"}} );
    info.insert( {"SWVIS", {A::SUMMARY_WELL_SEGMENT, "Segment water viscosity"}} );
    info.insert( {"SGVIS", {A::SUMMARY_WELL_SEGMENT, "Segment gas viscosity"}} );
    info.insert( {"SEMVIS", {A::SUMMARY_WELL_SEGMENT, "Segment effective mixture viscosity"}} );
    info.insert( {"SGLPP", {A::SUMMARY_WELL_SEGMENT, "Segment Gas-Liquid Profile Parameter, C0"}} );
    info.insert( {"SGLVD", {A::SUMMARY_WELL_SEGMENT, "Segment Gas-Liquid Drift Velocity, Vd"}} );
    info.insert( {"SOWPP", {A::SUMMARY_WELL_SEGMENT, "Segment Oil-Water Profile Parameter, C0"}} );
    info.insert( {"SOWVD", {A::SUMMARY_WELL_SEGMENT, "Segment Oil-Water Drift Velocity, Vd"}} );
    info.insert( {"SOIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Import Rate"}} );
    info.insert( {"SGIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Import Rate"}} );
    info.insert( {"SWIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Import Rate"}} );
    info.insert( {"SHIMR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Import Rate"}} );
    info.insert( {"SORMR", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Removal Rate"}} );
    info.insert( {"SGRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Removal Rate"}} );
    info.insert( {"SWRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Water Removal Rate"}} );
    info.insert( {"SHRMR", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Removal Rate"}} );
    info.insert( {"SOIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Import Total"}} );
    info.insert( {"SGIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Import Total"}} );
    info.insert( {"SWIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Import Total"}} );
    info.insert( {"SHIMT", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Import Total"}} );
    info.insert( {"SORMT", {A::SUMMARY_WELL_SEGMENT, "Segment Oil Removal Total"}} );
    info.insert( {"SGRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Removal Total"}} );
    info.insert( {"SWRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Water Removal Total"}} );
    info.insert( {"SHRMT", {A::SUMMARY_WELL_SEGMENT, "Segment Enthalpy Removal Total"}} );
    info.insert( {"SAPI", {A::SUMMARY_WELL_SEGMENT, "Segment API value"}} );
    info.insert( {"SCFR", {A::SUMMARY_WELL_SEGMENT, "Segment polymer flow rate"}} );
    info.insert( {"SCCN", {A::SUMMARY_WELL_SEGMENT, "Segment polymer concentration"}} );
    info.insert( {"SSFR", {A::SUMMARY_WELL_SEGMENT, "Segment brine flow rate"}} );
    info.insert( {"SSCN", {A::SUMMARY_WELL_SEGMENT, "Segment brine concentration"}} );
    info.insert( {"STFR", {A::SUMMARY_WELL_SEGMENT, "Segment tracer flow rate"}} );
    info.insert( {"STFC", {A::SUMMARY_WELL_SEGMENT, "Segment tracer concentration"}} );
    info.insert( {"SFD", {A::SUMMARY_WELL_SEGMENT, "Segment diameter for Karst Conduit Calcite Dissolution"}} );
    info.insert( {"SPSAT", {A::SUMMARY_WELL_SEGMENT, "Segment Psat"}} );
    info.insert( {"STEM", {A::SUMMARY_WELL_SEGMENT, "Segment Temperature"}} );
    info.insert( {"SENE", {A::SUMMARY_WELL_SEGMENT, "Segment Energy Density"}} );
    info.insert( {"SSQU", {A::SUMMARY_WELL_SEGMENT, "Segment Steam Quality"}} );
    info.insert( {"SCVPR", {A::SUMMARY_WELL_SEGMENT, "Segment Calorific Value Production Rate"}} );
    info.insert( {"SGQ", {A::SUMMARY_WELL_SEGMENT, "Segment Gas Quality"}} );
    info.insert( {"SCSA", {A::SUMMARY_WELL_SEGMENT, "Segment Cross Sectional Area"}} );
    info.insert( {"SSTR", {A::SUMMARY_WELL_SEGMENT, "Strength of ICD on segment"}} );
    info.insert( {"SFOPN", {A::SUMMARY_WELL_SEGMENT, "Setting of segment"}} );
    info.insert( {"SALQ", {A::SUMMARY_WELL_SEGMENT, "Artificial lift quantity for segment"}} );
    info.insert( {"SRRQR", {A::SUMMARY_WELL_SEGMENT, "Reach flow at current time"}} );
    info.insert( {"SRRQT", {A::SUMMARY_WELL_SEGMENT, "Reach cumulative flow"}} );
    info.insert( {"SRBQR", {A::SUMMARY_WELL_SEGMENT, "Branch flow at current time"}} );
    info.insert( {"SRBQT", {A::SUMMARY_WELL_SEGMENT, "Branch cumulative flow"}} );
    info.insert( {"SRTQR", {A::SUMMARY_WELL_SEGMENT, "River total flow at current time"}} );
    info.insert( {"SRTQT", {A::SUMMARY_WELL_SEGMENT, "River total cumulative flow"}} );
    info.insert( {"SRRFLOW", {A::SUMMARY_WELL_SEGMENT, "Reach flux through cross-sectional area at current time"}} );
    info.insert( {"SRRAREA", {A::SUMMARY_WELL_SEGMENT, "Reach area at current time"}} );
    info.insert( {"SRRDEPTH", {A::SUMMARY_WELL_SEGMENT, "Reach depth at current time"}} );
    info.insert( {"SRREXCH", {A::SUMMARY_WELL_SEGMENT, "Exchange flux at current time"}} );
    info.insert( {"SRRFRODE", {A::SUMMARY_WELL_SEGMENT, "Reach Froude number at current time"}} );
    info.insert( {"SRRHEAD", {A::SUMMARY_WELL_SEGMENT, "Reach hydraulic head at current time"}} );
    info.insert( {"SRTFR", {A::SUMMARY_WELL_SEGMENT, "Reach tracer flow rate"}} );
    info.insert( {"SRTFC", {A::SUMMARY_WELL_SEGMENT, "Reach tracer concentration"}} );
    info.insert( {"SRSFR", {A::SUMMARY_WELL_SEGMENT, "Reach brine flow rate through connections"}} );
    info.insert( {"SRSFC", {A::SUMMARY_WELL_SEGMENT, "Reach brine concentration"}} );
    info.insert( {"SU", {A::SUMMARY_WELL_SEGMENT, "User-defined segment quantity"}} );

    info.insert( {"AAQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    info.insert( {"ALQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    info.insert( {"AAQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    info.insert( {"ALQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    info.insert( {"AAQRG", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    info.insert( {"ALQRG", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    info.insert( {"AAQTG", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    info.insert( {"ALQTG", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    info.insert( {"AACMR", {A::SUMMARY_AQUIFER, "Aquifer component molar influx rate"}} );
    info.insert( {"AACMT", {A::SUMMARY_AQUIFER, "Aquifer component molar influx totals"}} );
    info.insert( {"AAQP", {A::SUMMARY_AQUIFER, "Aquifer pressure"}} );
    info.insert( {"AAQER", {A::SUMMARY_AQUIFER, "Aquifer thermal energy influx rate"}} );
    info.insert( {"AAQET", {A::SUMMARY_AQUIFER, "Cumulative aquifer thermal energy influx"}} );
    info.insert( {"AAQTEMP", {A::SUMMARY_AQUIFER, "Aquifer temperature"}} );
    info.insert( {"AAQENTH", {A::SUMMARY_AQUIFER, "Aquifer molar enthalpy"}} );
    info.insert( {"AAQTD", {A::SUMMARY_AQUIFER, "Aquifer dimensionless time"}} );
    info.insert( {"AAQPD", {A::SUMMARY_AQUIFER, "Aquifer dimensionless pressure"}} );
    info.insert( {"ANQR", {A::SUMMARY_AQUIFER, "Aquifer influx rate"}} );
    info.insert( {"ANQT", {A::SUMMARY_AQUIFER, "Cumulative aquifer influx"}} );
    info.insert( {"ANQP", {A::SUMMARY_AQUIFER, "Aquifer pressure"}} );

    info.insert( {"CPU", {A::SUMMARY_MISC, "CPU"}} );
    info.insert( {"DATE", {A::SUMMARY_MISC, "Date"}} );
    info.insert( {"DAY", {A::SUMMARY_MISC, "Day"}} );
    info.insert( {"ELAPSED", {A::SUMMARY_MISC, "Elapsed time in seconds"}} );
    info.insert( {"MLINEARS", {A::SUMMARY_MISC, "Number linear iterations for each timestep"}} );
    info.insert( {"MONTH", {A::SUMMARY_MISC, "Month"}} );
    info.insert( {"MSUMLINS", {A::SUMMARY_MISC, "Total number of linear iterations since the start of the run"}} );
    info.insert( {"MSUMNEWT", {A::SUMMARY_MISC, "Total number of Newton iterations since the start of the run"}} );
    info.insert( {"NEWTON", {A::SUMMARY_MISC, "Number of Newton iterations used for each timestep"}} );
    info.insert( {"STEPTYPE", {A::SUMMARY_MISC, "Step type"}} );
    info.insert( {"TCPU", {A::SUMMARY_MISC, "TCPU"}} );
    info.insert( {"TCPUDAY", {A::SUMMARY_MISC, "TCPUDAY"}} );
    info.insert( {"TCPUTS", {A::SUMMARY_MISC, "TCPUTS"}} );
    info.insert( {"TELAPLIN", {A::SUMMARY_MISC, "TELAPLIN"}} );
    info.insert( {"TIME", {A::SUMMARY_MISC, "Time"}} );
    info.insert( {"TIMESTEP", {A::SUMMARY_MISC, "Time step"}} );
    info.insert( {"TIMESTRY", {A::SUMMARY_MISC, "TIMESTRY"}} );
    info.insert( {"YEAR", {A::SUMMARY_MISC, "Year"}} );
    info.insert( {"YEARS", {A::SUMMARY_MISC, "Years"}} );

    return info;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityInfo>
    RiuSummaryQuantityNameInfoProvider::createInfoFor6xKeywords()
{
    using A = RifEclipseSummaryAddress;

    std::map<std::string, RiuSummaryQuantityInfo> info;

    info.insert( {"BAPIp", {A::SUMMARY_BLOCK, "Oil API"}} );
    info.insert( {"BCCP", {A::SUMMARY_BLOCK, "Pore Volume Compressibility"}} );
    info.insert( {"BCCPp", {A::SUMMARY_BLOCK, "Pore Volume Compressibility"}} );
    info.insert( {"BDENGAS", {A::SUMMARY_BLOCK, "Gas Density"}} );
    info.insert( {"BDENGASp", {A::SUMMARY_BLOCK, "Gas Density"}} );
    info.insert( {"BDENOIL", {A::SUMMARY_BLOCK, "Oil Density"}} );
    info.insert( {"BDENOILp", {A::SUMMARY_BLOCK, "Oil Density"}} );
    info.insert( {"BDENWAT", {A::SUMMARY_BLOCK, "Water Density"}} );
    info.insert( {"BDENWATp", {A::SUMMARY_BLOCK, "Water Density"}} );
    info.insert( {"BDP", {A::SUMMARY_BLOCK, "Oil Density"}} );
    info.insert( {"BDPp", {A::SUMMARY_BLOCK, "Oil Density"}} );
    info.insert( {"BDYNBRST", {A::SUMMARY_BLOCK, "Water Density"}} );
    info.insert( {"BDYNBRSTp", {A::SUMMARY_BLOCK, "Water Density"}} );
    info.insert( {"BDYNKX", {A::SUMMARY_BLOCK, "Change In Pressure Since Initial State"}} );
    info.insert( {"BDYNKXp", {A::SUMMARY_BLOCK, "Change In Pressure Since Initial State"}} );
    info.insert( {"BDYNKY", {A::SUMMARY_BLOCK, "Dynamic Breaking Stress"}} );
    info.insert( {"BDYNKYp", {A::SUMMARY_BLOCK, "Dynamic Breaking Stress"}} );
    info.insert( {"BDYNKZ", {A::SUMMARY_BLOCK, "Dynamic Permeability In X-Direction"}} );
    info.insert( {"BDYNKZp", {A::SUMMARY_BLOCK, "Dynamic Permeability In X-Direction"}} );
    info.insert( {"BDYNPV", {A::SUMMARY_BLOCK, "Dynamic Permeability In Y-Direction"}} );
    info.insert( {"BDYNPVp", {A::SUMMARY_BLOCK, "Dynamic Permeability In Y-Direction"}} );
    info.insert( {"BDYNSIG", {A::SUMMARY_BLOCK, "Dynamic Permeability In Z-Direction"}} );
    info.insert( {"BDYNSIGp", {A::SUMMARY_BLOCK, "Dynamic Permeability In Z-Direction"}} );
    info.insert( {"BKRGAS", {A::SUMMARY_BLOCK, "Dynamic Pore Volume"}} );
    info.insert( {"BKRGASp", {A::SUMMARY_BLOCK, "Dynamic Pore Volume"}} );
    info.insert( {"BKROIL", {A::SUMMARY_BLOCK, "Dynamic Sigma Permeability"}} );
    info.insert( {"BKROILp", {A::SUMMARY_BLOCK, "Dynamic Sigma Permeability"}} );
    info.insert( {"BKRWAT", {A::SUMMARY_BLOCK, "Gas Relative Permeability"}} );
    info.insert( {"BKRWATp", {A::SUMMARY_BLOCK, "Gas Relative Permeability"}} );
    info.insert( {"BPCOG", {A::SUMMARY_BLOCK, "Oil Relative Permeability"}} );
    info.insert( {"BPCOGp", {A::SUMMARY_BLOCK, "Oil Relative Permeability"}} );
    info.insert( {"BPCOIL", {A::SUMMARY_BLOCK, "Water Relative Permeability"}} );
    info.insert( {"BPCOILp", {A::SUMMARY_BLOCK, "Water Relative Permeability"}} );
    info.insert( {"BPCOW", {A::SUMMARY_BLOCK, "Oil-Gas Capillary Pressure"}} );
    info.insert( {"BPCOWp", {A::SUMMARY_BLOCK, "Oil-Gas Capillary Pressure"}} );
    info.insert( {"BPCWAT", {A::SUMMARY_BLOCK, "Oil Capillary Pressure"}} );
    info.insert( {"BPCWATp", {A::SUMMARY_BLOCK, "Oil Capillary Pressure"}} );
    info.insert( {"BPMODHYS", {A::SUMMARY_BLOCK, "Oil-Water Capillary Pressure"}} );
    info.insert( {"BPMODHYSp", {A::SUMMARY_BLOCK, "Oil-Water Capillary Pressure"}} );
    info.insert( {"BPRp", {A::SUMMARY_BLOCK, "Water Capillary Pressure"}} );
    info.insert( {"BRSp", {A::SUMMARY_BLOCK, "Water Capillary Pressure"}} );
    info.insert( {"BSGASp", {A::SUMMARY_BLOCK, "Dynamic Pmod Hysteresis Curve Type (1/2/3=B/S/P)"}} );
    info.insert( {"BSOILp", {A::SUMMARY_BLOCK, "Dynamic Pmod Hysteresis Curve Type (1/2/3=B/S/P)"}} );
    info.insert( {"BSRVSTAT", {A::SUMMARY_BLOCK, "Pressure"}} );
    info.insert( {"BSRVSTATp", {A::SUMMARY_BLOCK, "Pressure"}} );
    info.insert( {"BSTRAIN", {A::SUMMARY_BLOCK, "Gas Rs"}} );
    info.insert( {"BSTRAINp", {A::SUMMARY_BLOCK, "Gas Rs"}} );
    info.insert( {"BSTRESS", {A::SUMMARY_BLOCK, "Gas Rv"}} );
    info.insert( {"BSTRESSp", {A::SUMMARY_BLOCK, "Gas Rv"}} );
    info.insert( {"BSTRESSA", {A::SUMMARY_BLOCK, "Gas Saturation"}} );
    info.insert( {"BSTRESSAp", {A::SUMMARY_BLOCK, "Gas Saturation"}} );
    info.insert( {"BSTRESSN", {A::SUMMARY_BLOCK, "Oil Saturation"}} );
    info.insert( {"BSTRESSNp", {A::SUMMARY_BLOCK, "Oil Saturation"}} );
    info.insert( {"BVISCGAS", {A::SUMMARY_BLOCK, "Srv Status (1/2/3=Stim/Sigprop/Propprop)"}} );
    info.insert( {"BVISCGASp", {A::SUMMARY_BLOCK, "Srv Status (1/2/3=Stim/Sigprop/Propprop)"}} );
    info.insert( {"BVISCOIL", {A::SUMMARY_BLOCK, "Volumetric Strain"}} );
    info.insert( {"BVISCOILp", {A::SUMMARY_BLOCK, "Volumetric Strain"}} );
    info.insert( {"BVISCWAT", {A::SUMMARY_BLOCK, "Mean Normal Stress"}} );
    info.insert( {"BVISCWATp", {A::SUMMARY_BLOCK, "Mean Normal Stress"}} );
    info.insert( {"BXi", {A::SUMMARY_BLOCK, "Mean Normal Stress Idealised Analytic Solution"}} );
    info.insert( {"BXip", {A::SUMMARY_BLOCK, "Mean Normal Stress Idealised Analytic Solution"}} );
    info.insert( {"BYi", {A::SUMMARY_BLOCK, "Net Stress"}} );
    info.insert( {"BYip", {A::SUMMARY_BLOCK, "Net Stress"}} );
    info.insert( {"BZi", {A::SUMMARY_BLOCK, "Water Saturation"}} );
    info.insert( {"BZip", {A::SUMMARY_BLOCK, "Water Saturation"}} );
    info.insert( {"FDG", {A::SUMMARY_FIELD, "Lock Gas Viscosity"}} );
    info.insert( {"FDO", {A::SUMMARY_FIELD, "Lock Gas Viscosity"}} );
    info.insert( {"FDW", {A::SUMMARY_FIELD, "Lock Oil Viscosity"}} );
    info.insert( {"FLIR", {A::SUMMARY_FIELD, "Lock Oil Viscosity"}} );
    info.insert( {"FLIRH", {A::SUMMARY_FIELD, "Lock Water Viscosity"}} );
    info.insert( {"FLIT", {A::SUMMARY_FIELD, "Lock Water Viscosity"}} );
    info.insert( {"FMBG", {A::SUMMARY_FIELD, "Lock Liquid Phase Mole Fraction"}} );
    info.insert( {"FMBO", {A::SUMMARY_FIELD, "Lock Liquid Phase Mole Fraction"}} );
    info.insert( {"FMBW", {A::SUMMARY_FIELD, "Lock Vapor Phase Mole Fraction"}} );
    info.insert( {"FMSTR", {A::SUMMARY_FIELD, "Lock Vapor Phase Mole Fraction"}} );
    info.insert( {"FMWIR", {A::SUMMARY_FIELD, "Grid Block Total Mole Fraction"}} );
    info.insert( {"FMWSH", {A::SUMMARY_FIELD, "Grid Block Total Mole Fraction"}} );
    info.insert( {"FMWST", {A::SUMMARY_FIELD, "Completion Gas Flow"}} );
    info.insert( {"CHOPS", {A::SUMMARY_MISC, "Completion Oil Flow"}} );
    info.insert( {"CPDIAM", {A::SUMMARY_MISC, "Completion Water Flow"}} );
    info.insert( {"MLINEART", {A::SUMMARY_MISC, "Number Of Time-Step Chops At The Current Time-Step"}} );
    info.insert( {"MSUMCHOP", {A::SUMMARY_MISC, "Perforation Diameter"}} );
    info.insert( {"MSUMLINT", {A::SUMMARY_MISC, "Field Aquifer Influx Rate"}} );
    info.insert( {"TS", {A::SUMMARY_MISC, "Field Cumulative Aquifer Influx"}} );
    info.insert( {"NGIR", {A::SUMMARY_NETWORK, "S In Place Difference From Initial Conditions"}} );
    info.insert( {"NGIRH", {A::SUMMARY_NETWORK, "S In Place Difference From Initial Conditions History"}} );
    info.insert( {"NGIT", {A::SUMMARY_NETWORK, "Ter In Place Difference From Initial Conditions"}} );
    info.insert( {"NGOR", {A::SUMMARY_NETWORK, "S In Place"}} );
    info.insert( {"NGORH", {A::SUMMARY_NETWORK, "S In Place History"}} );
    info.insert( {"NGPR", {A::SUMMARY_NETWORK, "Served Liquid Injection Rate"}} );
    info.insert( {"NGPRH", {A::SUMMARY_NETWORK, "Served Liquid Injection Rate History"}} );
    info.insert( {"NGPT", {A::SUMMARY_NETWORK, "S Phase Material Balance Error"}} );
    info.insert( {"NLINEARS", {A::SUMMARY_NETWORK, "L Phase Material Balance Error"}} );
    info.insert( {"NLIR", {A::SUMMARY_NETWORK, "Ter Phase Material Balance Error"}} );
    info.insert( {"NLIRH", {A::SUMMARY_NETWORK, "Ter Phase Material Balance Error History"}} );
    info.insert( {"NLIT", {A::SUMMARY_NETWORK, "Mber Of Injecting Wells"}} );
    info.insert( {"NLPR", {A::SUMMARY_NETWORK, "Mber Of Stopped Wells"}} );
    info.insert( {"NLPRH", {A::SUMMARY_NETWORK, "Mber Of Stopped Wells History"}} );
    info.insert( {"NLPT", {A::SUMMARY_NETWORK, "L In Place"}} );
    info.insert( {"NOIR", {A::SUMMARY_NETWORK, "Ter In Place"}} );
    info.insert( {"NOIRH", {A::SUMMARY_NETWORK, "Ter In Place History"}} );
    info.insert( {"NOIT", {A::SUMMARY_NETWORK, "Served Gas Injection Rate"}} );
    info.insert( {"NOPR", {A::SUMMARY_NETWORK, "Mulative Gas Injection"}} );
    info.insert( {"NOPRH", {A::SUMMARY_NETWORK, "Mulative Gas Injection History"}} );
    info.insert( {"NOPT", {A::SUMMARY_NETWORK, "Served Gas-Oil Ratio"}} );
    info.insert( {"NWCT", {A::SUMMARY_NETWORK, "Quid Injection Rate"}} );
    info.insert( {"NWCTH", {A::SUMMARY_NETWORK, "Served Liquid Injection Rate"}} );
    info.insert( {"NWIR", {A::SUMMARY_NETWORK, "Mulative Liquid Injection"}} );
    info.insert( {"NWIRH", {A::SUMMARY_NETWORK, "Mulative Liquid Injection History"}} );
    info.insert( {"NWIT", {A::SUMMARY_NETWORK, "Served Water Cut"}} );
    info.insert( {"NWPR", {A::SUMMARY_NETWORK, "Or A Well Completion"}} );
    info.insert( {"NWPRH", {A::SUMMARY_NETWORK, " History"}} );
    info.insert( {"NWPT", {A::SUMMARY_NETWORK, "F Tracer Linear Iterations At The Current Time-Step"}} );
    info.insert( {"RDG", {A::SUMMARY_REGION, "F Well Completion"}} );
    info.insert( {"RDG_dfname", {A::SUMMARY_REGION, "Total Number Of Time-Step Chops"}} );
    info.insert( {"RDO", {A::SUMMARY_REGION, "Umber Of Tracer Linear Iterations"}} );
    info.insert( {"RDO_dfname", {A::SUMMARY_REGION, "Network Gas Injection Rate"}} );
    info.insert( {"RDW", {A::SUMMARY_REGION, " Observed Gas Injection Rate"}} );
    info.insert( {"RDW_dfname", {A::SUMMARY_REGION, "Network Cumulative Gas Injection"}} );
    info.insert( {"REPT", {A::SUMMARY_REGION, "Network Gas-Oil Ratio"}} );
    info.insert( {"RGIP_dfname", {A::SUMMARY_REGION, "Network Observed Gas-Oil Ratio"}} );
    info.insert( {"RGIR_dfname", {A::SUMMARY_REGION, "Network Gas Production Rate"}} );
    info.insert( {"RGIT_dfname", {A::SUMMARY_REGION, "Network Observed Gas Production Rate"}} );
    info.insert( {"RGPR_dfname", {A::SUMMARY_REGION, "Network Cumulative Gas Production"}} );
    info.insert( {"RGPT_dfname", {A::SUMMARY_REGION, "Average Linear Iterations Per Newton Iteration"}} );
    info.insert( {"RLIR", {A::SUMMARY_REGION, " Liquid Injection Rate"}} );
    info.insert( {"RLIR_dfname", {A::SUMMARY_REGION, "Network Observed Liquid Injection Rate"}} );
    info.insert( {"RLIT", {A::SUMMARY_REGION, " Cumulative Liquid Injection"}} );
    info.insert( {"RLIT_dfname", {A::SUMMARY_REGION, "Network Liquid Production Rate"}} );
    info.insert( {"RLPR", {A::SUMMARY_REGION, " Observed Liquid Production Rate"}} );
    info.insert( {"RLPR_dfname", {A::SUMMARY_REGION, "Network Cumulative Liquid Production"}} );
    info.insert( {"RLPT", {A::SUMMARY_REGION, " Oil Injection Rate"}} );
    info.insert( {"RLPT_dfname", {A::SUMMARY_REGION, "Network Observed Oil Injection Rate"}} );
    info.insert( {"RMSTR", {A::SUMMARY_REGION, " Cumulative Oil Injection"}} );
    info.insert( {"RMSTR_dfname", {A::SUMMARY_REGION, "Network Oil Production Rate"}} );
    info.insert( {"ROIP_dfname", {A::SUMMARY_REGION, "Network Observed Oil Production Rate"}} );
    info.insert( {"ROIR_dfname", {A::SUMMARY_REGION, "Network Cumulative Oil Production"}} );
    info.insert( {"ROIT_dfname", {A::SUMMARY_REGION, "Network Water Cut"}} );
    info.insert( {"ROPR_dfname", {A::SUMMARY_REGION, "Network Observed Water Cut"}} );
    info.insert( {"ROPT_dfname", {A::SUMMARY_REGION, "Network Water Injection Rate"}} );
    info.insert( {"RPRP_dfname", {A::SUMMARY_REGION, "Network Observed Water Injection Rate"}} );
    info.insert( {"RPR_dfname", {A::SUMMARY_REGION, "Network Cumulative Water Injection"}} );
    info.insert( {"RWIP_dfname", {A::SUMMARY_REGION, "Network Water Production Rate"}} );
    info.insert( {"RWIR_dfname", {A::SUMMARY_REGION, "Network Observed Water Production Rate"}} );
    info.insert( {"RWIT_dfname", {A::SUMMARY_REGION, "Network Cumulative Water Production"}} );
    info.insert( {"RWPR_dfname", {A::SUMMARY_REGION, "Region Gas In Place Difference From Initial Conditions"}} );
    info.insert( {"RWPT_dfname", {A::SUMMARY_REGION, "Dynamic Region Gas In Place Difference From Initial Conditions"}} );
    info.insert( {"WADEN", {A::SUMMARY_WELL, "N Oil In Place Difference From Initial Conditions"}} );
    info.insert( {"WLIR", {A::SUMMARY_WELL, "Ic Region Oil In Place Difference From Initial Conditions"}} );
    info.insert( {"WLIRH", {A::SUMMARY_WELL, "Ic Region Oil In Place Difference From Initial Conditions History"}} );
    info.insert( {"WLIT", {A::SUMMARY_WELL, "Ic Region Water In Place Difference From Initial Conditions"}} );
    info.insert( {"WLPV", {A::SUMMARY_WELL, "Nt Report Step Number"}} );
    info.insert( {"WSBULKV", {A::SUMMARY_WELL, "N Gas In Place"}} );
    info.insert( {"WSBVPROP", {A::SUMMARY_WELL, "Ic Region Gas In Place"}} );
    info.insert( {"WSBVUNPR", {A::SUMMARY_WELL, "N Gas Injection Rate"}} );
    info.insert( {"WSMFSA", {A::SUMMARY_WELL, "Ic Region Gas Injection Rate"}} );
    info.insert( {"WSMFSAP", {A::SUMMARY_WELL, "N Cumulative Gas Injection Rate"}} );
    info.insert( {"WSMFSAU", {A::SUMMARY_WELL, "Ic Region Cumulative Gas Injection Rate"}} );
    info.insert( {"WSPORVF", {A::SUMMARY_WELL, "N Gas Production Rate"}} );
    info.insert( {"WSPORVM", {A::SUMMARY_WELL, "Ic Region Gas Production Rate"}} );
    info.insert( {"CFGAS", {A::SUMMARY_WELL_COMPLETION, "Gas Flow Rate"}} );
    info.insert( {"INFLOWi", {A::SUMMARY_WELL_COMPLETION, "Inflow Rate"}} );
    info.insert( {"MSDEPTH", {A::SUMMARY_WELL_COMPLETION, "Region Liquid Injection Rate"}} );
    info.insert( {"GLIR", {A::SUMMARY_WELL_GROUP, "C Region Liquid Injection Rate"}} );
    info.insert( {"GLIRH", {A::SUMMARY_WELL_GROUP, "C Region Liquid Injection Rate History"}} );
    info.insert( {"GLIT", {A::SUMMARY_WELL_GROUP, "C Region Cumulative Liquid Injection Rate"}} );

    return info;
}
