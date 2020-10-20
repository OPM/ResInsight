/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#pragma once

#include "cafCmdFeature.h"

class RimEclipseCase;
class RimGeoMechCase;
class RimEclipseContourMapView;
class RimGeoMechContourMapView;
class RimEclipseView;
class RimGeoMechView;

//==================================================================================================
///
//==================================================================================================
class RicNewContourMapViewFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    static RimEclipseContourMapView*
                                     createEclipseContourMapFromExistingContourMap( RimEclipseCase*           eclipseCase,
                                                                                    RimEclipseContourMapView* existingContourMap );
    static RimEclipseContourMapView* createEclipseContourMapFrom3dView( RimEclipseCase*       eclipseCase,
                                                                        const RimEclipseView* sourceView );
    static RimEclipseContourMapView* createEclipseContourMap( RimEclipseCase* eclipseCase );

    static RimGeoMechContourMapView*
                                     createGeoMechContourMapFromExistingContourMap( RimGeoMechCase*           geoMechCase,
                                                                                    RimGeoMechContourMapView* existingContourMap );
    static RimGeoMechContourMapView* createGeoMechContourMapFrom3dView( RimGeoMechCase*       geoMechCase,
                                                                        const RimGeoMechView* sourceView );
    static RimGeoMechContourMapView* createGeoMechContourMap( RimGeoMechCase* geoMechCase );

    static void assignDefaultResultAndLegend( RimEclipseContourMapView* contourMap );
};
