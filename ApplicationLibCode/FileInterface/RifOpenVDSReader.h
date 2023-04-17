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

#include "RifSeismicReader.h"

class RifOpenVDSReader : public RifSeismicReader
{
public:
    RifOpenVDSReader();
    ~RifOpenVDSReader();

    bool open( QString filename ) override;
    void close() override;

    bool isValid() override;

    bool isOpen() const override;

    std::vector<std::pair<QString, QString>> metaData() override;

    cvf::BoundingBox boundingBox() override;

    void histogramData( std::vector<double>& xvals, std::vector<double>& yvals ) override;

    std::pair<double, double> dataRange() override;

    std::vector<cvf::Vec3d> worldCorners() override;

    cvf::Vec3i inlineMinMaxStep() override;
    cvf::Vec3i xlineMinMaxStep() override;

    double zStep() override;
    int    zSize() override;

    cvf::Vec3d          convertToWorldCoords( int iLine, int xLine, double depth ) override;
    std::pair<int, int> convertToInlineXline( double worldx, double worldy ) override;

    std::shared_ptr<ZGYAccess::SeismicSliceData>
        slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex = -1, int zSize = 0 ) override;
    std::shared_ptr<ZGYAccess::SeismicSliceData> trace( int inlineIndex, int xlineIndex, int zStartIndex = -1, int zSize = 0 ) override;

private:
    QString m_filename;
    // std::unique_ptr<ZGYAccess::ZGYReader> m_reader;
};
