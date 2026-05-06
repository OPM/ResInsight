/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiuMessageDialog.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaRegressionTestRunner.h"

#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageDialog::showError( QWidget* parent, const QString& title, const QString& text )
{
    if ( RiaGuiApplication::isRunning() && !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        if ( parent == nullptr )
        {
            parent = RiaGuiApplication::widgetToUseAsParent();
        }
        QMessageBox dlg( QMessageBox::Critical, title, text, QMessageBox::Ok, parent );
        dlg.setWindowFlags( dlg.windowFlags() | Qt::WindowStaysOnTopHint );
        dlg.exec();
    }

    RiaLogging::error( text.toStdString() );
}
