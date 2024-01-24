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

#include "RicImportGridModelFromSummaryCurveFeature.h"

#include "RimFileSummaryCase.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportGridModelFromSummaryCurveFeature, "RicImportGridModelFromSummaryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCurveFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( !userData.isNull() && userData.canConvert<int>() )
    {
        int summaryCaseId = userData.value<int>();

        auto sumCases = RimProject::current()->allSummaryCases();
        for ( auto sumCase : sumCases )
        {
            if ( sumCase->caseId() == summaryCaseId )
            {
                if ( auto fileSummaryCase = dynamic_cast<RimFileSummaryCase*>( sumCase ) )
                {
                    RimReloadCaseTools::openOrImportGridModelFromSummaryCase( fileSummaryCase );

                    return;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );

    // No action text is given here, as the custom text is defined in
    // RiuSummaryQwtPlot::contextMenuEvent
}
