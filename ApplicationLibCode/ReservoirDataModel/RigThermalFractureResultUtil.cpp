/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RigThermalFractureResultUtil.h"

#include "RiaLogging.h"

#include "RiaWeightedMeanCalculator.h"
#include "RigCellGeometryTools.h"
#include "RigConvexHull.h"
#include "RigFractureGrid.h"
#include "RigStatisticsMath.h"
#include "RigThermalFractureDefinition.h"

#include "cafAssert.h"
#include "cvfBoundingBox.h"

#include <cmath>

int numSamplesX = 60;
int numSamplesY = 60;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigThermalFractureResultUtil::RigThermalFractureResultUtil()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigThermalFractureResultUtil::~RigThermalFractureResultUtil()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>
    RigThermalFractureResultUtil::getDataAtTimeIndex( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                      const QString&                                      resultName,
                                                      const QString&                                      unitName,
                                                      size_t timeStepIndex )
{
    int propertyIndex = fractureDefinition->getPropertyIndex( resultName );

    std::vector<cvf::Vec3d> relativePos = getRelativeCoordinates( fractureDefinition, timeStepIndex );

    // Create bounding box
    cvf::BoundingBox boundingBox;
    for ( auto p : relativePos )
        boundingBox.add( p );

    boundingBox.expand( 1.0 );

    // Generate a uniform mesh
    auto [xCoordsAtNodes, yCoordsAtNodes] = generateUniformMesh( boundingBox, numSamplesX, numSamplesY );

    // Find center points
    std::vector<double> xCoords;
    for ( int i = 0; i < static_cast<int>( xCoordsAtNodes.size() ) - 1; i++ )
        xCoords.push_back( ( xCoordsAtNodes[i] + xCoordsAtNodes[i + 1] ) / 2 );
    std::vector<double> depthCoords;
    for ( int i = 0; i < static_cast<int>( yCoordsAtNodes.size() ) - 1; i++ )
        depthCoords.push_back( ( yCoordsAtNodes[i] + yCoordsAtNodes[i + 1] ) / 2 );

    std::vector<std::vector<double>> vec;

    // Fill with invalid value
    for ( int i = 0; i < numSamplesY; i++ )
    {
        std::vector<double> junk( numSamplesX, std::numeric_limits<double>::infinity() );
        vec.push_back( junk );
    }

    // Find the boundary of the fracture (i.e. convex hull around the points)
    std::vector<cvf::Vec3d> fractureBoundary = RigConvexHull::compute2d( relativePos );
    fractureBoundary.push_back( fractureBoundary.front() );

    for ( int i = 0; i < static_cast<int>( xCoords.size() ) - 1; i++ )
    {
        for ( int j = 0; j < static_cast<int>( depthCoords.size() ) - 1; j++ )
        {
            cvf::BoundingBox bb;
            bb.add( cvf::Vec3d( xCoords[i], depthCoords[j], 0.0 ) );
            bb.add( cvf::Vec3d( xCoords[i + 1], depthCoords[j], 0.0 ) );
            bb.add( cvf::Vec3d( xCoords[i + 1], depthCoords[j + 1], 0.0 ) );
            bb.add( cvf::Vec3d( xCoords[i], depthCoords[j + 1], 0.0 ) );

            if ( RigCellGeometryTools::pointInsidePolygon2D( bb.center(), fractureBoundary ) )
            {
                double value =
                    interpolateProperty( bb.center(), relativePos, fractureDefinition, propertyIndex, timeStepIndex );
                vec[j][i] = value;
            }
        }
    }

    return vec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureResultUtil::createFractureTriangleGeometry(
    std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
    int                                                 activeTimeStepIndex,
    double                                              xScaleFactor,
    double                                              yScaleFactor,
    double                                              wellPathIntersectionAtFractureDepth,
    std::vector<cvf::Vec3f>*                            vertices,
    std::vector<cvf::uint>*                             triangleIndices )
{
    // Convert to coordinates relative to center node
    std::vector<cvf::Vec3d> points = getRelativeCoordinates( fractureDefinition, activeTimeStepIndex );

    // Create bounding box
    cvf::BoundingBox boundingBox;
    for ( auto p : points )
        boundingBox.add( p );

    boundingBox.expand( 1.0 );

    // Generate a uniform mesh
    auto [xCoords, depthCoords] = generateUniformMesh( boundingBox, numSamplesX, numSamplesY );

    // Code adapted from RigStimPlanFractureDefinition
    std::vector<double> adjustedYs = depthCoords;
    cvf::uint           lenXcoords = static_cast<cvf::uint>( xCoords.size() );

    for ( cvf::uint k = 0; k < adjustedYs.size(); k++ )
    {
        for ( cvf::uint i = 0; i < lenXcoords; i++ )
        {
            cvf::Vec3f node = cvf::Vec3f( xCoords[i], adjustedYs[k], 0 );
            vertices->push_back( node );

            if ( i < lenXcoords - 1 && k < adjustedYs.size() - 1 )
            {
                double THRESHOLD_VALUE = 1e-5;

                if ( xCoords[i] < THRESHOLD_VALUE )
                {
                    // Upper triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    // Lower triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                }
                else
                {
                    // Upper triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                    // Lower triangle
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RigThermalFractureResultUtil::fractureGridResults( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                       const QString&                                      resultName,
                                                       const QString&                                      unitName,
                                                       size_t timeStepIndex )
{
    std::vector<double>                     fractureGridResults;
    const std::vector<std::vector<double>>& resultValuesAtTimeStep =
        getDataAtTimeIndex( fractureDefinition, resultName, unitName, timeStepIndex );

    for ( int i = 0; i < static_cast<int>( numSamplesX ) - 2; i++ )
    {
        for ( int j = 0; j < static_cast<int>( numSamplesY ) - 2; j++ )
        {
            if ( j + 1 < static_cast<int>( resultValuesAtTimeStep.size() ) &&
                 i + 1 < static_cast<int>( resultValuesAtTimeStep[j + 1].size() ) )
            {
                fractureGridResults.push_back( resultValuesAtTimeStep[j + 1][i + 1] );
            }
            else
            {
                fractureGridResults.push_back( HUGE_VAL );
            }
        }
    }

    return fractureGridResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::cref<RigFractureGrid>
    RigThermalFractureResultUtil::createFractureGrid( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                      const QString&                                      resultName,
                                                      int                           activeTimeStepIndex,
                                                      double                        xScaleFactor,
                                                      double                        yScaleFactor,
                                                      double                        wellPathIntersectionAtFractureDepth,
                                                      RiaDefines::EclipseUnitSystem requiredUnitSet )
{
    // Convert to coordinates relative to center node
    std::vector<cvf::Vec3d> points = getRelativeCoordinates( fractureDefinition, activeTimeStepIndex );

    QString conductivityUnitTextOnFile;

    std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = fractureDefinition->getPropertyNamesUnits();
    for ( auto properyNameUnit : propertyNamesUnitsOnFile )
    {
        if ( resultName == properyNameUnit.first )
        {
            conductivityUnitTextOnFile = properyNameUnit.second;
        }
    }

    CAF_ASSERT( !conductivityUnitTextOnFile.isEmpty() );
    if ( conductivityUnitTextOnFile.isEmpty() )
    {
        RiaLogging::error( "Did not find unit for conductivity on file" );
        return nullptr;
    }

    std::vector<std::vector<double>> conductivityValues =
        getDataAtTimeIndex( fractureDefinition, resultName, conductivityUnitTextOnFile, activeTimeStepIndex );
    if ( conductivityValues.empty() )
    {
        RiaLogging::error( QString( "No conductivity values found for result: %1" ).arg( resultName ) );
        return nullptr;
    }

    // Create bounding box
    cvf::BoundingBox boundingBox;
    for ( auto p : points )
        boundingBox.add( p );

    boundingBox.expand( 1.0 );

    // Generate a uniform mesh
    auto [xCoordsAtNodes, yCoordsAtNodes] = generateUniformMesh( boundingBox, numSamplesX, numSamplesY );

    // Find center points
    std::vector<double> xCoords;
    for ( int i = 0; i < static_cast<int>( xCoordsAtNodes.size() ) - 1; i++ )
        xCoords.push_back( ( xCoordsAtNodes[i] + xCoordsAtNodes[i + 1] ) / 2 );
    std::vector<double> depthCoords;
    for ( int i = 0; i < static_cast<int>( yCoordsAtNodes.size() ) - 1; i++ )
        depthCoords.push_back( ( yCoordsAtNodes[i] + yCoordsAtNodes[i + 1] ) / 2 );

    std::pair<size_t, size_t>    wellCenterStimPlanCellIJ = std::make_pair( 0, 0 );
    std::vector<RigFractureCell> stimPlanCells;
    for ( int i = 0; i < static_cast<int>( xCoords.size() ) - 1; i++ )
    {
        for ( int j = 0; j < static_cast<int>( depthCoords.size() ) - 1; j++ )
        {
            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back( cvf::Vec3d( xCoords[i], depthCoords[j], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i + 1], depthCoords[j], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i + 1], depthCoords[j + 1], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i], depthCoords[j + 1], 0.0 ) );

            RigFractureCell stimPlanCell( cellPolygon, i, j );
            if ( !conductivityValues.empty() ) // Assuming vector to be of correct length, or no values
            {
                stimPlanCell.setConductivityValue( conductivityValues[j + 1][i + 1] );
            }
            else
            {
                stimPlanCell.setConductivityValue( cvf::UNDEFINED_DOUBLE );
            }

            // The well path is intersecting the fracture at origo in the fracture coordinate system
            // Find the Stimplan cell where the well path is intersecting

            if ( cellPolygon[0].x() <= 0.0 && cellPolygon[1].x() >= 0.0 )
            {
                if ( cellPolygon[1].y() >= 0.0 && cellPolygon[2].y() <= 0.0 )
                {
                    wellCenterStimPlanCellIJ = std::make_pair( stimPlanCell.getI(), stimPlanCell.getJ() );
                }
            }

            stimPlanCells.push_back( stimPlanCell );
        }
    }

    cvf::ref<RigFractureGrid> fractureGrid = new RigFractureGrid;
    fractureGrid->setFractureCells( stimPlanCells );
    fractureGrid->setWellCenterFractureCellIJ( wellCenterStimPlanCellIJ );
    fractureGrid->setICellCount( numSamplesX - 2 );
    fractureGrid->setJCellCount( numSamplesY - 2 );
    fractureGrid->ensureCellSearchTreeIsBuilt();

    return cvf::cref<RigFractureGrid>( fractureGrid.p() );
}

//--------------------------------------------------------------------------------------------------
/// TODO: adapted from RimEnsembleFractureStatistics. Maybe extract?
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>>
    RigThermalFractureResultUtil::generateUniformMesh( const cvf::BoundingBox& bb, int numSamplesX, int numSamplesY )
{
    CAF_ASSERT( numSamplesX > 0 );
    CAF_ASSERT( numSamplesY > 0 );

    double minX = bb.min().x();
    double maxX = bb.max().x();

    double minY = bb.min().y();
    double maxY = bb.max().y();

    std::vector<double> gridXs;
    double              sampleDistanceX = linearSampling( minX, maxX, numSamplesX, gridXs );

    std::vector<double> gridYs;
    double              sampleDistanceY = linearSampling( minY, maxY, numSamplesY, gridYs );

    RiaLogging::info( QString( "Uniform Mesh. Output size: %1x%2. Sampling Distance X = %3 Sampling Distance Y = %4" )
                          .arg( numSamplesX )
                          .arg( numSamplesY )
                          .arg( sampleDistanceX )
                          .arg( sampleDistanceY ) );

    return std::make_pair( gridXs, gridYs );
}

//--------------------------------------------------------------------------------------------------
/// TODO: duplicated from RimEnsembleFractureStatistics. Extract to util.
//--------------------------------------------------------------------------------------------------
double RigThermalFractureResultUtil::linearSampling( double               minValue,
                                                     double               maxValue,
                                                     int                  numSamples,
                                                     std::vector<double>& samples )
{
    CAF_ASSERT( numSamples > 0 );
    double sampleDistance = ( maxValue - minValue ) / numSamples;

    for ( int s = 0; s < numSamples; s++ )
    {
        double pos = minValue + s * sampleDistance + sampleDistance * 0.5;
        samples.push_back( pos );
    }

    return sampleDistance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureResultUtil::appendDataToResultStatistics( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                                 const QString&     resultName,
                                                                 const QString&     unit,
                                                                 MinMaxAccumulator& minMaxAccumulator,
                                                                 PosNegAccumulator& posNegAccumulator )
{
    int propertyIndex = fractureDefinition->getPropertyIndex( resultName );

    int maxTs = static_cast<int>( fractureDefinition->timeSteps().size() );
    for ( int ts = 0; ts < maxTs; ts++ )
    {
        for ( int nodeIndex = 0; nodeIndex < static_cast<int>( fractureDefinition->numNodes() ); nodeIndex++ )
        {
            double value = fractureDefinition->getPropertyValue( propertyIndex, nodeIndex, ts );
            minMaxAccumulator.addValue( value );
            posNegAccumulator.addValue( value );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Taken from OverlayNavigationCube::computeNewUpVector
/// Consider move to geometry util class
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RigThermalFractureResultUtil::rotationMatrixBetweenVectors( const cvf::Vec3d& v1, const cvf::Vec3d& v2 )
{
    using namespace cvf;

    Vec3d rotAxis = v1 ^ v2;
    rotAxis.normalize();

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp( v1 * v2, -1.0, 1.0 );
    const double angle      = Math::acos( dotProduct );
    Mat4d        rotMat     = Mat4d::fromRotation( rotAxis, angle );
    return rotMat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>
    RigThermalFractureResultUtil::getRelativeCoordinates( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                          size_t timeStepIndex )
{
    std::vector<cvf::Vec3d> relativePos = fractureDefinition->relativeCoordinates( static_cast<int>( timeStepIndex ) );
    CAF_ASSERT( relativePos.size() == fractureDefinition->numNodes() );

    cvf::Plane plane;
    plane.setFromPoints( relativePos[0], relativePos[1], relativePos[2] );

    cvf::Vec3d planeNormal = plane.normal().getNormalized();
    RiaLogging::info(
        QString( "Plane normal: [%1 %2 %3]" ).arg( planeNormal.x() ).arg( planeNormal.y() ).arg( planeNormal.z() ) );
    auto rotMat = rotationMatrixBetweenVectors( planeNormal, cvf::Vec3d::Z_AXIS.getNormalized() );

    for ( auto& r : relativePos )
    {
        r.transformVector( rotMat );
        r.z() = 0.0;
    }

    return relativePos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureResultUtil::interpolateProperty( const cvf::Vec3d&                                   position,
                                                          const std::vector<cvf::Vec3d>&                      points,
                                                          std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                          int    propertyIndex,
                                                          size_t timeStepIndex )
{
    // Compute the distance to the other points
    std::vector<std::pair<double, int>> distances;

    int index = 0;
    for ( auto p : points )
    {
        double distance = position.pointDistance( p );
        distances.push_back( std::make_pair( distance, index ) );
        index++;
    }

    // Sort by distance
    std::sort( distances.begin(), distances.end() );

    // Create distance-weighthed mean of first few points
    size_t                            numPoints = 3;
    RiaWeightedMeanCalculator<double> calc;
    for ( size_t i = 0; i < numPoints; i++ )
    {
        auto [distance, nodeIndex] = distances[i];
        double value = fractureDefinition->getPropertyValue( propertyIndex, nodeIndex, static_cast<int>( timeStepIndex ) );
        calc.addValueAndWeight( value, distance );
    }

    return calc.weightedMean();
}
