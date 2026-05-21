/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RicfSetSnapshotSize.h"

#include "RiaGuiApplication.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowTools.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QApplication>

CAF_PDM_SOURCE_INIT( RicfSetSnapshotSize, "setSnapshotSize" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetSnapshotSize::RicfSetSnapshotSize()
{
    CAF_PDM_InitScriptableField( &m_height, "height", -1, "Height" );
    CAF_PDM_InitScriptableField( &m_width, "width", -1, "Width" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetSnapshotSize::execute()
{
    if ( m_width() <= 0 || m_height() <= 0 )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "setSnapshotSize: width and height must both be > 0" );
    }

    RiaGuiApplication* guiApp = RiaGuiApplication::instance();
    if ( !guiApp )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "Need GUI ResInsight to set snapshot size" );
    }

    // Resize plot sub-windows so plot snapshots render at the requested size.
    // This is the same helper --snapshotsize uses in the CLI dispatch path.
    // Note: for the inner plot widget to also be resized (and not just the
    // outer multi-plot wrapper), the caller must have invoked
    // set_plot_window_size first so the plots are realized inside MDI
    // sub-windows. Without that, only the wrapper resizes and the qwt
    // canvas stays anchored at its initial geometry.
    if ( auto* plotMainWindow = guiApp->mainPlotWindow() )
    {
        RiuMainWindowTools::setWindowSizeOnWidgetsInMdiWindows( plotMainWindow, m_width(), m_height() );
    }

    // Mirror --snapshotsize behavior for 3D views as well.
    if ( auto* mainWindow = guiApp->mainWindow() )
    {
        RiuMainWindowTools::setFixedWindowSizeFor3dViews( mainWindow, m_width(), m_height() );
    }

    // Let the resize events propagate before the caller's next gRPC
    // request (typically export_snapshot) renders the widget.
    QApplication::processEvents();

    return caf::PdmScriptResponse();
}
