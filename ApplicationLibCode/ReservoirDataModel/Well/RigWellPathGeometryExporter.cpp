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

#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathGeometryExporter::computeWellPathDataForExport( gsl::not_null<const RimWellPath*> wellPath,
                                                                double                            mdStepSize,
                                                                std::vector<double>&              xValues,
                                                                std::vector<double>&              yValues,
                                                                std::vector<double>&              tvdValues,
                                                                std::vector<double>&              mdValues,
                                                                bool&                             showTextMdRkb )
{
    auto wellPathGeom = wellPath->wellPathGeometry();
    if ( !wellPathGeom ) return;

    showTextMdRkb    = false;
    double rkbOffset = 0.0;

    {
        auto topLevelWellPath = wellPath.get()->topLevelWellPath();

        // Always use top level modeled well path for definitions MD at first coordinate

        const RimModeledWellPath* modeledWellPath = dynamic_cast<const RimModeledWellPath*>( topLevelWellPath );
        if ( modeledWellPath )
        {
            if ( modeledWellPath->geometryDefinition()->airGap() != 0.0 )
            {
                showTextMdRkb = true;
                rkbOffset     = modeledWellPath->geometryDefinition()->airGap();
            }
        }

        if ( dynamic_cast<const RimFileWellPath*>( topLevelWellPath ) )
        {
            showTextMdRkb = true;
        }
    }

    computeWellPathDataForExport( *wellPathGeom, mdStepSize, rkbOffset, xValues, yValues, tvdValues, mdValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathGeometryExporter::computeWellPathDataForExport( const RigWellPath&   wellPathGeom,
                                                                double               mdStepSize,
                                                                double               rkbOffset,
                                                                std::vector<double>& xValues,
                                                                std::vector<double>& yValues,
                                                                std::vector<double>& tvdValues,
                                                                std::vector<double>& mdValues )
{
    double currMd = wellPathGeom.measuredDepths().front() - mdStepSize;
    double endMd  = wellPathGeom.measuredDepths().back();

    bool   isFirst = true;
    double prevMd  = 0.0;
    double prevTvd = 0.0;
    while ( currMd < endMd )
    {
        currMd += mdStepSize;
        if ( currMd > endMd ) currMd = endMd;

        auto   pt  = wellPathGeom.interpolatedPointAlongWellPath( currMd );
        double tvd = -pt.z();

        // The change in measured depth (MD) must be greater or equal to change in
        // true vertical depth (TVD), and this was not always the case due to
        // interpolation imprecision. Fix by adjusting TVD.
        if ( !isFirst )
        {
            double deltaMd  = currMd - prevMd;
            double deltaTvd = tvd - prevTvd;
            if ( deltaMd < deltaTvd )
            {
                tvd = prevTvd + deltaMd;
            }
        }

        xValues.push_back( pt.x() );
        yValues.push_back( pt.y() );
        tvdValues.push_back( tvd );
        mdValues.push_back( currMd + rkbOffset );

        prevMd  = currMd;
        prevTvd = tvd;
        isFirst = false;
    }
}
