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
#include "RiuPickItemInfo.h"

namespace cvf { 
    class Part; 
} 

class Rim3dView;

//==================================================================================================
/// 
//==================================================================================================
class Ric3DPickEvent
{
public:
    Ric3DPickEvent(const std::vector<RiuPickItemInfo>& pickItemInfos, 
                   Rim3dView* view)
        : m_pickItemInfos(pickItemInfos)
        , m_view(view)
    {
    }

    std::vector<RiuPickItemInfo> m_pickItemInfos;
    Rim3dView*                   m_view; 
};


//==================================================================================================
/// 
//==================================================================================================
class RicPickEventHandler
{
public:
    virtual bool handlePickEvent(const Ric3DPickEvent& eventObject) = 0;
};

