/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Statoil ASA
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

#include "RigWellPathGeometryExporter.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathGeometryExporter::exportWellPathGeometry( const RimWellPath*   wellPath,
                                                          double               mdStepSize,
                                                          std::vector<double>& xValues,
                                                          std::vector<double>& yValues,
                                                          std::vector<double>& tvdValues,
                                                          std::vector<double>& mdValues,
                                                          bool&                useMdRkb )
{
    auto wellPathGeom = wellPath->wellPathGeometry();
    if ( !wellPathGeom ) return;

    useMdRkb         = false;
    double rkbOffset = 0.0;
    {
        const RimModeledWellPath* modeledWellPath = dynamic_cast<const RimModeledWellPath*>( wellPath );
        if ( modeledWellPath )
        {
            useMdRkb = true;
            if ( modeledWellPath->geometryDefinition()->airGap() != 0.0 )
            {
                rkbOffset = modeledWellPath->geometryDefinition()->airGap();
            }
            else
            {
                rkbOffset = modeledWellPath->geometryDefinition()->mdAtFirstTarget();
            }
        }
    }
    exportWellPathGeometry( wellPathGeom, mdStepSize, rkbOffset, xValues, yValues, tvdValues, mdValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathGeometryExporter::exportWellPathGeometry( const RigWellPath*   wellPathGeom,
                                                          double               mdStepSize,
                                                          double               rkbOffset,
                                                          std::vector<double>& xValues,
                                                          std::vector<double>& yValues,
                                                          std::vector<double>& tvdValues,
                                                          std::vector<double>& mdValues )
{
    double currMd = wellPathGeom->measureDepths().front() - mdStepSize;
    double endMd  = wellPathGeom->measureDepths().back();

    while ( currMd < endMd )
    {
        currMd += mdStepSize;
        if ( currMd > endMd ) currMd = endMd;

        auto   pt  = wellPathGeom->interpolatedPointAlongWellPath( currMd );
        double tvd = -pt.z();

        xValues.push_back( pt.x() );
        yValues.push_back( pt.y() );
        tvdValues.push_back( tvd );
        mdValues.push_back( currMd + rkbOffset );
    }
}
