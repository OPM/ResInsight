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

#include <optional>
#include <string>
#include <vector>

class RigMainGrid;
class RimEclipseCase;
class RimWellPath;

namespace Opm
{
class DeckKeyword;
class DeckRecord;
} // namespace Opm

//==================================================================================================
///
///
//==================================================================================================
namespace RigEclipseResultTools
{
struct BorderCellFace;
}

namespace RimKeywordFactory
{

Opm::DeckKeyword welspecsKeyword( const std::string wellGrpName, RimEclipseCase* eCase, RimWellPath* wellPath );
Opm::DeckKeyword compdatKeyword( RimEclipseCase* eCase, RimWellPath* wellPath );

Opm::DeckKeyword welsegsKeyword( RimEclipseCase* eCase, RimWellPath* wellPath, const std::string completionText );
Opm::DeckKeyword compsegsKeyword( RimEclipseCase* eCase, RimWellPath* wellPath, const std::string completionText );
Opm::DeckKeyword wsegvalvKeyword( RimEclipseCase* eCase, RimWellPath* wellPath, const std::string completionText );
Opm::DeckKeyword wsegaicdKeyword( RimEclipseCase* eCase, RimWellPath* wellPath, const std::string completionText );

Opm::DeckKeyword faultsKeyword( const RigMainGrid* mainGrid,
                                const cvf::Vec3st& min        = cvf::Vec3st::ZERO,
                                const cvf::Vec3st& max        = cvf::Vec3st::UNDEFINED,
                                const cvf::Vec3st& refinement = cvf::Vec3st( 1, 1, 1 ) );
Opm::DeckKeyword bcconKeyword( const std::vector<RigEclipseResultTools::BorderCellFace>& borderCellFaces );
Opm::DeckKeyword bcpropKeyword( const std::vector<RigEclipseResultTools::BorderCellFace>& boundaryConditions,
                                const std::vector<Opm::DeckRecord>&                       boundaryConditionProperties );
Opm::DeckKeyword operaterKeyword( std::string          targetProperty,
                                  int                  regionId,
                                  std::string          equation,
                                  std::string          inputProperty,
                                  std::optional<float> alpha = std::nullopt,
                                  std::optional<float> beta  = std::nullopt );

} // namespace RimKeywordFactory
