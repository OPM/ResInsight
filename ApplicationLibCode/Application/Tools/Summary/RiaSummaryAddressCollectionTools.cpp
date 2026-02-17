/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaSummaryAddressCollectionTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressDefines.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCurve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaSummaryAddressCollectionTools::objectIdentifierForAddress( const RifEclipseSummaryAddress&                    address,
                                                                          RimSummaryAddressCollection::CollectionContentType contentType )
{
    using Category    = RifEclipseSummaryAddressDefines::SummaryCategory;
    using ContentType = RimSummaryAddressCollection::CollectionContentType;

    if ( ( address.category() == Category::SUMMARY_WELL ) && ( contentType == ContentType::WELL ) )
    {
        return address.wellName();
    }
    else if ( ( address.category() == Category::SUMMARY_GROUP ) && ( contentType == ContentType::GROUP ) )
    {
        return address.groupName();
    }
    else if ( ( address.category() == Category::SUMMARY_NETWORK ) && ( contentType == ContentType::NETWORK ) )
    {
        return address.networkName();
    }
    else if ( ( address.category() == Category::SUMMARY_REGION ) && ( contentType == ContentType::REGION ) )
    {
        return std::to_string( address.regionNumber() );
    }
    else if ( ( address.category() == Category::SUMMARY_WELL_SEGMENT ) && ( contentType == ContentType::WELL_SEGMENT ) )
    {
        return std::to_string( address.wellSegmentNumber() );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition>
    RiaSummaryAddressCollectionTools::buildCurveDefs( const std::vector<RiaSummaryCurveDefinition>&      sourceCurveDefs,
                                                      const std::string&                                 droppedName,
                                                      RimSummaryAddressCollection::CollectionContentType contentType,
                                                      bool                                               appendHistoryVectors )
{
    std::set<RiaSummaryCurveDefinition> uniqueCurveDefs;

    for ( const auto& curveDef : sourceCurveDefs )
    {
        auto       newCurveDef = curveDef;
        const auto curveAdr    = newCurveDef.summaryAddressY();

        std::string objectIdentifierString = objectIdentifierForAddress( curveAdr, contentType );

        if ( !objectIdentifierString.empty() )
        {
            newCurveDef.setIdentifierText( curveAdr.category(), droppedName );
            uniqueCurveDefs.insert( newCurveDef );

            const auto& addr = curveDef.summaryAddressY();
            if ( !addr.isHistoryVector() && appendHistoryVectors )
            {
                auto historyAddr = addr;
                historyAddr.setVectorName( addr.vectorName() + RifEclipseSummaryAddressDefines::historyIdentifier() );

                auto historyCurveDef = newCurveDef;
                historyCurveDef.setSummaryAddressY( historyAddr );
                uniqueCurveDefs.insert( historyCurveDef );
            }
        }
    }

    return { uniqueCurveDefs.begin(), uniqueCurveDefs.end() };
}

//--------------------------------------------------------------------------------------------------
/// Filter out candidate curve definitions that already have a matching curve on the plot.
/// A candidate is considered a duplicate only when both the summary address and the data source (case or ensemble)
/// match an existing curve. This allows the same vector from a different case to pass through.
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiaSummaryAddressCollectionTools::removeExistingCurveDefs(
    const std::vector<RiaSummaryCurveDefinition>&                            candidateCurveDefs,
    const std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>>&     existingSummaryCurves,
    const std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>>& existingEnsembleCurves )
{
    std::vector<RiaSummaryCurveDefinition> filtered;

    for ( const auto& curveDef : candidateCurveDefs )
    {
        const auto& addr = curveDef.summaryAddressY();

        if ( curveDef.ensemble() )
        {
            // Skip if the plot already has an ensemble curve set with this address for the same ensemble
            auto it = existingEnsembleCurves.find( addr );
            if ( it != existingEnsembleCurves.end() && it->second.count( curveDef.ensemble() ) > 0 ) continue;
        }
        else if ( curveDef.summaryCaseY() )
        {
            // Skip if the plot already has a summary curve with this address for the same case
            auto it = existingSummaryCurves.find( addr );
            if ( it != existingSummaryCurves.end() && it->second.count( curveDef.summaryCaseY() ) > 0 ) continue;
        }

        filtered.push_back( curveDef );
    }

    return filtered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>>
    RiaSummaryAddressCollectionTools::buildAddressCaseMapFromCurves( const std::vector<RimSummaryCurve*>& curves )
{
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> result;
    for ( auto* curve : curves )
    {
        if ( !curve || !curve->summaryCaseY() ) continue;
        result[curve->summaryAddressY()].insert( curve->summaryCaseY() );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>>
    RiaSummaryAddressCollectionTools::buildAddressEnsembleMapFromCurveSets( const std::vector<RimEnsembleCurveSet*>& curveSets )
{
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>> result;
    for ( auto* curveSet : curveSets )
    {
        result[curveSet->summaryAddressY()].insert( curveSet->summaryEnsemble() );
    }
    return result;
}
