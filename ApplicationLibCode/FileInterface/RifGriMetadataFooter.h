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
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>

#include <map>
#include <optional>
#include <utility>
#include <vector>

//==================================================================================================
/// Appends and reads a text metadata footer at the end of a binary GRI (IRAP) surface file.
///
/// The IRAP binary format is record based: readers consume the fixed header and exactly
/// ncol x nrow values, so bytes after the last value record are ignored by surface importers.
/// This makes it possible to attach metadata, e.g. for the ensemble contour map statistics
/// cache, while the file remains importable as a regular surface.
///
/// The footer is a marker line followed by key=value lines:
///     ----RESINSIGHT_METADATA_V1----
///     validityKey=e60c3dfc979d4b5bfc14d1b3a29b8e2a
///     sampleSpacing=50
//==================================================================================================
class RifGriMetadataFooter
{
public:
    static bool appendFooter( const QString& fileName, const std::vector<std::pair<QString, QString>>& keyValues );

    static std::optional<std::map<QString, QString>> readFooter( const QString& fileName );

    static QString markerLine();
};
