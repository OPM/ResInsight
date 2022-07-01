/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfSetPlotWindowSize.h"

#include "RiaGuiApplication.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfSetPlotWindowSize, "setPlotWindowSize" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetPlotWindowSize::RicfSetPlotWindowSize()
{
    CAF_PDM_InitScriptableField( &m_height, "height", -1, "Height" );
    CAF_PDM_InitScriptableField( &m_width, "width", -1, "Width" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetPlotWindowSize::execute()
{
    RiaGuiApplication* guiApp = RiaGuiApplication::instance();
    if ( guiApp )
    {
        guiApp->getOrCreateAndShowMainPlotWindow()->resize( m_width, m_height );
        return caf::PdmScriptResponse();
    }
    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "Need GUI ResInsight to set plot window size" );
}
