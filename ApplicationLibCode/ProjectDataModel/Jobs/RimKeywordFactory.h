/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "cvfVector3.h"

#include <string>

class RigMainGrid;
class RimEclipseCase;
class RimWellPath;

namespace Opm
{
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
///
//==================================================================================================
namespace RimKeywordFactory
{

Opm::DeckKeyword welspecsKeyword( const std::string wellGrpName, RimEclipseCase* eCase, RimWellPath* wellPath );
Opm::DeckKeyword compdatKeyword( RimEclipseCase* eCase, RimWellPath* wellPath );
Opm::DeckKeyword faultsKeyword( const RigMainGrid* mainGrid,
                                const cvf::Vec3st& min        = cvf::Vec3st::ZERO,
                                const cvf::Vec3st& max        = cvf::Vec3st::UNDEFINED,
                                const cvf::Vec3st& refinement = cvf::Vec3st( 1, 1, 1 ) );

} // namespace RimKeywordFactory
