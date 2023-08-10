/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicCopyGridStatisticsToClipboardFeature.h"

#include "RiaApplication.h"

#include "RicWellLogTools.h"

#include "RimGridView.h"

#include "RiuViewer.h"

#include <QAction>
#include <QClipboard>
#include <QRegularExpression>

CAF_CMD_SOURCE_INIT( RicCopyGridStatisticsToClipboardFeature, "RicCopyGridStatisticsToClipboardFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyGridStatisticsToClipboardFeature::isCommandEnabled() const
{
    if ( RicWellLogTools::isWellPathOrSimWellSelectedInView() ) return false;

    return RiaApplication::instance()->activeGridView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyGridStatisticsToClipboardFeature::onActionTriggered( bool isChecked )
{
    RimGridView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView();

    if ( activeView )
    {
        auto text = activeView->viewer()->infoText();
        if ( !text.isEmpty() )
        {
            // Replace some tokens with newline
            text.replace( "<p>", "\n" );
            text.replace( "<br>", "\n" );
            text.replace( "<tr>", "\n" );
            text.replace( "</tr>", "\n" );

            // Regular expression to match HTML tags
            QRegularExpression htmlTagsRegex( "<[^>]*>" );

            // Remove HTML tags from the input string
            text.remove( htmlTagsRegex );

            QApplication::clipboard()->setText( text );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyGridStatisticsToClipboardFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Copy Grid Statistics to Clipboard" );
    actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
}
