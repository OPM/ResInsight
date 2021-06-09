/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RimObjectiveFunctionTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RiaStdStringTools.h"
#include "RifReaderEclipseSummary.h"
#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionWeight.h"
#include "RimEnsembleCurveSet.h"
#include "RiuSummaryVectorSelectionDialog.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionWeight* RimObjectiveFunctionTools::addWeight( RimCustomObjectiveFunction* customObjectiveFunction )
{
    if ( !customObjectiveFunction ) return nullptr;

    RimCustomObjectiveFunctionWeight* newWeight = customObjectiveFunction->addWeight();

    RifEclipseSummaryAddress candidateAdr;
    if ( customObjectiveFunction->weights().size() > 1 )
    {
        candidateAdr = customObjectiveFunction->weights().front()->summaryAddresses().front();
    }
    else
    {
        candidateAdr = newWeight->parentCurveSet()->summaryAddress();
    }

    auto nativeQuantityName = RimObjectiveFunctionTools::nativeQuantityName( candidateAdr.quantityName() );

    candidateAdr.setQuantityName( nativeQuantityName );
    newWeight->setSummaryAddress( candidateAdr );

    return newWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimObjectiveFunctionTools::nativeQuantityName( const std::string& quantityName )
{
    std::string nativeName = quantityName;

    {
        auto stringToRemove = RifReaderEclipseSummary::differenceIdentifier();
        if ( RiaStdStringTools::endsWith( nativeName, stringToRemove ) )
        {
            nativeName = nativeName.substr( 0, nativeName.size() - stringToRemove.size() );
        }
    }

    {
        auto stringToRemove = RifReaderEclipseSummary::historyIdentifier();
        if ( RiaStdStringTools::endsWith( nativeName, stringToRemove ) )
        {
            nativeName = nativeName.substr( 0, nativeName.size() - stringToRemove.size() );
        }
    }

    return nativeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunctionTools::configureDialogForObjectiveFunctions( RiuSummaryVectorSelectionDialog* dialog )
{
    if ( !dialog ) return;

    dialog->enableMultiSelect( true );
    dialog->hideDifferenceVectors();
    dialog->hideHistoryVectors();
    dialog->hideVectorsWithNoHistory();
    dialog->hideSummaryCases();
}
