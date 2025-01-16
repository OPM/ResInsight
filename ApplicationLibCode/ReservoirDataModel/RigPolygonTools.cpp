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

        auto                            rows = static_cast<int>( image.size() );
        auto                            cols = static_cast<int>( image[0].size() );
        std::stack<std::pair<int, int>> stack;
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
std::vector<std::pair<int, int>> boundary( const IntegerImage& image )
{
    if ( !internal::isValidImage( image ) ) return {};

    std::vector<std::pair<int, int>> boundaries;

    // Get dimensions of the image
    int rows = static_cast<int>( image.size() );
    int cols = static_cast<int>( image[0].size() );

    // Direction vectors for clockwise search (8-connectivity)
    const std::vector<std::pair<int, int>> directions = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };

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
    std::pair<int, int> start( -1, -1 );
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
    std::pair<int, int> current   = start;
    int                 direction = 0; // Start search direction (arbitrary)
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
} // namespace RigPolygonTools
