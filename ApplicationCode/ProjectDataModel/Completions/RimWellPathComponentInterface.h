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

#include "RiaDefines.h"

#include "cvfBase.h"
#include "cvfColor3.h"

//==================================================================================================
// Interface implemented by all well path construction components and completions
// 
//
//==================================================================================================
class RimWellPathComponentInterface
{
public:
    virtual bool                              isEnabled() const = 0;
    virtual RiaDefines::WellPathComponentType componentType() const = 0;
    virtual QString                           componentLabel() const = 0;
    virtual QString                           componentTypeLabel() const = 0;
    virtual cvf::Color3f                      defaultComponentColor() const = 0;
    virtual double                            startMD() const = 0;
    virtual double                            endMD() const = 0;
};


