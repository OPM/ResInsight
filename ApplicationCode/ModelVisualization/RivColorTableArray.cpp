/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivColorTableArray.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Color3fArray> RivColorTableArray::colorTableArray()
{
    cvf::ref<cvf::Color3fArray> partColors = new cvf::Color3fArray();
    partColors->reserve(10);

    partColors->add(cvf::Color3f(101.0f/255, 132.0f/255,  96.0f/255)); // Dark green 
    partColors->add(cvf::Color3f(255.0f/255, 131.0f/255, 140.0f/255)); // Old pink 
    partColors->add(cvf::Color3f(210.0f/255, 176.0f/255, 112.0f/255)); // Light Brown 
    partColors->add(cvf::Color3f(140.0f/255, 171.0f/255, 238.0f/255)); // Light gray blue 
    partColors->add(cvf::Color3f(255.0f/255, 205.0f/255, 131.0f/255)); // Peach 
    partColors->add(cvf::Color3f(220.0f/255, 212.0f/255, 166.0f/255)); // Dark off white 
    partColors->add(cvf::Color3f(130.0f/255, 255.0f/255, 120.0f/255)); // Light green 
    partColors->add(cvf::Color3f(166.0f/255, 220.0f/255, 215.0f/255)); // Light gray torquise 
    partColors->add(cvf::Color3f(168.0f/255, 220.0f/255, 166.0f/255)); // Light gray green
    partColors->add(cvf::Color3f(255.0f/255,  64.0f/255, 236.0f/255)); // Magneta 

    return partColors;
}

