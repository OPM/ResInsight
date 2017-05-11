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

class RigFractureData
{
public:
    RigFractureData();

    size_t reservoirCellIndex;
    double transmissibility;
    cvf::Vec3d transmissibilities;
    
    double totalArea;
    double fractureLenght;
    cvf::Vec3d projectedAreas;

    cvf::Vec3d permeabilities;
    cvf::Vec3d cellSizes;
    double NTG;
    double skinFactor;

    bool cellIsActive;
    
    //TODO: Used for upscaling - should be moved?
    double upscaledStimPlanValueHA;
    double upscaledStimPlanValueAH;
};


class RigStimPlanFractureCell
{
public:
    RigStimPlanFractureCell(size_t i, size_t j);

    size_t                  m_i;
    size_t                  m_j;

    std::vector<size_t>     getGlobalIndeciesToContributingEclipseCells() { return globalIndeciesToContributingEclipseCells; }
    std::vector<double>     getContributingEclipseCellTransmissibilities() { return contributingEclipseCellTransmissibilities; }
    void                    addContributingEclipseCell(size_t eclipseCell, double transmissibility);

private:
    double                  performationLenghtVertical;
    double                  performationLenghtHorizontal;

    std::vector<size_t>     globalIndeciesToContributingEclipseCells;
    std::vector<double>     contributingEclipseCellTransmissibilities;

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

    void setFractureData(const std::vector<RigFractureData>& data);
    const std::vector<RigFractureData>& fractureData() const; //Access frac data

    std::vector<RigFractureData> m_fractureData;

    void addStimPlanCellFractureCell(RigStimPlanFractureCell fracStimPlanCellData);

private:
    std::vector<cvf::uint>  m_triangleIndices;
    std::vector<cvf::Vec3f> m_nodeCoords;

    std::vector<RigStimPlanFractureCell>    m_stimPlanCellsFractureData;
    std::vector<size_t>                         m_perforatedStimPlanCellsIndex;
};

