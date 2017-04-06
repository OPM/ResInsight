/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
#include "cvfVector3.h"

#include <vector>

//==================================================================================================
///  
///  
//==================================================================================================
class RigStimPlanCell 
{

public:
    RigStimPlanCell();
    RigStimPlanCell(std::vector<cvf::Vec3d> polygon, size_t i, size_t j);


    virtual ~RigStimPlanCell();

    std::vector<cvf::Vec3d> getPolygon() { return m_polygon; }
    double                  getConductivtyValue() { return m_concutivityValue; }
    double                  getDisplayValue() { return m_displayValue; }
    size_t                  getI() { return m_i; }
    size_t                  getJ() { return m_j; }

    void setConductivityValue(double cond) { m_concutivityValue = cond; }
    void setDisplayValue(double value) { m_displayValue = value; };

    void                    addContributingEclipseCell(size_t fracCell, double transmissibility);
    std::vector<size_t>     getContributingEclipseCells() { return contributingEclipseCells; }
    std::vector<double>     getContributingEclipseCellTransmisibilities() { return contributingEclipseCellTransmisibilities; }

private:
    std::vector<cvf::Vec3d> m_polygon;
    double                  m_displayValue;
    double                  m_concutivityValue;
    size_t                  m_i;
    size_t                  m_j;

    std::vector<size_t>     contributingEclipseCells;
    std::vector<double>     contributingEclipseCellTransmisibilities;

};