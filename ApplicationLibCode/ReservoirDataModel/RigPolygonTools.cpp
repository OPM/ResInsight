/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RigPolygonTools.h"

#include "cvfGeometryTools.h"

#include <optional>
#include <stack>
#include <utility>

namespace RigPolygonTools
{
namespace internal
{
    struct GeometryData
    {
        double totalLength{ 0 };
        double lastSegmentLength{ 0 };
        double totalHorizontalLength{ 0 };
        double lastSegmentHorisontalLength{ 0 };
        double horizontalArea{ 0 };
    };

    // Function to check if a point is valid and within bounds
    bool isValid( int x, int y, int rows, int cols, const IntegerImage& image, const IntegerImage& visited )
    {
        return x >= 0 && x < rows && y >= 0 && y < cols && image[x][y] == 1 && !visited[x][y];
    }

    bool isValidImage( const IntegerImage& image )
    {
        if ( image.empty() ) return false;
        auto rowSize = image[0].size();
        for ( const auto& row : image )
        {
            if ( row.size() != rowSize ) return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void floodFill( IntegerImage& image, int x, int y, int oldColor, int newColor )
    {
        if ( !isValidImage( image ) ) return;

        auto              rows = static_cast<int>( image.size() );
        auto              cols = static_cast<int>( image[0].size() );
        std::stack<Point> stack;
        stack.push( { x, y } );

        while ( !stack.empty() )
        {
            auto [cx, cy] = stack.top();
            stack.pop();

            if ( cx < 0 || cy < 0 || cx >= rows || cy >= cols || image[cx][cy] != oldColor ) continue;

            image[cx][cy] = newColor;

            stack.push( { cx + 1, cy } );
            stack.push( { cx - 1, cy } );
            stack.push( { cx, cy + 1 } );
            stack.push( { cx, cy - 1 } );
        }
    }

    // Function to check if a point is on a line segment (edge of the polygon)
    bool isOnSegment( Point p, Point p1, Point p2 )
    {
        int x = p.first, y = p.second;
        int x1 = p1.first, y1 = p1.second;
        int x2 = p2.first, y2 = p2.second;

        // Check if the point (x, y) lies between (x1, y1) and (x2, y2)
        return ( ( x >= std::min( x1, x2 ) && x <= std::max( x1, x2 ) ) && ( y >= std::min( y1, y2 ) && y <= std::max( y1, y2 ) ) &&
                 ( ( x2 - x1 ) * ( y - y1 ) == ( y2 - y1 ) * ( x - x1 ) ) ); // Collinearity check
    }

    // Check if a point is inside a polygon using the Ray-Casting Algorithm
    bool isInsidePolygon( const Point& p, const std::vector<Point>& polygon )
    {
        int n     = static_cast<int>( polygon.size() );
        int count = 0;
        for ( int i = 0; i < n; i++ )
        {
            auto p1 = polygon[i];
            auto p2 = polygon[( i + 1 ) % n]; // Next vertex (looping back to first at end)

            // Check if point is exactly on an edge
            if ( isOnSegment( p, p1, p2 ) ) return true;

            // Check if point is between y-bounds of edge
            if ( ( p1.second > p.second ) != ( p2.second > p.second ) )
            {
                // Compute intersection point of the edge with horizontal line at p.y
                double xIntersect = p1.first + (double)( p.second - p1.second ) * ( p2.first - p1.first ) / ( p2.second - p1.second );
                if ( p.first < xIntersect )
                {
                    count++;
                }
            }
        }
        return ( count % 2 ) == 1; // Odd count means inside, even means outside
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    GeometryData computePolygonGeometryData( const std::vector<cvf::Vec3d>& vertices )
    {
        GeometryData geometryData;

        for ( size_t p = 1; p < vertices.size(); p++ )
        {
            const auto& p0 = vertices[p - 1];
            const auto& p1 = vertices[p];

            geometryData.lastSegmentLength = ( p1 - p0 ).length();

            const auto& p1_horiz = cvf::Vec3d( p1.x(), p1.y(), p0.z() );

            geometryData.lastSegmentHorisontalLength = ( p1_horiz - p0 ).length();

            geometryData.totalLength += geometryData.lastSegmentLength;
            geometryData.totalHorizontalLength += geometryData.lastSegmentHorisontalLength;
        }

        auto projectToZPlane = []( const std::vector<cvf::Vec3d>& vertices )
        {
            std::vector<cvf::Vec3d> pointsProjectedInZPlane;
            for ( const auto& p : vertices )
            {
                auto pointInZ = p;
                pointInZ.z()  = 0.0;
                pointsProjectedInZPlane.push_back( pointInZ );
            }
            return pointsProjectedInZPlane;
        };

        cvf::Vec3d area             = cvf::GeometryTools::polygonAreaNormal3D( projectToZPlane( vertices ) );
        geometryData.horizontalArea = cvf::Math::abs( area.z() );

        return geometryData;
    }

}; // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IntegerImage erode( IntegerImage image, int kernelSize )
{
    if ( !internal::isValidImage( image ) ) return {};
    if ( kernelSize <= 0 ) return {};

    auto         rows   = static_cast<int>( image.size() );
    auto         cols   = static_cast<int>( image[0].size() );
    int          offset = kernelSize / 2;
    IntegerImage eroded( rows, std::vector<int>( cols, 0 ) );

    for ( int i = offset; i < rows - offset; ++i )
    {
        for ( int j = offset; j < cols - offset; ++j )
        {
            bool erodePixel = true;
            for ( int ki = -offset; ki <= offset; ++ki )
            {
                for ( int kj = -offset; kj <= offset; ++kj )
                {
                    if ( image[i + ki][j + kj] == 0 )
                    {
                        erodePixel = false;
                        break;
                    }
                }
                if ( !erodePixel ) break;
            }
            eroded[i][j] = erodePixel ? 1 : 0;
        }
    }

    return eroded;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IntegerImage dilate( IntegerImage image, int kernelSize )
{
    if ( !internal::isValidImage( image ) ) return {};
    if ( kernelSize <= 0 ) return {};

    auto         rows   = static_cast<int>( image.size() );
    auto         cols   = static_cast<int>( image[0].size() );
    int          offset = kernelSize / 2;
    IntegerImage dilated( rows, std::vector<int>( cols, 0 ) );

    for ( int i = offset; i < rows - offset; ++i )
    {
        for ( int j = offset; j < cols - offset; ++j )
        {
            bool dilatePixel = false;
            for ( int ki = -offset; ki <= offset; ++ki )
            {
                for ( int kj = -offset; kj <= offset; ++kj )
                {
                    if ( image[i + ki][j + kj] == 1 )
                    {
                        dilatePixel = true;
                        break;
                    }
                }
                if ( dilatePixel ) break;
            }
            dilated[i][j] = dilatePixel ? 1 : 0;
        }
    }

    return dilated;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IntegerImage fillInterior( IntegerImage sourceImage )
{
    if ( !internal::isValidImage( sourceImage ) ) return {};

    auto image = sourceImage;
    auto rows  = static_cast<int>( image.size() );
    auto cols  = static_cast<int>( image[0].size() );

    // Flood fill the exterior (starting from the borders)
    for ( int i = 0; i < rows; ++i )
    {
        if ( image[i][0] == 0 ) internal::floodFill( image, i, 0, 0, -1 );
        if ( image[i][cols - 1] == 0 ) internal::floodFill( image, i, cols - 1, 0, -1 );
    }
    for ( int j = 0; j < cols; ++j )
    {
        if ( image[0][j] == 0 ) internal::floodFill( image, 0, j, 0, -1 );
        if ( image[rows - 1][j] == 0 ) internal::floodFill( image, rows - 1, j, 0, -1 );
    }

    // Fill interior holes (remaining 0s)
    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            if ( image[i][j] == 0 ) image[i][j] = 1;
        }
    }

    // Restore the exterior
    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            if ( image[i][j] == -1 ) image[i][j] = 0;
        }
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Point> boundary( const IntegerImage& image )
{
    if ( !internal::isValidImage( image ) ) return {};

    std::vector<Point> boundaries;

    // Get dimensions of the image
    int rows = static_cast<int>( image.size() );
    int cols = static_cast<int>( image[0].size() );

    // Direction vectors for clockwise search (8-connectivity)
    const std::vector<Point> directions = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };

    // Helper lambda to check if a pixel is a valid boundary pixel
    auto isBoundaryPixel = [&]( int x, int y )
    {
        if ( x < 0 || x >= rows || y < 0 || y >= cols || image[x][y] == 0 ) return false;
        // Check if it's adjacent to a background pixel
        for ( const auto& [dx, dy] : directions )
        {
            int nx = x + dx, ny = y + dy;
            if ( nx < 0 || nx >= rows || ny < 0 || ny >= cols || image[nx][ny] == 0 )
            {
                return true;
            }
        }
        return false;
    };

    // Find the starting boundary pixel
    Point start( -1, -1 );
    for ( int row = 0; row < rows; ++row )
    {
        for ( int col = 0; col < cols; ++col )
        {
            if ( isBoundaryPixel( row, col ) )
            {
                start = { row, col };
                break;
            }
        }
        if ( start.first != -1 ) break;
    }

    if ( start.first == -1 ) return boundaries; // No boundary found

    // Contour following algorithm
    Point current   = start;
    int   direction = 0; // Start search direction (arbitrary)
    do
    {
        boundaries.push_back( current );
        bool foundNext = false;

        // Look for the next boundary pixel in a clockwise direction
        for ( int i = 0; i < 8; ++i )
        {
            int newDir = ( direction + i ) % 8;
            int nx     = current.first + directions[newDir].first;
            int ny     = current.second + directions[newDir].second;

            if ( isBoundaryPixel( nx, ny ) )
            {
                current   = { nx, ny };
                direction = ( newDir + 6 ) % 8; // Adjust direction for next search
                foundNext = true;
                break;
            }
        }

        // If no next pixel is found, the boundary is invalid or incomplete
        if ( !foundNext ) break;

    } while ( current != start ); // Stop when we loop back to the start

    return boundaries;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigPolygonTools::IntegerImage assignValueInsidePolygon( IntegerImage image, const std::vector<Point>& polygon, int value )
{
    if ( !internal::isValidImage( image ) ) return {};

    auto rows = static_cast<int>( image.size() );
    auto cols = static_cast<int>( image[0].size() );

    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            if ( internal::isInsidePolygon( { i, j }, polygon ) )
            {
                image[i][j] = value;
            }
        }
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double area( const std::vector<Point>& polygon )
{
    int n = static_cast<int>( polygon.size() );
    if ( n < 3 ) return 0.0; // A polygon must have at least 3 points

    double area = 0.0;
    for ( int i = 0; i < n; i++ )
    {
        int j = ( i + 1 ) % n; // Next vertex, wrapping around at the end
        area += polygon[i].first * polygon[j].second;
        area -= polygon[j].first * polygon[i].second;
    }

    return std::fabs( area ) / 2.0;
}

//--------------------------------------------------------------------------------------------------
/// Ramer-Douglas-Peucker simplification algorithm
///
/// https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
//--------------------------------------------------------------------------------------------------
void simplifyPolygon( std::vector<cvf::Vec3d>& vertices, double epsilon )
{
    // If the polygon has fewer than 3 vertices, it cannot be simplified.
    if ( vertices.size() < 3 ) return;

    // Find the point with the maximum perpendicular distance from the line connecting the endpoints.
    std::optional<std::pair<size_t, double>> maxDistPoint;

    for ( size_t i = 1; i < vertices.size() - 1; ++i )
    {
        const cvf::Vec3d& point     = vertices[i];
        cvf::Vec3d        projected = cvf::GeometryTools::projectPointOnLine( vertices.front(), vertices.back(), point );
        double            distance  = ( projected - point ).length();

        if ( !maxDistPoint || distance > maxDistPoint->second )
        {
            maxDistPoint = std::make_pair( i, distance );
        }
    }

    // If the maximum distance exceeds epsilon, split and simplify recursively.
    if ( maxDistPoint && maxDistPoint->second > epsilon )
    {
        size_t splitIndex = maxDistPoint->first;

        // Divide the vertices into two segments.
        std::vector<cvf::Vec3d> segment1( vertices.begin(), vertices.begin() + splitIndex + 1 );
        std::vector<cvf::Vec3d> segment2( vertices.begin() + splitIndex, vertices.end() );

        // Recursively simplify both segments.
        simplifyPolygon( segment1, epsilon );
        simplifyPolygon( segment2, epsilon );

        // Combine the simplified segments, avoiding duplication at the split point.
        vertices = std::move( segment1 );
        vertices.pop_back(); // Remove duplicate at the split point.
        vertices.insert( vertices.end(), segment2.begin(), segment2.end() );
    }
    else
    {
        // If no point exceeds the threshold, reduce to endpoints.
        vertices = { vertices.front(), vertices.back() };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString geometryDataAsText( const std::vector<cvf::Vec3d>& vertices, bool includeLastSegmentInfo )
{
    auto geometryData = internal::computePolygonGeometryData( vertices );

    QString text;

    if ( vertices.size() > 2 )
    {
        if ( includeLastSegmentInfo )
        {
            text += QString( "Segment Length: %1\nSegment Horizontal Length: %2\n" )
                        .arg( geometryData.lastSegmentLength )
                        .arg( geometryData.lastSegmentHorisontalLength );
        }
        text +=
            QString( "Total Length: %1\nTotal Horizontal Length: %2\n" ).arg( geometryData.totalLength ).arg( geometryData.totalHorizontalLength );
        text += QString( "\nHorizontal Area : %1" ).arg( geometryData.horizontalArea );
    }
    else
    {
        text = QString( "Length: %1\nHorizontal Length: %2\n" ).arg( geometryData.lastSegmentLength ).arg( geometryData.lastSegmentHorisontalLength );
    }

    return text;
}

} // namespace RigPolygonTools
