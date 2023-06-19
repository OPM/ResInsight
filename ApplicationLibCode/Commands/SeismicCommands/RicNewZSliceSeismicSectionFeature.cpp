/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RicNewZSliceSeismicSectionFeature.h"

#include "RicSeismicSectionFeatureImpl.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewZSliceSeismicSectionFeature, "RicNewZSliceSeismicSectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewZSliceSeismicSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewZSliceSeismicSectionFeature::onActionTriggered( bool isChecked )
{
    RicSeismicSectionFeatureImpl::createSeismicSection( RiaDefines::SeismicSectionType::SS_DEPTHSLICE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewZSliceSeismicSectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Seismic16x16.png" ) );
    actionToSetup->setText( "New Depth Slice" );
}
