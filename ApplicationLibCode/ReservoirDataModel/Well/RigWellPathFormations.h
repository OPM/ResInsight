/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaDefines.h"

#include "cvfVector2.h"

#include <optional>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include <QString>

namespace RiaDefines
{
enum class WellLogTrackFormationLevel;
}

struct RigWellPathFormation
{
    double                    mdTop{ 0.0 };
    double                    mdBase{ 0.0 };
    double                    tvdTop{ 0.0 };
    double                    tvdBase{ 0.0 };
    QString                   formationName;
    std::optional<cvf::Vec2d> topXY;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigWellPathFormations
{
public:
    using FormationLevel = RiaDefines::WellLogTrackFormationLevel;

public:
    RigWellPathFormations( const std::vector<RigWellPathFormation>& formations, const QString& filePath, const QString& key );

    // Returns formation names and depths as parallel vectors
    std::pair<std::vector<QString>, std::vector<double>>
        depthAndFormationNamesUpToLevel( FormationLevel level, bool includeFluids, RiaDefines::DepthType depthType ) const;

    // Returns formation name and (top, base) depth range per zone, for shaded region plotting.
    std::vector<std::tuple<QString, double, double>> depthRangesUpToLevel( FormationLevel level, RiaDefines::DepthType depthType ) const;

    std::vector<FormationLevel> formationsLevelsPresent() const;

    QString filePath() const;
    QString keyInFile() const;

    size_t                      formationNamesCount() const;
    size_t                      formationCount() const;
    const RigWellPathFormation& formationAt( size_t index ) const;

private:
    QString m_filePath;
    QString m_keyInFile;

    std::set<FormationLevel> m_formationsLevelsPresent;

    std::vector<std::pair<RigWellPathFormation, FormationLevel>> m_formations;
    std::vector<RigWellPathFormation>                            m_fluids;
};
