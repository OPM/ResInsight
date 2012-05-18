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

#include "cvfCollection.h"

namespace cvf {

class Uniform;


//==================================================================================================
//
// 
//
//==================================================================================================
class UniformSet : public Object
{
public:
    UniformSet();
    ~UniformSet();

    size_t          count() const;
    Uniform*        uniform(size_t index);
    const Uniform*  uniform(size_t index) const;
    void            setUniform(Uniform* uniform);
    void            removeUniform(const Uniform* uniform);

private:
    Collection<Uniform> m_uniforms;
};


}

