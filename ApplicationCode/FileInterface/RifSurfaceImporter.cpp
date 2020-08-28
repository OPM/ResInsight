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
            {
                unsigned q1 = indexToPointData[iIdx + 0][jIdx + 0];
                unsigned q2 = indexToPointData[iIdx + 0][jIdx + 1];
                unsigned q3 = indexToPointData[iIdx + 1][jIdx + 0];
                unsigned q4 = indexToPointData[iIdx + 1][jIdx + 1];

                if ( q1 != ( (unsigned)-1 ) && q2 != ( (unsigned)-1 ) && q4 != ( (unsigned)-1 ) )
                {
                    triangleIndices.push_back( q1 );
                    triangleIndices.push_back( q2 );
                    triangleIndices.push_back( q4 );
                }

                if ( q1 != ( (unsigned)-1 ) && q4 != ( (unsigned)-1 ) && q3 != ( (unsigned)-1 ) )
                {
                    triangleIndices.push_back( q1 );
                    triangleIndices.push_back( q4 );
                    triangleIndices.push_back( q3 );
                }
            }
        }
    }

    return std::make_pair( vertices, triangleIndices );
}
