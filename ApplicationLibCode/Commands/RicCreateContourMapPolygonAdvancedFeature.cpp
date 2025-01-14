/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicCreateContourMapPolygonAdvancedFeature.h"

#include "RicCreateContourMapPolygonTools.h"
#include "RicPolygonFromImageDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateContourMapPolygonAdvancedFeature, "RicCreateContourMapPolygonAdvancedFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonAdvancedFeature::onActionTriggered( bool isChecked )
{
    auto rigContourMapProjection = RicCreateContourMapPolygonTools::findCurrentContourMapProjection();
    if ( !rigContourMapProjection ) return;

    auto sourceImage = RicCreateContourMapPolygonTools::convertToBinaryImage( rigContourMapProjection );

    ImageProcessingDialog dlg;
    dlg.setSourceImageData( sourceImage );
    dlg.show();
    dlg.updateAndShowImages();

    if ( dlg.exec() == QDialog::Rejected ) return;

    auto processedImage = dlg.processedImageData();
    if ( processedImage.empty() ) return;

    RicCreateContourMapPolygonTools::createAndAddBoundaryPolygonFromImage( processedImage, rigContourMapProjection );
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonAdvancedFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
    actionToSetup->setText( "Create Polygon From Contour Map - Interactive" );
}
