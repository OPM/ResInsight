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
#include <vector>

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
    // Main entry point - extends all grid data
    static std::expected<void, QString> extendGrid( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings );

    // Helper functions for property array extension (exposed for testing)
    static std::vector<double> extendPropertyArray( const std::vector<double>& original,
                                                    int                        nx,
                                                    int                        ny,
                                                    int                        nz,
                                                    int                        nzUpper,
                                                    int                        nzLower,
                                                    double                     upperDefault,
                                                    double                     lowerDefault );

    static std::vector<int>
        extendIntPropertyArray( const std::vector<int>& original, int nx, int ny, int nz, int nzUpper, int nzLower, int upperDefault, int lowerDefault );

    // Geometry correction utilities (exposed for testing)
    static void makeVerticalPillars( std::vector<double>& coord, int nx, int ny );
    static void enforceMonotonicZcorn( std::vector<double>& zcorn, int nx, int ny, int nz, double minDist );
    static void fillZcornGaps( std::vector<double>& zcorn, int nx, int ny, int nz );

private:
    // Grid dimension extension
    static std::expected<void, QString> extendDimens( RifOpmFlowDeckFile& deckFile, int nzUpper, int nzLower );

    // COORD/ZCORN extension (core grid geometry)
    static std::expected<void, QString>
        extendGRDECL( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings, int nx, int ny, int nz, int nzUpper, int nzLower );

    // Property array extension (PORO, PERMX, etc.)
    static std::expected<void, QString>
        extendGridSection( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower, double upperPorosity );

    // Region keyword extension (EQUILNUM, SATNUM, etc.)
    static std::expected<void, QString>
        extendRegions( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower, int upperEquilnum );

    // ACTNUM extension
    static std::expected<void, QString> extendActnum( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower );

    // Helper to extract double array from deck keyword
    static std::expected<std::vector<double>, QString> extractDoubleArray( RifOpmFlowDeckFile& deckFile, const std::string& keyword );

    // Helper to extract int array from deck keyword
    static std::expected<std::vector<int>, QString> extractIntArray( RifOpmFlowDeckFile& deckFile, const std::string& keyword );

    // COORD extension helper
    static std::vector<double> extendCoord( const std::vector<double>& coord, int nx, int ny, double topZ, double bottomZ );

    // ZCORN extension helper
    static std::vector<double>
        extendZcorn( const std::vector<double>& zcorn, int nx, int ny, int nz, int nzUpper, int nzLower, double topZ, double bottomZ );
};
