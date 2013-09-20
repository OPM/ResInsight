//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfAssert.h"
#include "cvfUniformSet.h"
#include "cvfUniform.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::UniformSet
/// \ingroup Render
///
/// Manages a set of Uniform objects. The uniform names are used as keys, so unique names will be
/// enforced. Adding a uniform with a name that already exists in the set will replace the 
/// the existing uniform with the one being added.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformSet::UniformSet()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformSet::~UniformSet()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t UniformSet::count() const
{
    return m_uniforms.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Uniform* UniformSet::uniform(size_t index) const
{
    CVF_TIGHT_ASSERT(index < count());
    return m_uniforms[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Uniform* UniformSet::uniform(size_t index)
{
    CVF_TIGHT_ASSERT(index < count());
    return m_uniforms[index].p();
}


//--------------------------------------------------------------------------------------------------
/// Set or add a uniform to the UniformSet
/// 
/// If a uniform with the same name as the incoming uniform is already present in the set,
/// the existing uniform will be replaced.
//--------------------------------------------------------------------------------------------------
void UniformSet::setUniform(Uniform* uniform)
{
    CVF_ASSERT(uniform);

    // Check if uniform is already in the set
    size_t numUniforms = m_uniforms.size();
    for (size_t i = 0; i < numUniforms; ++i)
    {
        if (System::strcmp(m_uniforms[i]->name(), uniform->name()) == 0)
        {
            if (m_uniforms[i] != uniform)
            {
                m_uniforms[i] = uniform;
            }

            return;
        }
    }

    m_uniforms.push_back(uniform);
}


//--------------------------------------------------------------------------------------------------
/// Remove a uniform from the set
//--------------------------------------------------------------------------------------------------
void UniformSet::removeUniform(const Uniform* uniform)
{
    m_uniforms.erase(uniform);
}

} // namespace cvf
