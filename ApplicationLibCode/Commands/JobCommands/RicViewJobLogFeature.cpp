/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicViewJobLogFeature.h"

#include "RiuTextDialog.h"

#include "Jobs/RimGenericJob.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QStringList>

CAF_CMD_SOURCE_INIT( RicViewJobLogFeature, "RicViewJobLogFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicViewJobLogFeature::onActionTriggered( bool isChecked )
{
    if ( auto job = dynamic_cast<RimGenericJob*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        const QStringList log = job->jobLog();

        QString text = log.join( "\n" );

        auto* textDlg = new RiuTextDialog();
        textDlg->setMinimumSize( 600, 800 );

        textDlg->setWindowTitle( "Job log for \"" + job->name() + "\"" );
        textDlg->setText( text );

        textDlg->exec();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicViewJobLogFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/inspect.svg" ) );
    actionToSetup->setText( "View Log..." );
}
