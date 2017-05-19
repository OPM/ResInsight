/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>

class RigFracturedEclipseCellExportData
{
public:
    RigFracturedEclipseCellExportData();

    // Compdat export data
    size_t reservoirCellIndex; 
    double transmissibility; // Total cell to well transmissibility finally used in COMPDAT keyword
    bool cellIsActive;

    // General intermediate results
    double NTG;
    cvf::Vec3d permeabilities;
    double skinFactor;
    
    // Elipse fracture related values
    cvf::Vec3d transmissibilities; //matrixToFractureTransmissibilitiesXYZ
    double totalArea; // Elipse cell overlap area
    double fractureLenght;
    cvf::Vec3d projectedAreas;

    cvf::Vec3d cellSizes;
    
    //TODO: Used for upscaling - should be moved?
    double upscaledStimPlanValueHA;
    double upscaledStimPlanValueAH;
};

//==================================================================================================
/// 
//==================================================================================================
class RigFracture : public cvf::Object
{
public:
    RigFracture();

    void setGeometry(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords);

    const std::vector<cvf::uint>&  triangleIndices() const;
    const std::vector<cvf::Vec3f>& nodeCoords() const;
 
private:
    std::vector<cvf::uint>  m_triangleIndices;
    std::vector<cvf::Vec3f> m_nodeCoords;
};

