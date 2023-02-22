/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimWellLogCurveInfoTextProvider.h"

#include "RimDepthTrackPlot.h"
#include "RimWellLogCurve.h"

#include "RiuPlotCurve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveInfoTextProvider::curveInfoText( RiuPlotCurve* riuCurve ) const
{
    if ( riuCurve )
    {
        RimWellLogCurve* wlCurve = dynamic_cast<RimWellLogCurve*>( riuCurve->ownerRimCurve() );
        if ( wlCurve )
        {
            return QString( "%1" ).arg( wlCurve->curveName() );
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveInfoTextProvider::additionalText( RiuPlotCurve* curve, int sampleIndex ) const
{
    if ( !curve ) return {};

    std::vector<std::pair<QString, double>> propertyNameValues;

    auto* sourceCurve = curve->ownerRimCurve();
    if ( !sourceCurve ) return {};

    auto annotationCurves = sourceCurve->additionalDataSources();
    for ( auto annotationCurve : annotationCurves )
    {
        RimDepthTrackPlot* depthTrackPlot = nullptr;
        annotationCurve->firstAncestorOfType( depthTrackPlot );
        if ( depthTrackPlot )
        {
            auto [xValue, yValue] = curve->sample( sampleIndex );

            auto depth = depthTrackPlot->depthOrientation() == RiaDefines::Orientation::VERTICAL ? yValue : xValue;

            auto propertyValue = annotationCurve->closestYValueForX( depth );

            // Use template to get as short label as possible. The default curve name will often
            // contain too much and redundant information.
            QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();
            auto    resultName   = annotationCurve->createCurveNameFromTemplate( templateText );

            propertyNameValues.push_back( std::make_pair( resultName, propertyValue ) );
        }
    }

    QString txt;
    for ( const auto& [name, value] : propertyNameValues )
    {
        txt += QString( "%1 : %2\n" ).arg( name ).arg( value );
    }

    return txt;
}
