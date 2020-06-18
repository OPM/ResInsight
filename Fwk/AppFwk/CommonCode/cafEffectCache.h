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

#pragma once

#include "cvfBase.h"
#include "cvfEffect.h"
#include "cvfObject.h"

#include "cafEffectGenerator.h"

namespace caf
{
//==================================================================================================
//
// Private class. Used only by caf::EffectGenerator to cache effects (with GPU resources)
//
//==================================================================================================
class EffectCache
{
private:
    friend class EffectGenerator;
    EffectCache();

    static EffectCache* instance();

    cvf::Effect* findEffect( const EffectGenerator* generator );
    void         addEffect( const EffectGenerator* generator, cvf::Effect* effect );

    void releaseUnreferencedEffects();
    void clear();

private:
    EffectGenerator::RenderingModeType                              m_effectType;
    std::vector<std::pair<EffectGenerator*, cvf::ref<cvf::Effect>>> m_effectCache;
};

} // namespace caf
