/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicCreateContourMapPolygonTools.h"

#include "RicExportContourMapToTextFeature.h"

#include "ContourMap/RigContourMapProjection.h"
#include "RigPolygonTools.h"

#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimGeoMechContourMapView.h"
#include "RimTools.h"

#include "RiuMainWindow.h"

#include "cvfBase.h"

#include <QInputDialog>

namespace internal
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, std::vector<cvf::Vec3d>>> findPolygons( std::vector<std::vector<int>>  image,
                                                                   const RigContourMapProjection* contourMapProjection )
{
    std::vector<std::pair<int, std::vector<cvf::Vec3d>>> polygons;

    if ( !contourMapProjection ) return {};
    if ( image.empty() ) return {};

    auto xVertexPositions = contourMapProjection->xVertexPositions();
    auto yVertexPositions = contourMapProjection->yVertexPositions();
    auto origin3d         = contourMapProjection->origin3d();
    auto depth            = contourMapProjection->topDepthBoundingBox();

    auto hasAnyValue = []( const std::vector<std::vector<int>>& image ) -> bool
    {
        for ( const auto& row : image )
        {
            for ( int val : row )
            {
                if ( val != 0 ) return true;
            }
        }

        return false;
    };

    while ( hasAnyValue( image ) )
    {
        std::vector<cvf::Vec3d> polygonDomainCoords;
        auto                    boundaryPoints = RigPolygonTools::boundary( image );

        for ( const auto& [i, j] : boundaryPoints )
        {
            auto xDomain = xVertexPositions.at( i ) + origin3d.x();
            auto yDomain = yVertexPositions.at( j ) + origin3d.y();

            polygonDomainCoords.emplace_back( cvf::Vec3d( xDomain, yDomain, depth ) );
        }

        // Epsilon used to simplify polygon. Useful range typical value in [5..50]
        const double defaultEpsilon = 40.0;
        RigPolygonTools::simplifyPolygon( polygonDomainCoords, defaultEpsilon );

        if ( polygonDomainCoords.size() >= 3 )
        {
            const auto area = RigPolygonTools::area( boundaryPoints );

            polygons.push_back( { area, polygonDomainCoords } );
        }

        // Subtract all pixels inside the polygon by setting their value to 0
        const int value = 0;
        image           = RigPolygonTools::assignValueInsidePolygon( image, boundaryPoints, value );

        /*
                // NB: Do not remove, debug code to export images to file
                {
                    QString filename = "boundary_removed";
                    filename         = QString( "f:/scratch/images/%1%2.png" ).arg( filename ).arg( QString::number( imageCounter ) );
                    RicCreateContourMapPolygonTools::exportVectorAsGrayscaleImage( currentImage, filename );
                }
        */
    }

    return polygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void createPolygonObjects( const std::vector<std::vector<cvf::Vec3d>>& polygons )
{
    auto polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return;

    for ( const auto& polygonDomainCoords : polygons )
    {
        auto newPolygon = polygonCollection->appendUserDefinedPolygon();
        newPolygon->setPointsInDomainCoords( polygonDomainCoords );
        newPolygon->coordinatesChanged.send();
    }

    polygonCollection->updateAllRequiredEditors();
}

} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RicCreateContourMapPolygonTools::convertBinaryToImage( const std::vector<std::vector<int>>& data, QColor color, int transparency )
{
    if ( data.empty() || data[0].empty() )
    {
        qWarning( "Data is empty. Cannot export an image." );
        return {};
    }

    // Get dimensions
    int height = static_cast<int>( data[0].size() );
    int width  = static_cast<int>( data.size() );

    // Create a QImage
    QImage image( width, height, QImage::Format_ARGB32 );

    // Fill QImage with data
    for ( int y = 0; y < height; ++y )
    {
        for ( int x = 0; x < width; ++x )
        {
            int value = std::clamp( data[x][y], 0, 255 );
            if ( value > 0 )
                image.setPixel( x, height - y - 1, qRgba( color.red(), color.green(), color.blue(), transparency ) ); // Grayscale
            else
            {
                image.setPixel( x, height - y - 1, qRgba( 0, 0, 0, transparency ) );
            }
        }
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RicCreateContourMapPolygonTools::convertBinaryToGrayscaleImage( const std::vector<std::vector<int>>& data, int colorValue )
{
    if ( data.empty() || data[0].empty() )
    {
        qWarning( "Data is empty. Cannot export an image." );
        return {};
    }

    // Get dimensions
    int height = static_cast<int>( data[0].size() );
    int width  = static_cast<int>( data.size() );

    // Create a QImage
    QImage image( width, height, QImage::Format_Grayscale8 );

    // Fill QImage with data
    for ( int y = 0; y < height; ++y )
    {
        for ( int x = 0; x < width; ++x )
        {
            int value = std::clamp( data[x][y] * colorValue, 0, 255 ); // Ensure value is in [0, 255]
            image.setPixel( x, height - y - 1, qRgb( value, value, value ) ); // Grayscale
        }
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonTools::exportVectorAsImage( const std::vector<std::vector<int>>& data, int transparency, const QString& filename )
{
    if ( data.empty() || data[0].empty() )
    {
        qWarning( "Data is empty. Cannot export an image." );
        return;
    }

    auto image = convertBinaryToImage( data, QColorConstants::Green, transparency );

    if ( !image.save( filename, "PNG" ) )
    {
        qWarning( "Failed to save image as PNG." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonTools::exportVectorAsGrayscaleImage( const std::vector<std::vector<int>>& data, const QString& filename )
{
    if ( data.empty() || data[0].empty() )
    {
        qWarning( "Data is empty. Cannot export an image." );
        return;
    }

    auto image = convertBinaryToGrayscaleImage( data, 255 );

    // Save the QImage as a PNG file
    if ( !image.save( filename, "PNG" ) )
    {
        qWarning( "Failed to save image as PNG." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RicCreateContourMapPolygonTools::convertImageToBinary( QImage image )
{
    std::vector<std::vector<int>> binaryImage( image.width(), std::vector<int>( image.height(), 0 ) );
    for ( int i = 0; i < image.width(); ++i )
    {
        for ( int j = 0; j < image.height(); ++j )
        {
            auto pixelColor = image.pixel( i, j );
            auto gray       = qGray( pixelColor );

            binaryImage[i][image.height() - j - 1] = gray > 0 ? 1 : 0;
        }
    }
    return binaryImage;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RicCreateContourMapPolygonTools::convertToBinaryImage( const RigContourMapProjection* rigContourMapProjection )
{
    if ( !rigContourMapProjection ) return {};

    auto vertexSizeIJ = rigContourMapProjection->numberOfVerticesIJ();

    std::vector<std::vector<int>> image( vertexSizeIJ.x(), std::vector<int>( vertexSizeIJ.y(), 0 ) );

    auto filteredValues = rigContourMapProjection->aggregatedVertexResultsFiltered();

    for ( cvf::uint i = 0; i < vertexSizeIJ.x(); i++ )
    {
        for ( cvf::uint j = 0; j < vertexSizeIJ.y(); j++ )
        {
            auto vertexIndex = rigContourMapProjection->vertexIndex( i, j );
            double valueAtVertex = vertexIndex < filteredValues.size() ? filteredValues[vertexIndex] : std::numeric_limits<double>::infinity();
            if ( !std::isinf( valueAtVertex ) )
            {
                image[i][j] = 1;
            }
            else
            {
                image[i][j] = 0;
            }
        }
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateContourMapPolygonTools::createPolygonObjects( std::vector<std::vector<int>>  image,
                                                            const RigContourMapProjection* contourMapProjection )
{
    auto polygons = internal::findPolygons( image, contourMapProjection );
    if ( polygons.empty() ) return;

    std::sort( polygons.begin(), polygons.end(), []( const auto& a, const auto& b ) { return a.first > b.first; } );

    int         polygonCount = 0;
    QStringList polygonInfo;
    for ( const auto& polygon : polygons )
    {
        polygonInfo
            << QString( "Polygon %1: Normalized area %2, %3 vertices " ).arg( polygonCount++ ).arg( polygon.first ).arg( polygon.second.size() );
        if ( polygonCount > 10 )
        {
            polygonInfo << "More polygons found, but only the first 10 are shown.";
            break;
        }
    }

    QString txt = polygonInfo.join( "\n" );
    txt += "\n\n Define normalized area threshold:";

    bool ok;
    int  areaThreshold = QInputDialog::getInt( RiuMainWindow::instance(), "Create Polygons", txt, 10, 1, 10000, 1, &ok );
    if ( ok )
    {
        std::vector<std::vector<cvf::Vec3d>> polygonsToCreate;
        for ( const auto& polygon : polygons )
        {
            if ( polygon.first >= areaThreshold )
            {
                polygonsToCreate.push_back( polygon.second );
            }
        }

        internal::createPolygonObjects( polygonsToCreate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigContourMapProjection* RicCreateContourMapPolygonTools::findCurrentContourMapProjection()
{
    RimContourMapProjection* contourMapProjection = nullptr;

    auto [existingEclipseContourMap, existingGeoMechContourMap] = RicExportContourMapToTextFeature::findContourMapView();
    if ( existingEclipseContourMap ) contourMapProjection = existingEclipseContourMap->contourMapProjection();
    if ( existingGeoMechContourMap ) contourMapProjection = existingGeoMechContourMap->contourMapProjection();

    if ( !contourMapProjection ) return nullptr;

    return contourMapProjection->mapProjection();
}
