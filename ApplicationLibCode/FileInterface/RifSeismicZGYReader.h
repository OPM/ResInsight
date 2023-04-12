/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022  Equinor ASA
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

#include "cvfVector3.h"

#include "RiaSeismicDefines.h"

#include <QString>

#include <memory>
#include <utility>
#include <vector>

namespace ZGYAccess
{
class ZGYReader;
class SeismicSliceData;
} // namespace ZGYAccess

namespace cvf
{
class BoundingBox;
} // namespace cvf

class RifSeismicZGYReader
{
public:
    RifSeismicZGYReader();
    ~RifSeismicZGYReader();

    bool open( QString filename );
    void close();

    bool isValid();

    bool isOpen() const;

    std::vector<std::pair<QString, QString>> metaData();

    cvf::BoundingBox boundingBox();

    void histogramData( std::vector<double>& xvals, std::vector<double>& yvals );

    std::pair<double, double> dataRange();

    std::vector<cvf::Vec3d> worldCorners();

    cvf::Vec3i inlineMinMaxStep();
    cvf::Vec3i xlineMinMaxStep();

    double zStep();
    int    zSize();

    cvf::Vec3d          convertToWorldCoords( int iLine, int xLine, double depth );
    std::pair<int, int> convertToInlineXline( double worldx, double worldy );

    std::shared_ptr<ZGYAccess::SeismicSliceData>
        slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex = -1, int zSize = 0 );
    std::shared_ptr<ZGYAccess::SeismicSliceData> trace( int inlineIndex, int xlineIndex, int zStartIndex = -1, int zSize = 0 );

private:
    QString                               m_filename;
    std::unique_ptr<ZGYAccess::ZGYReader> m_reader;
};
