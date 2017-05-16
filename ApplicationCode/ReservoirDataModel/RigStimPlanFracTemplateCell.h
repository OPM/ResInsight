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
class RigStimPlanFracTemplateCell 
{

public:
    RigStimPlanFracTemplateCell();
    RigStimPlanFracTemplateCell(std::vector<cvf::Vec3d> polygon, size_t i, size_t j);


    virtual ~RigStimPlanFracTemplateCell();

    std::vector<cvf::Vec3d> getPolygon() const { return m_polygon; }
    double                  getConductivtyValue() const { return m_concutivityValue; }
    double                  getDisplayValue() { return m_displayValue; }
    size_t                  getI() { return m_i; }
    size_t                  getJ() { return m_j; }
    double                  getVerticalTransmissibilityInFracture()   { return m_transmissibilityInFractureVertical; }
    double                  getHorizontalTransmissibilityInFracture() { return m_transmissibilityInFractureHorizontal; }
        

    void                    setConductivityValue(double cond) { m_concutivityValue = cond; }
    void                    setDisplayValue(double value) { m_displayValue = value; };
    void                    setTransmissibilityInFracture(double valueHorizontal, double valueVertical);

    double                  cellSizeX() const;
    double                  cellSizeZ() const;
private:
    std::vector<cvf::Vec3d> m_polygon;
    double                  m_displayValue;
    double                  m_concutivityValue;
    size_t                  m_i;
    size_t                  m_j;

    double                  m_transmissibilityInFractureVertical;
    double                  m_transmissibilityInFractureHorizontal;

};