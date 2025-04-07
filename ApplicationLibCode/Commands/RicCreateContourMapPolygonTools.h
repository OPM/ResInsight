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

#pragma once

#include <QImage>
#include <QString>

#include <vector>

class RigContourMapProjection;
class RimPolygon;

//==================================================================================================
///
//==================================================================================================
namespace RicCreateContourMapPolygonTools
{
QImage convertBinaryToImage( const std::vector<std::vector<int>>& data, QColor color, int transparency );
QImage convertBinaryToGrayscaleImage( const std::vector<std::vector<int>>& data, int colorValue );

void exportVectorAsImage( const std::vector<std::vector<int>>& data, int transparency, const QString& filename );
void exportVectorAsGrayscaleImage( const std::vector<std::vector<int>>& data, const QString& filename );

std::vector<std::vector<int>> convertImageToBinary( QImage image );
std::vector<std::vector<int>> convertToBinaryImage( const RigContourMapProjection* contourMapProjection );

void createPolygonObjects( std::vector<std::vector<int>> image, const RigContourMapProjection* contourMapProjection );

const RigContourMapProjection* findCurrentContourMapProjection();

}; // namespace RicCreateContourMapPolygonTools
