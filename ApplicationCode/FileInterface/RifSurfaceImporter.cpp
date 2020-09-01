/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RifSurfaceImporter.h"
#include "RigGocadData.h"

#include "cvfAssert.h"
#include "cvfVector3.h"

#include "QStringList"

#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
/// Import vertices and triangle IDs from the first TFACE section in the file
/// Returns vertices with z-value as depth, z is increasing downwards
///
//--------------------------------------------------------------------------------------------------
void RifSurfaceImporter::readGocadFile( const QString& filename, RigGocadData* gocadData )
{
    CVF_ASSERT( gocadData );

    enum class GocadZPositive
    {
        Elevation,
        Depth,
        Unknown
    };

    std::vector<cvf::Vec3d> vertices;
    std::map<int, unsigned> vertexIdToIndex;
    std::vector<unsigned>   trianglesByIds;

    std::vector<QString>            propertyNames;
    std::vector<std::vector<float>> propertyValues;

    {
        std::ifstream stream( filename.toLatin1().data() );

        bool           isInTfaceSection = false;
        GocadZPositive zDir             = GocadZPositive::Unknown;

        while ( stream.good() )
        {
            std::string line;
            std::getline( stream, line );

            std::transform( line.begin(), line.end(), line.begin(), ::toupper );

            std::istringstream lineStream( line );

            std::string firstToken;
            lineStream >> firstToken;

            if ( isInTfaceSection )
            {
                if ( firstToken.compare( "VRTX" ) == 0 )
                {
                    int         vertexId = -1;
                    double      x{std::numeric_limits<double>::infinity()};
                    double      y{std::numeric_limits<double>::infinity()};
                    double      z{std::numeric_limits<double>::infinity()};
                    std::string endVertex;

                    lineStream >> vertexId >> x >> y >> z >> endVertex;

                    if ( vertexId > -1 )
                    {
                        if ( zDir == GocadZPositive::Depth )
                        {
                            z = -z;
                        }

                        vertices.emplace_back( cvf::Vec3d( x, y, z ) );
                        vertexIdToIndex[vertexId] = static_cast<unsigned>( vertices.size() - 1 );
                    }
                }
                else if ( firstToken.compare( "PVRTX" ) == 0 )
                {
                    int    vertexId = -1;
                    double x{std::numeric_limits<double>::infinity()};
                    double y{std::numeric_limits<double>::infinity()};
                    double z{std::numeric_limits<double>::infinity()};

                    lineStream >> vertexId >> x >> y >> z;

                    if ( vertexId > -1 )
                    {
                        if ( zDir == GocadZPositive::Depth ) z = -z;

                        vertices.emplace_back( cvf::Vec3d( x, y, z ) );
                        vertexIdToIndex[vertexId] = static_cast<unsigned>( vertices.size() - 1 );
                    }

                    for ( size_t i = 0; i < propertyNames.size(); i++ )
                    {
                        float value = std::numeric_limits<double>::infinity();

                        lineStream >> value;

                        propertyValues[i].push_back( value );
                    }
                }
                else if ( firstToken.compare( "TRGL" ) == 0 )
                {
                    int id1{-1};
                    int id2{-1};
                    int id3{-1};

                    lineStream >> id1 >> id2 >> id3;

                    if ( id1 >= 0 && id2 >= 0 && id3 >= 0 )
                    {
                        trianglesByIds.emplace_back( static_cast<unsigned int>( id1 ) );
                        trianglesByIds.emplace_back( static_cast<unsigned int>( id2 ) );
                        trianglesByIds.emplace_back( static_cast<unsigned int>( id3 ) );
                    }
                }
                else if ( firstToken.compare( "END" ) == 0 )
                {
                    isInTfaceSection = false;
                }
            }
            else if ( firstToken.compare( "TFACE" ) == 0 )
            {
                isInTfaceSection = true;
            }
            else if ( firstToken.compare( "PROPERTIES" ) == 0 )
            {
                QString qstringLine = QString::fromStdString( line );

                qstringLine.remove( "PROPERTIES" );

                QStringList words = qstringLine.split( " ", QString::SkipEmptyParts );

                for ( auto w : words )
                {
                    propertyNames.push_back( w );
                }

                propertyValues.resize( propertyNames.size() );
            }
            else if ( firstToken.compare( "ZPOSITIVE" ) == 0 )
            {
                std::string secondToken;
                lineStream >> secondToken;

                if ( secondToken == "DEPTH" )
                {
                    zDir = GocadZPositive::Depth;
                }
                else if ( secondToken == "ELEVATION" )
                {
                    zDir = GocadZPositive::Elevation;
                }
            }
        }
    }

    std::vector<unsigned> triangleIndices;
    if ( !trianglesByIds.empty() )
    {
        triangleIndices.reserve( trianglesByIds.size() );

        for ( auto triangle : trianglesByIds )
        {
            auto vertexIndex = vertexIdToIndex[triangle];
            triangleIndices.push_back( vertexIndex );
        }
    }

    if ( gocadData )
    {
        gocadData->setGeometryData( vertices, triangleIndices );
        gocadData->addPropertyData( propertyNames, propertyValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> RifSurfaceImporter::readPetrelFile( const QString& filename )
{
    std::ifstream stream( filename.toLatin1().data() );

    struct SurfacePointData
    {
        int                 i;
        int                 j;
        cvf::Vec3d          point;
        std::vector<double> values;
    };

    std::vector<SurfacePointData> surfaceDataPoints;

    int minI = std::numeric_limits<int>::max();
    int minJ = std::numeric_limits<int>::max();
    int maxI = std::numeric_limits<int>::min();
    int maxJ = std::numeric_limits<int>::min();

    while ( stream.good() )
    {
        std::string line;
        std::getline( stream, line );
        std::istringstream lineStream( line );

        double x( std::numeric_limits<double>::infinity() );
        double y( std::numeric_limits<double>::infinity() );
        double z( std::numeric_limits<double>::infinity() );

        int                 i( -1 ), j( -1 );
        std::vector<double> values;

        // First check if we can read a number

        lineStream >> x;
        if ( lineStream.good() ) // If we can, assume this line is a surface point
        {
            lineStream >> y >> z >> i >> j;

            if ( x != std::numeric_limits<double>::infinity() && y != std::numeric_limits<double>::infinity() &&
                 z != std::numeric_limits<double>::infinity() && i != -1 && j != -1 )
            {
                // Check for extra data
                while ( lineStream.good() )
                {
                    double d;
                    lineStream >> d;
                    if ( lineStream.good() ) values.push_back( d );
                }

                // Add point

                surfaceDataPoints.push_back( {i, j, {x, y, z}, values} );

                minI = std::min( minI, i );
                minJ = std::min( minJ, j );
                maxI = std::max( maxI, i );
                maxJ = std::max( maxJ, j );
            }
        }
        else // Probably a comment line, skip
        {
        }
    }

    // clang-format off
    if ( surfaceDataPoints.empty() 
        || minI == std::numeric_limits<int>::max() 
        || minJ == std::numeric_limits<int>::max() 
        || maxI == std::numeric_limits<int>::min() 
        || maxJ == std::numeric_limits<int>::min() )
    {
        return {};
    }
    // clang-format on

    // Create full size grid matrix

    size_t iCount = maxI - minI + 1;
    size_t jCount = maxJ - minJ + 1;

    if ( iCount < 2 || jCount < 2 )
    {
        return {};
    }

    std::vector<std::vector<unsigned>> indexToPointData;
    indexToPointData.resize( iCount, std::vector<unsigned>( jCount, -1 ) );
    std::vector<cvf::Vec3d> vertices;

    for ( unsigned pIdx = 0; pIdx < surfaceDataPoints.size(); ++pIdx )
    {
        const auto& pointData = surfaceDataPoints[pIdx];

        indexToPointData[pointData.i - minI][pointData.j - minJ] = pIdx;

        vertices.push_back( pointData.point );
        // Todo: Move result values for each point into the
    }

    std::vector<unsigned> triangleIndices;
    if ( indexToPointData.size() < 2 )
    {
        return {};
    }

    for ( size_t iIdx = 0; iIdx < indexToPointData.size() - 1; ++iIdx )
    {
        for ( size_t jIdx = 0; jIdx < indexToPointData[iIdx].size() - 1; ++jIdx )
        {
            generateTriangleIndices( indexToPointData, iIdx, jIdx, triangleIndices );
        }
    }

    return std::make_pair( vertices, triangleIndices );
}

std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> RifSurfaceImporter::readOpenWorksXyzFile( const QString& filename )
{
    std::ifstream stream( filename.toLatin1().data() );

    struct vec3dCompare
    {
        bool operator()( const cvf::Vec3d& lhs, const cvf::Vec3d& rhs ) const { return lhs.length() < rhs.length(); }
    };

    std::vector<cvf::Vec3d> surfacePoints;

    // Mapping normalized vectors to all vectors having their direction
    std::map<cvf::Vec3d, double, vec3dCompare> axesVectorCandidates;
    // Mapping axes vectors to their number of occurrence
    std::map<cvf::Vec3d, unsigned, vec3dCompare> axesVectorCandidatesNum;
    // Mapping axes vectors to their number of neighbored points
    std::map<cvf::Vec3d, unsigned, vec3dCompare> numOfNeighbouredPoints;
    std::vector<cvf::Vec3d>                      vectorChanges;
    std::vector<size_t>                          potentialEndPointIndices;
    // Max values
    cvf::Vec3d maxValues;
    cvf::Vec3d minValues;
    cvf::Vec3d lastVector;

    double epsilon = 0.001;

    auto maybeInsertAxisVectorCandidate = []( const cvf::Vec3d                              vector,
                                              std::map<cvf::Vec3d, double, vec3dCompare>&   axesVectorCandidates,
                                              std::map<cvf::Vec3d, unsigned, vec3dCompare>& axesVectorCandidatesNum,
                                              std::map<cvf::Vec3d, unsigned, vec3dCompare>& numOfNeighbouredPoints ) -> bool {
        double     length           = vector.length();
        cvf::Vec3d normalizedVector = vector.getNormalized();
        if ( axesVectorCandidates.count( normalizedVector ) == 0 )
        {
            axesVectorCandidates.insert( std::pair<cvf::Vec3d, double>( normalizedVector, length ) );
            axesVectorCandidatesNum.insert( std::pair<cvf::Vec3d, unsigned>( normalizedVector, 0 ) );
            axesVectorCandidatesNum[normalizedVector] += 1;
            return true;
        }
        else if ( length < axesVectorCandidates[normalizedVector] )
        {
            axesVectorCandidates[normalizedVector] = length;
        }
        if ( numOfNeighbouredPoints.count( normalizedVector ) == 0 )
        {
            numOfNeighbouredPoints.insert( std::pair<cvf::Vec3d, unsigned>( normalizedVector, 0 ) );
        }
        numOfNeighbouredPoints[normalizedVector] += 1;
        axesVectorCandidatesNum[normalizedVector] += 1;
        return false;
    };

    while ( stream.good() )
    {
        std::string line;
        std::getline( stream, line );
        std::istringstream lineStream( line );

        double x( std::numeric_limits<double>::infinity() );
        double y( std::numeric_limits<double>::infinity() );
        double z( std::numeric_limits<double>::infinity() );

        std::vector<double> values;

        // First check if we can read all numbers
        bool ok = true;
        lineStream >> x;
        ok &= lineStream.good();
        lineStream >> y;
        ok &= lineStream.good();
        lineStream >> z;
        ok &= lineStream.good() || lineStream.eof();
        if ( ok ) // If we can, assume this line is a surface point
        {
            if ( x != std::numeric_limits<double>::infinity() && y != std::numeric_limits<double>::infinity() &&
                 z != std::numeric_limits<double>::infinity() )
            {
                // Check for extra data
                while ( lineStream.good() )
                {
                    double d;
                    lineStream >> d;
                    if ( lineStream.good() ) values.push_back( d );
                }

                // Add point

                surfacePoints.push_back( {x, y, z} );

                if ( surfacePoints.size() == 1 )
                {
                    maxValues = {x, y, z};
                    minValues = {x, y, z};
                }
                else
                {
                    if ( x > maxValues.x() ) maxValues.ptr()[0] = x;
                    if ( y > maxValues.y() ) maxValues.ptr()[1] = y;
                    if ( z > maxValues.z() ) maxValues.ptr()[2] = z;
                    if ( x < minValues.x() ) minValues.ptr()[0] = x;
                    if ( y < minValues.y() ) minValues.ptr()[1] = y;
                    if ( z < minValues.z() ) minValues.ptr()[2] = z;
                }

                if ( surfacePoints.size() > 1 )
                {
                    cvf::Vec3d pointToPointVector = surfacePoints.back() - surfacePoints[surfacePoints.size() - 2];
                    bool       inserted           = maybeInsertAxisVectorCandidate( pointToPointVector,
                                                                    axesVectorCandidates,
                                                                    axesVectorCandidatesNum,
                                                                    numOfNeighbouredPoints );

                    if ( lastVector.dot( pointToPointVector ) < 0.5 )
                    {
                        for ( size_t i = 0; i < vectorChanges.size(); i++ )
                        {
                            cvf::Vec3d vector = surfacePoints.back() - vectorChanges[i];
                            inserted          = maybeInsertAxisVectorCandidate( vector,
                                                                       axesVectorCandidates,
                                                                       axesVectorCandidatesNum,
                                                                       numOfNeighbouredPoints );
                        }
                        vectorChanges.push_back( surfacePoints.back() );
                        if ( !std::count( potentialEndPointIndices.begin(),
                                          potentialEndPointIndices.end(),
                                          surfacePoints.size() - 1 ) )
                            potentialEndPointIndices.push_back( surfacePoints.size() - 1 );

                        if ( !std::count( potentialEndPointIndices.begin(),
                                          potentialEndPointIndices.end(),
                                          surfacePoints.size() - 2 ) )
                            potentialEndPointIndices.push_back( surfacePoints.size() - 2 );
                    }

                    lastVector = pointToPointVector.getNormalized();
                }
            }
        }
        else // Probably a comment line, skip
        {
        }
    }

    // Determine axes vectors
    std::vector<std::pair<cvf::Vec3d, unsigned>> pairs;
    for ( auto itr = axesVectorCandidatesNum.begin(); itr != axesVectorCandidatesNum.end(); ++itr )
    {
        pairs.push_back( *itr );
    }

    sort( pairs.begin(), pairs.end(), [=]( std::pair<cvf::Vec3d, unsigned>& a, std::pair<cvf::Vec3d, unsigned>& b ) {
        return a.second > b.second;
    } );

    size_t primaryIndex = 0;
    if ( numOfNeighbouredPoints[pairs[0].first] > numOfNeighbouredPoints[pairs[1].first] )
    {
        primaryIndex = 0;
    }
    else
    {
        primaryIndex = 1;
    }

    cvf::Vec3d primaryAxisVector   = pairs[primaryIndex].first * axesVectorCandidates[pairs[primaryIndex].first];
    cvf::Vec3d normPrimAxisVector  = primaryAxisVector.getNormalized();
    cvf::Vec3d secondaryAxisVector = pairs[1 - primaryIndex].first * axesVectorCandidates[pairs[1 - primaryIndex].first];
    cvf::Vec3d normSecAxisVector   = secondaryAxisVector.getNormalized();

    // Find starting and end point
    cvf::Vec3d startPoint = surfacePoints.front();
    cvf::Vec3d endPoint   = surfacePoints.back();

    for ( size_t k = 0; k < potentialEndPointIndices.size(); k++ )
    {
        cvf::Vec3d currentPoint = surfacePoints[potentialEndPointIndices[k]];
        if ( currentPoint == startPoint ) continue;

        double scalarProjection = secondaryAxisVector.dot( currentPoint - startPoint ) /
                                  ( ( currentPoint - startPoint ).length() * secondaryAxisVector.length() );
        if ( scalarProjection <= 0 )
        {
            startPoint = startPoint + secondaryAxisVector.dot( currentPoint - startPoint ) * normSecAxisVector /
                                          secondaryAxisVector.length();
        }

        scalarProjection = primaryAxisVector.dot( currentPoint - startPoint ) /
                           ( ( currentPoint - startPoint ).length() * primaryAxisVector.length() );
        if ( scalarProjection <= 0 )
        {
            startPoint = startPoint + primaryAxisVector.dot( currentPoint - startPoint ) * normPrimAxisVector /
                                          primaryAxisVector.length();
        }

        scalarProjection = secondaryAxisVector.dot( endPoint - currentPoint ) /
                           ( ( endPoint - currentPoint ).length() * secondaryAxisVector.length() );
        if ( scalarProjection <= 0 )
        {
            endPoint = endPoint + secondaryAxisVector.dot( endPoint - currentPoint ) * normSecAxisVector /
                                      secondaryAxisVector.length();
        }

        scalarProjection = primaryAxisVector.dot( endPoint - currentPoint ) /
                           ( ( endPoint - currentPoint ).length() * primaryAxisVector.length() );
        if ( scalarProjection <= 0 )
        {
            endPoint = endPoint + primaryAxisVector.dot( endPoint - currentPoint ) * normPrimAxisVector /
                                      primaryAxisVector.length();
        }
    }

    unsigned   maxRow       = 1;
    unsigned   maxColumn    = 1;
    cvf::Vec3d currentPoint = startPoint;

    while ( !vectorFuzzyCompare( ( endPoint - currentPoint ).getNormalized(), primaryAxisVector.getNormalized(), epsilon ) )
    {
        maxColumn++;
        currentPoint += secondaryAxisVector;
    }
    while ( !vectorFuzzyCompare( endPoint, currentPoint, epsilon ) )
    {
        maxRow++;
        currentPoint += primaryAxisVector;
    }

    unsigned                           row    = 0;
    unsigned                           column = 0;
    size_t                             index  = 0;
    std::vector<std::vector<unsigned>> indexToPointData;
    indexToPointData.resize( maxColumn, std::vector<unsigned>( maxRow, -1 ) );
    for ( unsigned column = 0; column < maxColumn; column++ )
    {
        currentPoint = startPoint + secondaryAxisVector * column;
        for ( unsigned row = 0; row < maxRow; row++ )
        {
            if ( vectorFuzzyCompare( surfacePoints[index], currentPoint, epsilon ) )
            {
                indexToPointData[column][row] = static_cast<unsigned>( index );
                index                         = std::min( index + 1, surfacePoints.size() - 1 );
            }
            currentPoint += primaryAxisVector;
        }
    }

    std::vector<unsigned> triangleIndices;
    if ( indexToPointData.size() < 2 )
    {
        return {};
    }

    for ( size_t iIdx = 0; iIdx < indexToPointData.size() - 1; ++iIdx )
    {
        for ( size_t jIdx = 0; jIdx < indexToPointData[iIdx].size() - 1; ++jIdx )
        {
            generateTriangleIndices( indexToPointData, iIdx, jIdx, triangleIndices );
        }
    }

    return std::make_pair( surfacePoints, triangleIndices );
}

bool RifSurfaceImporter::generateTriangleIndices( const std::vector<std::vector<unsigned>>& indexToPointData,
                                                  const size_t&                             i,
                                                  const size_t&                             j,
                                                  std::vector<unsigned>&                    triangleIndices )
{
    unsigned q1 = indexToPointData[i + 0][j + 0];
    unsigned q2 = indexToPointData[i + 0][j + 1];
    unsigned q3 = indexToPointData[i + 1][j + 0];
    unsigned q4 = indexToPointData[i + 1][j + 1];

    if ( q1 != ( (unsigned)-1 ) && q2 != ( (unsigned)-1 ) && q4 != ( (unsigned)-1 ) && q1 != ( (unsigned)-1 ) &&
         q4 != ( (unsigned)-1 ) && q3 != ( (unsigned)-1 ) )
    {
        triangleIndices.push_back( q1 );
        triangleIndices.push_back( q2 );
        triangleIndices.push_back( q4 );
        triangleIndices.push_back( q1 );
        triangleIndices.push_back( q4 );
        triangleIndices.push_back( q3 );
        return true;
    }
    else
    {
        return false;
    }
}

bool RifSurfaceImporter::vectorFuzzyCompare( const cvf::Vec3d& vector1, const cvf::Vec3d& vector2, double epsilon, double maxDiff )
{
    auto AlmostEqualRelativeAndAbs = [=]( double A, double B, double maxDiff, double maxRelDiff ) -> bool {
        // Check if the numbers are really close -- needed
        // when comparing numbers near zero.
        double diff = fabs( A - B );
        // if ( diff <= maxDiff ) return true;

        A             = fabs( A );
        B             = fabs( B );
        float largest = ( B > A ) ? B : A;

        if ( diff <= largest * maxRelDiff ) return true;
        return false;
    };
    return ( AlmostEqualRelativeAndAbs( vector1.x(), vector2.x(), maxDiff, epsilon ) &&
             AlmostEqualRelativeAndAbs( vector1.y(), vector2.y(), maxDiff, epsilon ) &&
             AlmostEqualRelativeAndAbs( vector1.z(), vector2.z(), maxDiff, epsilon ) );
}

cvf::Vec3d RifSurfaceImporter::absVector( const cvf::Vec3d& vector )
{
    return cvf::Vec3d( abs( vector.x() ), abs( vector.y() ), abs( vector.z() ) );
}
