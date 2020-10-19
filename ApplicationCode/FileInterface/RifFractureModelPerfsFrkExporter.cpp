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

#include "RifFractureModelPerfsFrkExporter.h"

#include "RiaLogging.h"

#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimWellPath.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include <QFile>
#include <QTextStream>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelPerfsFrkExporter::writeToFile( RimFractureModelPlot* plot, const QString& filepath )
{
    RimFractureModel* fractureModel = plot->fractureModel();
    if ( !fractureModel )
    {
        return false;
    }

    RimWellPath* wellPath = fractureModel->wellPath();
    if ( !wellPath )
    {
        return false;
    }

    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    bool isTransverse =
        ( fractureModel->fractureOrientation() == RimFractureModel::FractureOrientation::TRANSVERSE_WELL_PATH ||
          fractureModel->fractureOrientation() == RimFractureModel::FractureOrientation::AZIMUTH );

    appendFractureOrientationToStream( stream, isTransverse );

    // Unit: meter
    double perforationLength = fractureModel->perforationLength();

    double anchorPositionMD = computeMeasuredDepthForPosition( wellPath, fractureModel->anchorPosition() );
    double topMD            = anchorPositionMD - ( perforationLength / 2.0 );
    double bottomMD         = anchorPositionMD + ( perforationLength / 2.0 );

    appendPerforationToStream( stream,
                               1,
                               RiaEclipseUnitTools::meterToFeet( topMD ),
                               RiaEclipseUnitTools::meterToFeet( bottomMD ) );

    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelPerfsFrkExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << "<perfs>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelPerfsFrkExporter::appendFractureOrientationToStream( QTextStream& stream, bool isTransverse )
{
    stream << "<transverse>" << endl << static_cast<int>( isTransverse ) << endl << "</transverse>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelPerfsFrkExporter::appendPerforationToStream( QTextStream& stream, int index, double topMD, double bottomMD )
{
    stream << "<perf frac=\"" << index << "\">" << endl
           << "<topMD>" << endl
           << topMD << endl
           << "</topMD>" << endl
           << "<bottomMD>" << endl
           << bottomMD << endl
           << "</bottomMD>" << endl
           << "</perf>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelPerfsFrkExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</perfs>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifFractureModelPerfsFrkExporter::computeMeasuredDepthForPosition( const RimWellPath* wellPath,
                                                                          const cvf::Vec3d&  position )
{
    const RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();

    const std::vector<double>& mdValuesOfWellPath  = wellPathGeometry->measuredDepths();
    const std::vector<double>& tvdValuesOfWellPath = wellPathGeometry->trueVerticalDepths();
    const double               targetTvd           = -position.z();

    std::vector<double> tvDepthValues;
    size_t              index   = 0;
    bool                isAdded = false;
    for ( size_t i = 0; i < tvdValuesOfWellPath.size(); i++ )
    {
        double currentTvd = tvdValuesOfWellPath[i];
        double prevTvd    = 0.0;
        if ( i > 0 ) prevTvd = tvdValuesOfWellPath[i - 1];

        if ( !isAdded && targetTvd > prevTvd && targetTvd <= currentTvd )
        {
            // Insert the anchor position at correct order in well depths
            index   = i;
            isAdded = true;
            tvDepthValues.push_back( targetTvd );
        }

        tvDepthValues.push_back( currentTvd );
    }

    // Generate MD data by interpolation
    std::vector<double> measuredDepthValues =
        RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfWellPath, tvdValuesOfWellPath, tvDepthValues );

    if ( index < measuredDepthValues.size() )
        return measuredDepthValues[index];
    else
    {
        RiaLogging::error( "Unable to compute measured depth from well path data." );
        return -1.0;
    }
}
