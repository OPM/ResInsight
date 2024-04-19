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
class RimEclipseView;
class RimGeoMechCase;
class RimGeoMechView;
class Rim3dView;
class RimEclipseViewCollection;
class RimEclipseCaseEnsemble;

//==================================================================================================
///
//==================================================================================================
class RicNewViewFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void addReservoirView( RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase, RimEclipseViewCollection* viewColl );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static Rim3dView* createReservoirView( RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase, RimEclipseViewCollection* viewColl );

    static RimEclipseCase*           selectedEclipseCase();
    static RimGeoMechCase*           selectedGeoMechCase();
    static RimEclipseView*           selectedEclipseView();
    static RimGeoMechView*           selectedGeoMechView();
    static RimEclipseViewCollection* selectedEclipseViewCollection();
    static RimEclipseCaseEnsemble*   selectedEclipseCaseEnsemble();
};
