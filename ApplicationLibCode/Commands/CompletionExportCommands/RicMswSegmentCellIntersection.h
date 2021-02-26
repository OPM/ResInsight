/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RicMswSegmentCellIntersection
{
public:
    RicMswSegmentCellIntersection( const QString&     gridName, // Pass in empty string for main grid
                                      size_t             globalCellIndex,
                                      const cvf::Vec3st& gridLocalCellIJK,
                                      const cvf::Vec3d&  lengthsInCell );
    const QString&    gridName() const;
    size_t            globalCellIndex() const;
    cvf::Vec3st       gridLocalCellIJK() const;
    const cvf::Vec3d& lengthsInCell() const;

private:
    QString     m_gridName;
    size_t      m_globalCellIndex;
    cvf::Vec3st m_gridLocalCellIJK;
    cvf::Vec3d  m_lengthsInCell;
};
