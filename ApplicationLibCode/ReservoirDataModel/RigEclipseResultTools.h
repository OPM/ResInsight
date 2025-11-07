/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025  Equinor ASA
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
#include <vector>

#include "cvfArray.h"
#include "cvfStructGrid.h"

class RimEclipseCase;
class RimEclipseView;

namespace RigEclipseResultTools
{
enum BorderType : int
{
    INVISIBLE_CELL = 0,
    BORDER_CELL    = 1,
    INTERIOR_CELL  = 2
};

struct BorderCellFace
{
    cvf::Vec3st                        ijk; // Cell indices (0-based)
    cvf::StructGridInterface::FaceType faceType;
    int                                boundaryCondition; // BCCON grid value
};

void createResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<int>& intValues );

void generateBorderResult( RimEclipseCase* eclipseCase, cvf::ref<cvf::UByteArray> customVisibility, const QString& resultName = "BORDER" );

int generateOperNumResult( RimEclipseCase* eclipseCase, int borderCellValue = -1 );

int findMaxOperNumValue( RimEclipseCase* eclipseCase );

int findMaxBcconValue( RimEclipseCase* eclipseCase );

void generateBcconResult( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max );

std::vector<BorderCellFace> generateBorderCellFaces( RimEclipseCase* eclipseCase );

} // namespace RigEclipseResultTools
