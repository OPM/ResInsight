/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiuPickItemInfo.h"

#include "cafCmdFeature.h"
#include "cafPickEventHandler.h"


#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf { 
    class Part; 
} 

class Rim3dView;

//==================================================================================================
/// 
//==================================================================================================
class Ric3dPickEvent : public caf::PickEvent
{
public:
    Ric3dPickEvent(const std::vector<RiuPickItemInfo>& pickItemInfos, 
                   Rim3dView* view)
        : m_pickItemInfos(pickItemInfos)
        , m_view(view)
    {
    }

    std::vector<RiuPickItemInfo> m_pickItemInfos;
    Rim3dView*                   m_view; 
};


//==================================================================================================
/// A static always-on pick handler used in the RiuViewerCommand        
//==================================================================================================
class RicDefaultPickEventHandler
{
public:
    virtual bool handle3dPickEvent(const Ric3dPickEvent& eventObject) = 0;
};

