/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-    Equinor ASA
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

#include "cvfVector3.h"

class RimStimPlanModel;
class RimWellPath;

class QString;
class QTextStream;

//==================================================================================================
//
//==================================================================================================
class RifStimPlanModelPerfsFrkExporter
{
public:
    static bool writeToFile( RimStimPlanModel* stimPlanModel, const QString& filepath );

private:
    static void   appendHeaderToStream( QTextStream& stream );
    static void   appendFractureOrientationToStream( QTextStream& stream, bool isTranseverse );
    static void   appendPerforationToStream( QTextStream& stream, int index, double topMd, double bottomMd );
    static void   appendFooterToStream( QTextStream& stream );
    static double computeMeasuredDepthForPosition( const RimWellPath* wellPath, const cvf::Vec3d& position );
};
