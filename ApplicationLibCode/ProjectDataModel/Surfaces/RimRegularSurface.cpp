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

#include "RimRegularSurface.h"

#include "RigSurface.h"

#include "RimSurfaceCollection.h"

#include "cafPdmObjectScriptingCapability.h"
// #include "cafPdmUiDoubleSliderEditor.h"
// #include "cafPdmUiDoubleValueEditor.h"

#include "cvfVector3.h"

CAF_PDM_SOURCE_INIT( RimRegularSurface, "RimRegularSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularSurface::RimRegularSurface()
{
    CAF_PDM_InitScriptableObject( "RegularSurface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitField( &m_nx, "Nx", 10, "Nx" );
    CAF_PDM_InitField( &m_ny, "Ny", 10, "Ny" );
    CAF_PDM_InitField( &m_originX, "OriginX", 0.0, "Origin X" );
    CAF_PDM_InitField( &m_originY, "OriginY", 0.0, "Origin Y" );
    CAF_PDM_InitField( &m_incrementX, "IncrementX", 20.0, "Increment X" );
    CAF_PDM_InitField( &m_incrementY, "IncrementY", 20.0, "Increment Y" );
    CAF_PDM_InitField( &m_rotation, "Rotation", 0.0, "Rotation" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularSurface::~RimRegularSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimRegularSurface::createCopy()
{
    return copyObject<RimRegularSurface>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// bool RimRegularSurface::showIntersectionCellResults()
// {
//     // Avoid use of cell intersection results color for depth surfaces
//     return false;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimRegularSurface::setPlaneExtent( double minX, double minY, double maxX, double maxY )
// {
//     m_minX = minX;
//     m_minY = minY;
//     m_maxX = maxX;
//     m_maxY = maxY;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimRegularSurface::setDepth( double depth )
// {
//     m_depth = depth;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimRegularSurface::setDepthSliderLimits( double lower, double upper )
// {
//     m_depthLowerLimit = lower;
//     m_depthUpperLimit = upper;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimRegularSurface::setAreaOfInterest( cvf::Vec3d min, cvf::Vec3d max )
// {
//     m_areaOfInterestMin = min;
//     m_areaOfInterestMax = max;

//     auto lowerDepthLimit = -max.z();
//     auto upperDepthLimit = -min.z();
//     setDepthSliderLimits( lowerDepthLimit, upperDepthLimit );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    clearCachedNativeData();
    updateSurfaceData();

    auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
    surfColl->updateViews( { this } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimSurface::defineEditorAttribute( field, uiConfigName, attribute );

    // caf::PdmUiDoubleValueEditorAttribute::testAndSetFixedWithTwoDecimals( attribute );

    // if ( field == &m_depth )
    // {
    //     if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
    //     {
    //         attr->m_minimum = m_depthLowerLimit;
    //         attr->m_maximum = m_depthUpperLimit;
    //     }
    // }

    // if ( field == &m_minX || field == &m_maxX )
    // {
    //     if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
    //     {
    //         attr->m_minimum = m_areaOfInterestMin().x();
    //         attr->m_maximum = m_areaOfInterestMax().x();
    //     }
    // }

    // if ( field == &m_minY || field == &m_maxY )
    // {
    //     if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
    //     {
    //         attr->m_minimum = m_areaOfInterestMin().y();
    //         attr->m_maximum = m_areaOfInterestMax().y();
    //     }
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // uiOrdering.add( &m_depth );

    // {
    //     auto group = uiOrdering.addNewGroup( "Depth Limits" );
    //     group->setCollapsedByDefault();
    //     group->add( &m_depthLowerLimit );
    //     group->add( &m_depthUpperLimit );
    // }

    // {
    //     auto group = uiOrdering.addNewGroup( "Extent" );
    //     group->add( &m_minX );
    //     group->add( &m_maxX );
    //     group->add( &m_minY );
    //     group->add( &m_maxY );
    // }

    {
        // Fields from RimSurface

        auto group = uiOrdering.addNewGroup( "Appearance" );
        group->add( &m_userDescription );
        group->add( &m_color );

        group->add( &m_enableOpacity );
        group->add( &m_opacity );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::clearCachedNativeData()
{
    m_vertices.clear();
    m_triangleIndices.clear();
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimRegularSurface::updateSurfaceData()
{
    //    auto displayZ = -m_depth;

    auto surf_xyz_from_ij = []( int i, int j, double xori, double xinc, double yori, double yinc, double rot_deg ) -> cvf::Vec3d
    {
        /* locals */
        // double angle, xdist, ydist, dist, beta, gamma, dxrot, dyrot;

        // if (i < 1 || i > nx || j < 1 || j > ny) {
        //     if (i == 0)
        //         i = 1;
        //     if (i == nx + 1)
        //         i = nx;
        //     if (j == 0)
        //         j = 1;
        //     if (j == ny + 1)
        //         j = ny;

        //     /* retest if more severe and return -1 if case*/
        //     if (i < 1 || i > nx || j < 1 || j > ny) {
        //         *x = 0.0;
        //         *y = 0.0;
        //         *z = UNDEF;
        //         throw_exception("Accessing value outside surface");
        //         return -1;
        //     }
        // }

        // if ( flag == 0 )
        // {
        //     ic = x_ijk2ic( i, j, 1, nx, ny, 1, 0 ); /* C order */
        //     if ( ic < 0 )
        //     {
        //         *z = UNDEF;
        //     }
        //     else
        //     {
        //         *z = p_map_v[ic];
        //     }
        // }
        // else
        // {
        //     *z = 999.00;
        // }

        double x = xori;
        double y = yori;
        // TODO:
        double z = 1000.0;

        if ( i == 0 && j == 0 )
        {
            x = xori;
            y = yori;
            // return ( 0 );
        }
        else
        {
            //   yinc = yinc * yflip;

            /* cube rotation: this should be the usual angle, anti-clock from x axis */
            double angle = rot_deg * std::numbers::pi / 180.0; /* radians, positive */

            double xdist = xinc * i;
            double ydist = yinc * j;

            /* distance of point from "origo" */
            double dist = sqrt( xdist * xdist + ydist * ydist );

            double beta = acos( xdist / dist );

            /* secure that angle is in right mode */
            /* if (xdist<0 && ydist<0)  beta=2*PI - beta; */
            /* if (xdist>=0 && ydist<0) beta=PI + beta; */

            if ( beta < 0 || beta > std::numbers::pi / 2.0 || std::isnan( beta ) )
            {
                x = 0.0;
                y = 0.0;
                //            throw_exception( "Unvalid value for beta in: surf_xyz_from_ij" );
                //            return ( -9 );
            }
            else
            {
                double gamma = angle + beta; /* the difference in rotated coord system */

                double dxrot = dist * cos( gamma );
                double dyrot = dist * sin( gamma );

                x = xori + dxrot;
                y = yori + dyrot;
            }
        }

        return cvf::Vec3d( x, y, z );
    };

    for ( int j = 0; j < m_ny(); j++ )
    {
        for ( int i = 0; i < m_nx(); i++ )
        {
            m_vertices.push_back( surf_xyz_from_ij( i, j, m_originX, m_originY, m_incrementX, m_incrementY, m_rotation ) );
        }
    }

    // cvf::Vec3d a( m_minX, m_minY, displayZ );
    // cvf::Vec3d b( m_maxX, m_minY, displayZ );
    // cvf::Vec3d c( m_maxX, m_maxY, displayZ );
    // cvf::Vec3d d( m_minX, m_maxY, displayZ );

    // m_vertices.push_back( a );
    // m_vertices.push_back( b );
    // m_vertices.push_back( c );
    // m_vertices.push_back( d );

    m_triangleIndices.push_back( 0 );
    m_triangleIndices.push_back( 1 );
    m_triangleIndices.push_back( 2 );

    m_triangleIndices.push_back( 0 );
    m_triangleIndices.push_back( 2 );
    m_triangleIndices.push_back( 3 );

    if ( !m_triangleIndices.empty() )
    {
        std::vector<unsigned>   tringleIndices{ m_triangleIndices };
        std::vector<cvf::Vec3d> vertices{ m_vertices };

        cvf::Vec3d offset;
        offset.z() += depthOffset();

        RimSurface::applyDepthOffset( offset, &vertices );
        auto surfaceData = new RigSurface;
        surfaceData->setTriangleData( tringleIndices, vertices );

        setSurfaceData( surfaceData );
    }
    else
    {
        setSurfaceData( nullptr );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the project tree
//--------------------------------------------------------------------------------------------------
// QString RimRegularSurface::fullName() const
// {
//     QString retval = RimSurface::fullName();
//     if ( !retval.isEmpty() ) retval += " - ";
//     retval += "Depth: ";
//     retval += QString::number( m_depth );
//     return retval;
// }
