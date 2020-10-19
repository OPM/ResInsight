/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimFractureModelLayerCalculator.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelLayerCalculator.h"
#include "RimFractureModelTemplate.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelLayerCalculator::RimFractureModelLayerCalculator( RimFractureModelCalculator* fractureModelCalculator )
    : m_fractureModelCalculator( fractureModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelLayerCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    return curveProperty == RiaDefines::CurveProperty::LAYERS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelLayerCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                 const RimFractureModel*   fractureModel,
                                                 int                       timeStep,
                                                 std::vector<double>&      values,
                                                 std::vector<double>&      measuredDepthValues,
                                                 std::vector<double>&      tvDepthValues,
                                                 double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = fractureModel->eclipseCase();
    if ( !eclipseCase )
    {
        return false;
    }

    RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(),
                                             fractureModel->thicknessDirectionWellPath()->wellPathGeometry(),
                                             "fracture model" );

    rkbDiff = eclExtractor.wellPathData()->rkbDiff();

    // Extract formation data
    cvf::ref<RigResultAccessor> formationResultAccessor =
        RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(),
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                                                    RiaDefines::activeFormationNamesResultName() ) );
    if ( !formationResultAccessor.notNull() )
    {
        RiaLogging::error( QString( "No formation result found." ) );
        return false;
    }

    CurveSamplingPointData curveData =
        RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

    double overburdenHeight = fractureModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        RimWellLogTrack::addOverburden( formationNamesVector, curveData, overburdenHeight );
    }

    double underburdenHeight = fractureModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        RimWellLogTrack::addUnderburden( formationNamesVector, curveData, underburdenHeight );
    }

    measuredDepthValues = curveData.md;
    tvDepthValues       = curveData.tvd;

    // Extract facies data
    std::vector<double> faciesValues =
        m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::FACIES, timeStep );
    if ( faciesValues.empty() )
    {
        RiaLogging::error( QString( "Empty facies data found for layer curve." ) );
        return false;
    }

    std::vector<double> netToGrossValues =
        m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );
    if ( netToGrossValues.empty() )
    {
        RiaLogging::warning( QString( "Empty net-to-gross data found for layer curve." ) );
    }

    CAF_ASSERT( faciesValues.size() == curveData.data.size() );

    values.resize( faciesValues.size() );

    int    layerNo            = 0;
    double previousFormation  = -1.0;
    double previousFacies     = -1.0;
    double previousNetToGross = -1.0;
    double netToGrossCutoff   = 1.0;
    if ( fractureModel->fractureModelTemplate() && fractureModel->fractureModelTemplate()->nonNetLayers() )
    {
        netToGrossCutoff = fractureModel->fractureModelTemplate()->nonNetLayers()->cutOff();
    }
    bool useNetToGross = !netToGrossValues.empty();
    for ( size_t i = 0; i < faciesValues.size(); i++ )
    {
        if ( previousFormation != curveData.data[i] || previousFacies != faciesValues[i] ||
             ( useNetToGross && netToGrossValues[i] <= netToGrossCutoff && previousNetToGross != netToGrossValues[i] ) )
        {
            layerNo++;
        }

        values[i]         = layerNo;
        previousFormation = curveData.data[i];
        previousFacies    = faciesValues[i];
        if ( useNetToGross )
        {
            previousNetToGross = netToGrossValues[i];
        }
    }

    return true;
}
