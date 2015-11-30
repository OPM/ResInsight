/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafCmdFeature.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"


class RicCommandFeature : public caf::CmdFeature
{
public:
    virtual bool handleUiEvent(cvf::Object* uiEventObject) = 0;
};

class RicLocalIntersectionUiEvent : public cvf::Object
{
public:
    RicLocalIntersectionUiEvent(cvf::Vec3d localIntersectionPoint)
        : localIntersectionPoint(localIntersectionPoint)
    {
    }

    cvf::Vec3d localIntersectionPoint;
};

