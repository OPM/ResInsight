/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace RimCompletionEnums
{
enum class ReferenceMDType
{
    AUTO_REFERENCE_MD = 0,
    MANUAL_REFERENCE_MD
};

enum class PressureDropType
{
    HYDROSTATIC,
    HYDROSTATIC_FRICTION,
    HYDROSTATIC_FRICTION_ACCELERATION
};

enum class LengthAndDepthType
{
    ABS,
    INC
};

enum NonDarcyFlowType
{
    NON_DARCY_NONE,
    NON_DARCY_COMPUTED,
    NON_DARCY_USER_DEFINED,
};
} // namespace RimCompletionEnums
