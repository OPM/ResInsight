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
#include "RimSurface.h"

#include "RimSurfaceCollection.h"

#include "RigSurface.h"

#include <QFileInfo>

#include <cmath>

#include <fstream>
#include <sstream>

CAF_PDM_SOURCE_INIT( RimSurface, "Surface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::RimSurface()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SurfaceUserDecription", "Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File", "", "", "" );
    CAF_PDM_InitField( &m_color, "SurfaceColor", cvf::Color3f( 0.5f, 0.3f, 0.2f ), "Color", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::~RimSurface() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setSurfaceFilePath( const QString& filePath )
{
    m_surfaceDefinitionFilePath = filePath;
    if ( m_userDescription().isEmpty() )
    {
        m_userDescription = QFileInfo( filePath ).fileName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurface::surfaceFilePath()
{
    return m_surfaceDefinitionFilePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setColor( const cvf::Color3f& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSurface::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurface::userDescription()
{
    return m_userDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface* RimSurface::surfaceData()
{
    return m_surfaceData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurface::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                   const QVariant&            oldValue,
                                   const QVariant&            newValue )
{
    if ( changedField == &m_surfaceDefinitionFilePath )
    {
        updateSurfaceDataFromFile();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
    else if ( changedField == &m_color )
    {
        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
}

struct SurfacePointData
{
    int                 i;
    int                 j;
    cvf::Vec3d          point;
    std::vector<double> values;
};

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimSurface::updateSurfaceDataFromFile()
{
    std::ifstream stream( this->surfaceFilePath().toLatin1().data() );

    std::vector<SurfacePointData> surfaceDataPoints;
    int                           minI = std::numeric_limits<int>::max();
    int                           minJ = std::numeric_limits<int>::max();
    int                           maxI = std::numeric_limits<int>::min();
    int                           maxJ = std::numeric_limits<int>::min();

    while ( stream.good() )
    {
        std::string line;
        std::getline( stream, line );
        std::istringstream lineStream( line );

        double              x( HUGE_VAL ), y( HUGE_VAL ), z( HUGE_VAL );
        int                 i( -1 ), j( -1 );
        std::vector<double> values;

        // First check if we can read a number

        lineStream >> x;
        if ( lineStream.good() ) // If we can, assume this line is a surface point
        {
            lineStream >> y >> z >> i >> j;

            if ( x != HUGE_VAL && y != HUGE_VAL && z != HUGE_VAL && i != -1 && j != -1 )
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
        return false;
    }
    // clang-format on

    // Create full size grid matrix

    size_t iCount = maxI - minI + 1;
    size_t jCount = maxJ - minJ + 1;

    if ( iCount < 2 || jCount < 2 )
    {
        return false;
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
        return false;
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

                if ( q1 != ( (unsigned)-1 ) && q2 != ( (unsigned)-1 ) && q3 != ( (unsigned)-1 ) )
                {
                    triangleIndices.push_back( q1 );
                    triangleIndices.push_back( q4 );
                    triangleIndices.push_back( q3 );
                }
            }
        }
    }

    m_surfaceData = new RigSurface();
    m_surfaceData->setTriangleData( triangleIndices, vertices );

    return true;
}
