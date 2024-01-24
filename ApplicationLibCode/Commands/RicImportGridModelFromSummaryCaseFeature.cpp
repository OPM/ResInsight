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

#include "RicImportGridModelFromSummaryCaseFeature.h"

#include "RiaImportEclipseCaseTools.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimReloadCaseTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportGridModelFromSummaryCaseFeature, "RicImportGridModelFromSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    auto* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();

    RimReloadCaseTools::openOrImportGridModelFromSummaryCase( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );

    auto* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();

    QString summaryCaseName;
    if ( summaryCase ) summaryCaseName = summaryCase->caseName();

    QString txt;
    auto    gridCase = RimReloadCaseTools::gridModelFromSummaryCase( summaryCase );
    if ( gridCase )
    {
        txt = "Open Grid Model View";
    }
    else
    {
        txt = "Import Grid Model";
    }

    if ( !summaryCaseName.isEmpty() )
    {
        txt += QString( " for '%1'" ).arg( summaryCaseName );
    }

    actionToSetup->setText( txt );
}
