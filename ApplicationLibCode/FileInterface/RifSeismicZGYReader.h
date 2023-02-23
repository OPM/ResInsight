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

#include <QString>

#include <memory>
#include <utility>
#include <vector>

namespace ZGYAccess
{
class ZGYReader;
}

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

    bool isOpen() const;

    std::vector<std::pair<QString, QString>> metaData();

    cvf::BoundingBox boundingBox();

    void histogramData( std::vector<double>& xvals, std::vector<double>& yvals );

    std::pair<double, double> dataRange();

    std::vector<cvf::Vec3d> worldCorners();

    double depthStep();

    cvf::Vec3d convertToWorldCoords( int iLine, int xLine, double depth );

private:
    QString                               m_filename;
    std::shared_ptr<ZGYAccess::ZGYReader> m_reader;
};
