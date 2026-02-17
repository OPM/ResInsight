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

#pragma once

#include "Summary/RiaSummaryCurveDefinition.h"

#include "RimSummaryAddressCollection.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class RifEclipseSummaryAddress;
class RimEnsembleCurveSet;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryEnsemble;

//==================================================================================================
//
//==================================================================================================
namespace RiaSummaryAddressCollectionTools
{

std::vector<RiaSummaryCurveDefinition> buildCurveDefs( const std::vector<RiaSummaryCurveDefinition>&      sourceCurveDefs,
                                                       const std::string&                                 droppedName,
                                                       RimSummaryAddressCollection::CollectionContentType contentType,
                                                       bool                                               appendHistoryVectors );

std::vector<RiaSummaryCurveDefinition>
    removeExistingCurveDefs( const std::vector<RiaSummaryCurveDefinition>&                            candidateCurveDefs,
                             const std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>>&     existingSummaryCurves,
                             const std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>>& existingEnsembleCurves );

std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> buildAddressCaseMapFromCurves( const std::vector<RimSummaryCurve*>& curves );

std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>>
    buildAddressEnsembleMapFromCurveSets( const std::vector<RimEnsembleCurveSet*>& curveSets );

// Not used externally, public for testing purposes
std::string objectIdentifierForAddress( const RifEclipseSummaryAddress&                    address,
                                        RimSummaryAddressCollection::CollectionContentType contentType );

}; // namespace RiaSummaryAddressCollectionTools
