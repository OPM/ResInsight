/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Riu3DMainWindowTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RiuMainWindow.h"

#include <QMessageBox>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* Riu3DMainWindowTools::mainWindowWidget()
{
    return RiuMainWindow::instance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::setActiveViewer( QWidget* subWindow )
{
    if ( RiuMainWindow::instance() )
    {
        RiuMainWindow::instance()->setActiveViewer( subWindow );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::setExpanded( const caf::PdmUiItem* uiItem, bool expanded /*= true*/ )
{
    if ( RiuMainWindow::instance() )
    {
        RiuMainWindow::instance()->setExpanded( uiItem, expanded );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::selectAsCurrentItem( const caf::PdmObject* object, bool allowActiveViewChange /*= true*/ )
{
    if ( RiuMainWindow::instance() )
    {
        RiuMainWindow::instance()->selectAsCurrentItem( object, allowActiveViewChange );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::reportAndShowWarning( const QString& warningDialogHeader, const QString& warningtext )
{
    if ( RiaGuiApplication::isRunning() )
    {
        QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), warningDialogHeader, warningtext );
    }

    RiaLogging::error( warningtext );
}
