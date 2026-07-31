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

#include "RiuNameConflictTools.h"

#include "RiaGuiApplication.h"
#include "RiaNameUniquenessTools.h"
#include "RiaRegressionTestRunner.h"

#include <QMessageBox>
#include <QPushButton>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<QString> RiuNameConflictTools::resolveRenameConflict( const caf::PdmObjectHandle* object, const QString& desiredName )
{
    if ( RiaNameUniquenessTools::isUniqueAmongSiblings( object, desiredName ) ) return desiredName;

    const QString suggestedName = RiaNameUniquenessTools::makeUniqueAmongSiblings( object, desiredName );

    if ( !RiaGuiApplication::isRunning() || RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        return suggestedName;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle( "Name Already In Use" );
    msgBox.setIcon( QMessageBox::Icon::Warning );
    msgBox.setText( QString( "\"%1\" already exists in this folder." ).arg( desiredName ) );
    msgBox.setInformativeText( QString( "Use \"%1\" instead?" ).arg( suggestedName ) );

    auto* useSuggestedButton = msgBox.addButton( QString( "Use \"%1\"" ).arg( suggestedName ), QMessageBox::AcceptRole );
    msgBox.addButton( QMessageBox::Cancel );
    msgBox.setDefaultButton( useSuggestedButton );

    msgBox.exec();

    if ( msgBox.clickedButton() == useSuggestedButton ) return suggestedName;

    return {};
}
