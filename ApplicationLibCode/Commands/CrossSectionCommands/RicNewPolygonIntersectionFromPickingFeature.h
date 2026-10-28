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

#pragma once

#include "cafCmdFeature.h"

class RimIntersectionCollection;

//==================================================================================================
/// Creates a new project polygon, attaches a polygon intersection to it and activates 3D editing of
/// the polygon, so that points are appended when clicking on geometry in the 3D view.
//==================================================================================================
class RicNewPolygonIntersectionFromPickingFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static RimIntersectionCollection* intersectionCollection();
};
