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
}

class RifSeismicZGYReader
{
public:
    RifSeismicZGYReader( QString filename );
    ~RifSeismicZGYReader();

    bool Open();
    void Close();

    std::vector<std::pair<QString, QString>> metaData();

    cvf::BoundingBox boundingBox();

private:
    QString                               m_filename;
    std::shared_ptr<ZGYAccess::ZGYReader> m_reader;
};
