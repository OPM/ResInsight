/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicSplitMultiPlotFeature.h"

#include "Summary/RiaSummaryPlotTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSplitMultiPlotFeature, "RicSplitMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSplitMultiPlotFeature::isCommandEnabled() const
{
    RimSummaryPlot* plot = getSelectedPlot();
    if ( plot )
    {
        return ( ( plot->summaryCurves().size() > 1 ) || ( plot->curveSets().size() > 1 ) );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSplitMultiPlotFeature::onActionTriggered( bool isChecked )
{
    RimSummaryPlot* plot = getSelectedPlot();
    if ( plot == nullptr ) return;

    std::vector<caf::PdmObjectHandle*> objects = {};

    for ( auto curve : plot->summaryCurves() )
    {
        RimSummaryAddress* addr = RimSummaryAddress::wrapFileReaderAddress( curve->summaryAddressY() );
        addr->setCaseId( curve->summaryCaseY()->caseId() );
        objects.push_back( addr );
    }

    for ( auto curveSet : plot->curveSets() )
    {
        RimSummaryAddress* addr = RimSummaryAddress::wrapFileReaderAddress( curveSet->summaryAddressY() );
        addr->setEnsembleId( curveSet->ensembleId() );
        objects.push_back( addr );
    }

    RiaSummaryPlotTools::createAndAppendSummaryMultiPlot( objects );

    for ( auto object : objects )
        delete object;

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSplitMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Split Into Single Curve Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSplitMultiPlotFeature::getSelectedPlot() const
{
    RimSummaryPlot* plot = nullptr;

    if ( sender() )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<void*>() )
        {
            plot = static_cast<RimSummaryPlot*>( userData.value<void*>() );
        }
    }

    if ( plot == nullptr )
    {
        const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
        if ( !selectedItems.empty() ) plot = dynamic_cast<RimSummaryPlot*>( selectedItems[0] );
    }

    return plot;
}
