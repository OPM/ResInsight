/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimFishbonesDefines.h"

class RimFishbones;
class RimFishbonesCollection;
class RimWellPath;
class QAction;
class QMenu;
class QString;

//==================================================================================================
/// Helper class for common fishbones creation functionality
//==================================================================================================
class RicFishbonesCreateHelper
{
public:
    static RimFishbones* createAndConfigureFishbones( RimFishbonesCollection*                                  fishbonesCollection,
                                                      const RimFishbonesDefines::RicFishbonesSystemParameters& customParameters,
                                                      double                                                   measuredDepth );

    static void setupFishbonesSubMenu( QAction* actionToSetup, const QString& actionText );

private:
    static void finalizeFishbonesCreation( RimFishbones* fishbones, RimFishbonesCollection* fishbonesCollection );
};
