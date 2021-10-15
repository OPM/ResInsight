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
#include "RiaStdStringTools.h"
#include "RigGocadData.h"

#include "cafProgressInfo.h"

#include "cvfAssert.h"
#include "cvfGeometryTools.h"
#include "cvfVector3.h"

#include <QFile>
#include <QStringList>

#include <cmath>
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
        std::stringstream stream;
        {
            // Read the file content into a stringstream to avoid expensive file operations
            std::ifstream t( filename.toLatin1().data() );
            stream << t.rdbuf();
        }

        bool           isInTfaceSection = false;
        GocadZPositive zDir             = GocadZPositive::Unknown;

        int    vertexId = -1;
        double x        = 0.0;
        double y        = 0.0;
        double z        = 0.0;

        int id1 = -1;
        int id2 = -1;
        int id3 = -1;

        while ( stream.good() )
        {
            std::string line;
            std::getline( stream, line );

            auto tokens = RiaStdStringTools::splitString( line, ' ' );

            std::string firstToken;
            if ( !tokens.empty() ) firstToken = tokens.front();

            if ( isInTfaceSection )
            {
                if ( firstToken.compare( "VRTX" ) == 0 )
                {
                    if ( tokens.size() > 4 )
                    {
                        RiaStdStringTools::toIntFast( tokens[1], vertexId );
                        RiaStdStringTools::toDoubleFast( tokens[2], x );
                        RiaStdStringTools::toDoubleFast( tokens[3], y );
                        RiaStdStringTools::toDoubleFast( tokens[4], z );

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
                }
                else if ( firstToken.compare( "PVRTX" ) == 0 )
                {
                    if ( tokens.size() > 4 )
                    {
                        RiaStdStringTools::toIntFast( tokens[1], vertexId );
                        RiaStdStringTools::toDoubleFast( tokens[2], x );
                        RiaStdStringTools::toDoubleFast( tokens[3], y );
                        RiaStdStringTools::toDoubleFast( tokens[4], z );

                        if ( vertexId > -1 )
                        {
                            if ( zDir == GocadZPositive::Depth ) z = -z;

                            vertices.emplace_back( cvf::Vec3d( x, y, z ) );
                            vertexIdToIndex[vertexId] = static_cast<unsigned>( vertices.size() - 1 );

                            double value = std::numeric_limits<double>::infinity();
                            for ( size_t i = 0; i < propertyNames.size(); i++ )
                            {
                                auto tokenIndex = 5 + i;
                                if ( tokenIndex < tokens.size() )
                                    RiaStdStringTools::toDoubleFast( tokens[tokenIndex], value );

                                propertyValues[i].push_back( static_cast<float>( value ) );
                            }
                        }
                    }
                }
                else if ( firstToken.compare( "TRGL" ) == 0 )
                {
                    if ( tokens.size() > 3 )
                    {
                        RiaStdStringTools::toIntFast( tokens[1], id1 );
                        RiaStdStringTools::toIntFast( tokens[2], id2 );
                        RiaStdStringTools::toIntFast( tokens[3], id3 );

                        if ( id1 >= 0 && id2 >= 0 && id3 >= 0 )
                        {
                            trianglesByIds.emplace_back( static_cast<unsigned int>( id1 ) );
                            trianglesByIds.emplace_back( static_cast<unsigned int>( id2 ) );
                            trianglesByIds.emplace_back( static_cast<unsigned int>( id3 ) );
                        }
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
                for ( size_t i = 1; i < tokens.size(); i++ )
                {
                    propertyNames.push_back( QString::fromStdString( tokens[i] ) );
                }

                propertyValues.resize( propertyNames.size() );
            }
            else if ( firstToken.compare( "ZPOSITIVE" ) == 0 )
            {
                std::string secondToken;

                if ( tokens.size() > 1 ) secondToken = RiaStdStringTools::toUpper( tokens[1] );
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

                surfaceDataPoints.push_back( { i, j, { x, y, z }, values } );

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

    size_t iCount = static_cast<size_t>( maxI ) - static_cast<size_t>( minI ) + 1;
    size_t jCount = static_cast<size_t>( maxJ ) - static_cast<size_t>( minJ ) + 1;

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

        indexToPointData[static_cast<size_t>( pointData.i ) - minI][static_cast<size_t>( pointData.j ) - minJ] = pIdx;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>>
    RifSurfaceImporter::readOpenWorksXyzFile( const QString& filename, double preferredPointDistance )
{
    size_t estimatedPointCount = 0;
    {
        QFile  f( filename );
        size_t fileSizeInBytes = size_t( f.size() );

        size_t estimatedBytesPerLine = 40; // Test file has 4.5M lines, file size is 220 MB, one point per line

        if ( fileSizeInBytes > estimatedBytesPerLine * 50 )
        {
            estimatedPointCount = fileSizeInBytes / estimatedBytesPerLine;
        }
    }

    // Note: the assumption is that the grid will always be in the x-y plane.
    std::ifstream stream( filename.toLatin1().data() );

    caf::ProgressInfo progInfo( estimatedPointCount, "Importing Surface Data" );

    struct vec2dCompare
    {
        bool operator()( const cvf::Vec2d& lhs, const cvf::Vec2d& rhs ) const { return lhs.length() < rhs.length(); }
    };

    std::vector<cvf::Vec3d> surfacePoints;

    // Mapping normalized vectors to all vectors having their direction
    std::map<cvf::Vec2d, double, vec2dCompare> axesVectorCandidates;
    // Mapping axes vectors to their number of occurrence
    std::map<cvf::Vec2d, unsigned, vec2dCompare> axesVectorCandidatesNum;

    double epsilon = 0.01;

    // Converts a 3d vector to a 2d vector
    auto to2d = []( const cvf::Vec3d vector ) -> cvf::Vec2d { return cvf::Vec2d( vector.x(), vector.y() ); };
    auto to3d = []( const cvf::Vec2d vector ) -> cvf::Vec3d { return cvf::Vec3d( vector.x(), vector.y(), 0.0 ); };

    // Checks if the given vector is a possible new candidate for an axis vector and adds it to the given list
    // of axesVectorCandidates. Also increases the number of occurrences of vector candidates.
    auto maybeInsertAxisVectorCandidate =
        [epsilon]( const cvf::Vec2d                              vector,
                   std::map<cvf::Vec2d, double, vec2dCompare>&   axesVectorCandidates,
                   std::map<cvf::Vec2d, unsigned, vec2dCompare>& axesVectorCandidatesNum ) -> bool {
        double     length           = vector.length();
        cvf::Vec2d normalizedVector = vector.getNormalized();
        for ( std::map<cvf::Vec2d, double, vec2dCompare>::iterator iter = axesVectorCandidates.begin();
              iter != axesVectorCandidates.end();
              ++iter )
        {
            if ( vectorFuzzyCompare( iter->first, normalizedVector, 0.1 ) )
            {
                normalizedVector = iter->first;
                break;
            }
        }
        if ( axesVectorCandidates.count( normalizedVector ) == 0 )
        {
            axesVectorCandidates.insert( std::pair<cvf::Vec2d, double>( normalizedVector, length ) );
            axesVectorCandidatesNum.insert( std::pair<cvf::Vec2d, unsigned>( normalizedVector, 0 ) );
            axesVectorCandidatesNum[normalizedVector] += 1;
            return true;
        }
        else if ( length < axesVectorCandidates[normalizedVector] )
        {
            axesVectorCandidates[normalizedVector] = length;
        }
        axesVectorCandidatesNum[normalizedVector] += 1;
        return false;
    };

    size_t lineIndex = 0;

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

                // Z should be given in negative values, switch sign if positive
                if ( z > 0.0 ) z = -z;

                // Add point
                surfacePoints.push_back( { x, y, z } );

                if ( surfacePoints.size() > 1 )
                {
                    cvf::Vec3d pointToPointVector = surfacePoints.back() - surfacePoints[surfacePoints.size() - 2];
                    maybeInsertAxisVectorCandidate( to2d( pointToPointVector ),
                                                    axesVectorCandidates,
                                                    axesVectorCandidatesNum );
                }
            }
            else // Probably a comment line, skip
            {
            }
        }

        lineIndex++;

        if ( estimatedPointCount < 100 || lineIndex % ( estimatedPointCount / 100 ) == 0 )
        {
            progInfo.setProgress( std::min( lineIndex, estimatedPointCount ) );
        }
    }

    // Determine axes vectors
    std::vector<std::pair<cvf::Vec2d, unsigned>> pairs;
    for ( auto itr = axesVectorCandidatesNum.begin(); itr != axesVectorCandidatesNum.end(); ++itr )
    {
        pairs.push_back( *itr );
    }

    sort( pairs.begin(), pairs.end(), [=]( std::pair<cvf::Vec2d, unsigned>& a, std::pair<cvf::Vec2d, unsigned>& b ) {
        return a.second > b.second;
    } );

    cvf::Vec2d primaryAxisVector = pairs[0].first * axesVectorCandidates[pairs[0].first];

    size_t                             row    = 0;
    size_t                             column = 0;
    std::vector<std::vector<unsigned>> indexToPointData;
    std::vector<unsigned>              startOffsets;
    cvf::Vec2d                         lineStartPoint;
    cvf::Vec2d                         lineEndPoint;
    unsigned                           largestStartOffset = 0;
    unsigned                           largestEndOffset   = 0;

    auto distanceOnLine = [to3d, to2d, surfacePoints, primaryAxisVector, epsilon]( const cvf::Vec2d linePoint1,
                                                                                   const cvf::Vec2d linePoint2,
                                                                                   const cvf::Vec2d point ) -> int {
        double     normalizedIntersection = 0.0;
        cvf::Vec2d projectedPoint         = to2d( cvf::GeometryTools::projectPointOnLine( to3d( linePoint1 ),
                                                                                  to3d( linePoint2 ),
                                                                                  to3d( point ),
                                                                                  &normalizedIntersection ) );
        if ( vectorFuzzyCompare( ( projectedPoint - to2d( surfacePoints[0] ) ).getNormalized(),
                                 primaryAxisVector.getNormalized(),
                                 epsilon ) )
            return static_cast<int>( ( projectedPoint - to2d( surfacePoints[0] ) ).length() / primaryAxisVector.length() );
        else
            return static_cast<int>( -( projectedPoint - to2d( surfacePoints[0] ) ).length() / primaryAxisVector.length() );
    };

    for ( size_t index = 0; index < surfacePoints.size(); index++ )
    {
        if ( index > 0 )
        {
            cvf::Vec2d vector = to2d( surfacePoints[index] - surfacePoints[index - 1] );
            if ( !vectorFuzzyCompare( vector.getNormalized(), primaryAxisVector.getNormalized(), epsilon ) )
            {
                column++;
                if ( indexToPointData.size() < column + 1 )
                {
                    indexToPointData.push_back( std::vector<unsigned>() );
                }
                row        = 0;
                int offset = distanceOnLine( lineStartPoint, lineEndPoint, to2d( surfacePoints[index] ) );
                if ( offset < 0 )
                {
                    startOffsets.push_back( std::abs( offset ) );
                    largestStartOffset = std::max<unsigned>( largestStartOffset, std::abs( offset ) );
                }
                else
                {
                    for ( int i = 0; i < offset; i++ )
                    {
                        indexToPointData[column].push_back( -1 );
                        row++;
                    }
                    startOffsets.push_back( 0 );
                }
            }
            else
            {
                size_t rowDiff = static_cast<size_t>( std::round( vector.length() / primaryAxisVector.length() ) );
                if ( rowDiff > 1 )
                {
                    for ( size_t i = 1; i < rowDiff; i++ )
                    {
                        indexToPointData[column].push_back( -1 );
                        row++;
                    }
                }
                int offset       = distanceOnLine( lineStartPoint, lineEndPoint, to2d( surfacePoints[index] ) );
                largestEndOffset = std::max<unsigned>( largestEndOffset, std::abs( offset ) );
            }
        }
        else
        {
            lineStartPoint = to2d( surfacePoints[index] ) - 9999999999.0 * primaryAxisVector;
            lineEndPoint   = to2d( surfacePoints[index] ) + 9999999999.0 * primaryAxisVector;
            startOffsets.push_back( 0 );
        }
        if ( indexToPointData.size() < column + 1 )
        {
            indexToPointData.push_back( std::vector<unsigned>() );
        }
        indexToPointData[column].push_back( static_cast<unsigned>( index ) );
        row++;
    }

    for ( size_t i = 0; i < startOffsets.size(); i++ )
    {
        unsigned endOffset = largestStartOffset + largestEndOffset;
        for ( unsigned j = startOffsets[i]; j < largestStartOffset; j++ )
        {
            indexToPointData[i].insert( indexToPointData[i].begin(), static_cast<unsigned>( -1 ) );
        }
        for ( unsigned j = static_cast<unsigned>( indexToPointData[i].size() ); j <= endOffset; j++ )
        {
            indexToPointData[i].push_back( static_cast<unsigned>( -1 ) );
        }
    }

    std::vector<unsigned> triangleIndices;
    if ( indexToPointData.size() < 2 )
    {
        return {};
    }

    size_t rowColumnStride = size_t( 1 );
    if ( primaryAxisVector.length() < preferredPointDistance )
    {
        rowColumnStride = preferredPointDistance / primaryAxisVector.length();
    }

    for ( size_t iIdx = 0; iIdx < indexToPointData.size() - 1; iIdx += rowColumnStride )
    {
        for ( size_t jIdx = 0; jIdx < indexToPointData[iIdx].size() - 1; jIdx += rowColumnStride )
        {
            generateTriangleIndices( indexToPointData, iIdx, jIdx, triangleIndices, static_cast<unsigned>( rowColumnStride ) );
        }
    }

    return std::make_pair( surfacePoints, triangleIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSurfaceImporter::generateTriangleIndices( const std::vector<std::vector<unsigned>>& indexToPointData,
                                                  const size_t&                             i,
                                                  const size_t&                             j,
                                                  std::vector<unsigned>&                    triangleIndices,
                                                  unsigned                                  resolution /*= 1 */ )
{
    unsigned topI = unsigned( std::min( indexToPointData.size() - 1, i + resolution ) );
    unsigned topJ = unsigned( std::min( indexToPointData[topI].size() - 1, j + resolution ) );

    if ( topI == i || topJ == j ) return false;

    unsigned q1 = indexToPointData[i + 0][j + 0];
    unsigned q2 = indexToPointData[i + 0][topJ];
    unsigned q3 = indexToPointData[topI][j + 0];
    unsigned q4 = indexToPointData[topI][topJ];

    if ( q1 != ( (unsigned)-1 ) && q2 != ( (unsigned)-1 ) && q3 != ( (unsigned)-1 ) && q4 != ( (unsigned)-1 ) )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSurfaceImporter::vectorFuzzyCompare( const cvf::Vec2d& vector1, const cvf::Vec2d& vector2, double epsilon )
{
    auto AlmostEqualRelativeAndAbs = [=]( double A, double B, double maxRelDiff ) -> bool {
        // Check if the numbers are really close -- needed
        // when comparing numbers near zero.
        double diff = fabs( A - B );

        A             = fabs( A );
        B             = fabs( B );
        float largest = ( B > A ) ? B : A;

        if ( diff <= largest * maxRelDiff ) return true;
        return false;
    };
    return ( AlmostEqualRelativeAndAbs( vector1.x(), vector2.x(), epsilon ) &&
             AlmostEqualRelativeAndAbs( vector1.y(), vector2.y(), epsilon ) );
}
