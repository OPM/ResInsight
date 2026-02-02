/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include <expected>
#include <string>
#include <vector>

namespace Opm
{
class Deck;
class FileDeck;
} // namespace Opm

class RifOpmFlowDeckFile;
class RigModelPaddingSettings;

//==================================================================================================
/// Grid padding utilities adapted from OPM-Flow's padmodel.cpp
/// Source: https://github.com/hnil/opm-flowgeomechanics/blob/geomech_hypre_cgal/examples/padmodel.cpp
///
/// This class provides functions to extend eclipse grid data in the Z-direction by adding
/// padding layers above and/or below the existing grid. This is useful for sector models
/// that need additional layers for geomechanical simulations.
//==================================================================================================
class RigPadModel
{
public:
    // Main entry point - extends all grid data via FileDeck in-place modification
    static std::expected<void, QString> extendGrid( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings );

    // Helper functions for property array extension
    static double getPropsDefaultValue( const Opm::Deck& deck, const std::string& keyword );

    static std::vector<double> extendPropertyArray( const std::vector<double>& original,
                                                    int                        nx,
                                                    int                        ny,
                                                    int                        nz,
                                                    int                        nzUpper,
                                                    int                        nzLower,
                                                    double                     upperDefault,
                                                    double                     lowerDefault );
};
