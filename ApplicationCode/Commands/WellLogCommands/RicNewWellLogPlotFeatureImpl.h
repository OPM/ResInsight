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

#include <QString>

class RimWellLogPlotCollection;
class RimWellLogPlot;
class RimWellLogTrack;

//==================================================================================================
/// 
//==================================================================================================
class RicNewWellLogPlotFeatureImpl
{
public:
    static RimWellLogPlot*  createWellLogPlot(bool showAfterCreation = true, const QString& plotDescription = QString(""));
    static RimWellLogTrack* createWellLogPlotTrack(bool updateAfterCreation = true, const QString& trackDescription = QString(""), RimWellLogPlot* existingPlot = nullptr);
    static void             updateAfterCreation(RimWellLogPlot* plot);
private:
    static RimWellLogPlotCollection* wellLogPlotCollection();
};
