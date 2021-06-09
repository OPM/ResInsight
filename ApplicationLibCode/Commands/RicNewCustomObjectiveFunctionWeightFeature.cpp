/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewCustomObjectiveFunctionWeightFeature.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionWeight.h"
#include "RimEnsembleCurveSet.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "RiaStdStringTools.h"
#include "RifReaderEclipseSummary.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCustomObjectiveFunctionWeightFeature, "RicNewCustomObjectiveFunctionWeightFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionWeight*
    RicNewCustomObjectiveFunctionWeightFeature::addWeight( RimCustomObjectiveFunction* customObjectiveFunction )
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

    auto nativeQuantityName = RicNewCustomObjectiveFunctionWeightFeature::nativeQuantityName( candidateAdr.quantityName() );

    candidateAdr.setQuantityName( nativeQuantityName );
    newWeight->setSummaryAddress( candidateAdr );

    return newWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RicNewCustomObjectiveFunctionWeightFeature::nativeQuantityName( const std::string& quantityName )
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
bool RicNewCustomObjectiveFunctionWeightFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomObjectiveFunctionWeightFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selObj ) return;

    std::vector<RimCustomObjectiveFunction*> func;
    selObj->descendantsIncludingThisOfType( func );

    if ( func.size() == 1 )
    {
        auto firstObjectiveFunction = func.front();

        auto newWeight = addWeight( firstObjectiveFunction );

        firstObjectiveFunction->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( newWeight );
        RiuPlotMainWindowTools::setExpanded( func.front() );
    }

    selObj->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomObjectiveFunctionWeightFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Weight" );
    actionToSetup->setIcon( QIcon( ":/ObjectiveFunctionWeight.svg" ) );
}
