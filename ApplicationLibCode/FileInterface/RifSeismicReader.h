/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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
class SeismicSliceData;
} // namespace ZGYAccess

namespace cvf
{
class BoundingBox;
} // namespace cvf

class RifSeismicReader
{
public:
    RifSeismicReader(){};
    virtual ~RifSeismicReader(){};

    virtual bool open( QString filename ) = 0;
    virtual void close()                  = 0;

    virtual bool isValid() = 0;

    virtual bool isOpen() const = 0;

    virtual std::vector<std::pair<QString, QString>> metaData() = 0;

    virtual cvf::BoundingBox boundingBox() = 0;

    virtual void histogramData( std::vector<double>& xvals, std::vector<double>& yvals ) = 0;

    virtual std::pair<double, double> dataRange() = 0;

    virtual std::vector<cvf::Vec3d> worldCorners() = 0;

    virtual cvf::Vec3i inlineMinMaxStep() = 0;
    virtual cvf::Vec3i xlineMinMaxStep()  = 0;

    virtual double zStep() = 0;
    virtual int    zSize() = 0;

    virtual cvf::Vec3d          convertToWorldCoords( int iLine, int xLine, double depth ) = 0;
    virtual std::pair<int, int> convertToInlineXline( double worldx, double worldy )       = 0;

    virtual std::shared_ptr<ZGYAccess::SeismicSliceData>
        slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex = -1, int zSize = 0 )                      = 0;
    virtual std::shared_ptr<ZGYAccess::SeismicSliceData> trace( int inlineIndex, int xlineIndex, int zStartIndex = -1, int zSize = 0 ) = 0;
};
