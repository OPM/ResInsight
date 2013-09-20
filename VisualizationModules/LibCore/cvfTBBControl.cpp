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
#include "cvfTBBControl.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::TBBControl
/// \ingroup Core
///
/// Static class used to enable or disable the use of Intel TBB inside the libraries. 
/// Only relevant if the libraries are being built with TBB support, in which case the default 
/// enable setting will be true. If no TBB support is compiled in, the default enable state is false.
/// 
//==================================================================================================

#ifdef CVF_USE_TBB
bool TBBControl::sm_useTBB = true;
#else
bool TBBControl::sm_useTBB = false;
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TBBControl::enable(bool enableTBB)
{
#ifdef CVF_USE_TBB
    sm_useTBB = enableTBB;
#else
    CVF_UNUSED(enableTBB);
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TBBControl::isEnabled() 
{
    return sm_useTBB;
}


} // namespace cvf

