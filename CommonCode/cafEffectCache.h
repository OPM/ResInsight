//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfEffect.h"

#include "cafEffectGenerator.h"

namespace caf {

//==================================================================================================
//
// 
//
//==================================================================================================
class EffectCache
{
public:
    EffectCache();

    static EffectCache* instance();
   
    cvf::ref<cvf::Effect>   getOrCreateEffect(const EffectGenerator* generator);
    void                    clear();

private:
    EffectGenerator::RenderingModeType m_effectType;

    std::vector<std::pair<EffectGenerator*, cvf::ref<cvf::Effect> > > m_effectCache;
};
    


}
