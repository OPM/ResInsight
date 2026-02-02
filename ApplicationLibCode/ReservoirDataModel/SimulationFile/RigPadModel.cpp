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

#include "RigPadModel.h"

#include "RifOpmFlowDeckFile.h"
#include "RigModelPaddingSettings.h"

#include "RiaLogging.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
/// Main entry point - extends all grid data
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigPadModel::extendGrid( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings )
{
    if ( !settings.isEnabled() )
    {
        return {};
    }

    int nzUpper = settings.nzUpper();
    int nzLower = settings.nzLower();

    if ( nzUpper == 0 && nzLower == 0 )
    {
        RiaLogging::info( "Model padding enabled but no padding layers specified - skipping" );
        return {};
    }

    RiaLogging::info( QString( "Applying model padding: %1 upper layers, %2 lower layers" ).arg( nzUpper ).arg( nzLower ) );

    // Get current grid dimensions from SPECGRID keyword
    auto specgridKw = deckFile.findKeyword( "SPECGRID" );
    if ( !specgridKw.has_value() || specgridKw->size() == 0 )
    {
        return std::unexpected( "Failed to read SPECGRID dimensions for model padding" );
    }

    const auto& record = specgridKw->getRecord( 0 );
    int         nx     = record.getItem( 0 ).get<int>( 0 );
    int         ny     = record.getItem( 1 ).get<int>( 0 );
    int         nz     = record.getItem( 2 ).get<int>( 0 );

    RiaLogging::info( QString( "Grid dimensions: %1 x %2 x %3 -> %1 x %2 x %4" ).arg( nx ).arg( ny ).arg( nz ).arg( nz + nzUpper + nzLower ) );

    // Step 1: Extend grid geometry (COORD, ZCORN)
    auto grdeclResult = extendGRDECL( deckFile, settings, nx, ny, nz, nzUpper, nzLower );
    if ( !grdeclResult.has_value() )
    {
        return grdeclResult;
    }

    // Step 2: Update DIMENS and SPECGRID keywords
    auto dimensResult = extendDimens( deckFile, nzUpper, nzLower );
    if ( !dimensResult.has_value() )
    {
        return dimensResult;
    }

    // Step 3: Extend ACTNUM
    auto actnumResult = extendActnum( deckFile, nx, ny, nz, nzUpper, nzLower );
    if ( !actnumResult.has_value() )
    {
        return actnumResult;
    }

    // Step 4: Extend property arrays (PORO, PERMX, PERMY, PERMZ, NTG)
    auto propsResult = extendGridSection( deckFile, nx, ny, nz, nzUpper, nzLower, settings.upperPorosity() );
    if ( !propsResult.has_value() )
    {
        return propsResult;
    }

    // Step 5: Extend region arrays (EQUILNUM, SATNUM, PVTNUM)
    auto regionsResult = extendRegions( deckFile, nx, ny, nz, nzUpper, nzLower, settings.upperEquilnum() );
    if ( !regionsResult.has_value() )
    {
        return regionsResult;
    }

    RiaLogging::info( QString( "Model padding applied successfully: %1 x %2 x %3" ).arg( nx ).arg( ny ).arg( nz + nzUpper + nzLower ) );

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Update DIMENS and SPECGRID keywords with new Z dimension
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigPadModel::extendDimens( RifOpmFlowDeckFile& deckFile, int nzUpper, int nzLower )
{
    // Get current dimensions from SPECGRID
    auto specgridKw = deckFile.findKeyword( "SPECGRID" );
    if ( !specgridKw.has_value() || specgridKw->size() == 0 )
    {
        return std::unexpected( "Failed to read SPECGRID for dimension update" );
    }

    const auto& record = specgridKw->getRecord( 0 );
    int         nx     = record.getItem( 0 ).get<int>( 0 );
    int         ny     = record.getItem( 1 ).get<int>( 0 );
    int         nz     = record.getItem( 2 ).get<int>( 0 );
    int         newNz  = nz + nzUpper + nzLower;

    // Update DIMENS keyword
    if ( !deckFile.setDimens( nx, ny, newNz ) )
    {
        return std::unexpected( "Failed to update DIMENS keyword for model padding" );
    }

    // Update SPECGRID keyword
    if ( !deckFile.setSpecgrid( nx, ny, newNz ) )
    {
        return std::unexpected( "Failed to update SPECGRID keyword for model padding" );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend COORD and ZCORN arrays for grid geometry
//--------------------------------------------------------------------------------------------------
std::expected<void, QString>
    RigPadModel::extendGRDECL( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings, int nx, int ny, int nz, int nzUpper, int nzLower )
{
    // Extract COORD array
    auto coordResult = extractDoubleArray( deckFile, "COORD" );
    if ( !coordResult.has_value() )
    {
        return std::unexpected( coordResult.error() );
    }
    std::vector<double> coord = coordResult.value();

    // Extract ZCORN array
    auto zcornResult = extractDoubleArray( deckFile, "ZCORN" );
    if ( !zcornResult.has_value() )
    {
        return std::unexpected( zcornResult.error() );
    }
    std::vector<double> zcorn = zcornResult.value();

    // Validate array sizes
    size_t expectedCoordSize = static_cast<size_t>( ( nx + 1 ) * ( ny + 1 ) * 6 );
    size_t expectedZcornSize = static_cast<size_t>( nx * ny * nz * 8 );

    if ( coord.size() != expectedCoordSize )
    {
        return std::unexpected( QString( "COORD array size mismatch: expected %1, got %2" ).arg( expectedCoordSize ).arg( coord.size() ) );
    }

    if ( zcorn.size() != expectedZcornSize )
    {
        return std::unexpected( QString( "ZCORN array size mismatch: expected %1, got %2" ).arg( expectedZcornSize ).arg( zcorn.size() ) );
    }

    // Get existing Z range from COORD pillars
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for ( int j = 0; j <= ny; j++ )
    {
        for ( int i = 0; i <= nx; i++ )
        {
            int    idx  = ( j * ( nx + 1 ) + i ) * 6;
            double zTop = coord[idx + 2];
            double zBot = coord[idx + 5];
            minZ        = std::min( minZ, std::min( zTop, zBot ) );
            maxZ        = std::max( maxZ, std::max( zTop, zBot ) );
        }
    }

    // Determine new top and bottom Z values
    double topZ    = ( settings.topUpper() != 0.0 ) ? settings.topUpper() : minZ;
    double bottomZ = ( settings.bottomLower() != 0.0 ) ? settings.bottomLower() : maxZ;

    // Apply geometry corrections if requested
    if ( settings.verticalPillars() )
    {
        makeVerticalPillars( coord, nx, ny );
    }

    if ( settings.fillGaps() )
    {
        fillZcornGaps( zcorn, nx, ny, nz );
    }

    if ( settings.monotonicZcorn() )
    {
        enforceMonotonicZcorn( zcorn, nx, ny, nz, settings.minLayerThickness() );
    }

    // Extend COORD array (adjust pillar Z values)
    std::vector<double> newCoord = extendCoord( coord, nx, ny, topZ, bottomZ );

    // Extend ZCORN array (add Z corners for new layers)
    std::vector<double> newZcorn = extendZcorn( zcorn, nx, ny, nz, nzUpper, nzLower, topZ, bottomZ );

    // Replace keywords in deck
    if ( !deckFile.replaceKeyword( "COORD", newCoord, true ) )
    {
        return std::unexpected( "Failed to update COORD keyword" );
    }

    if ( !deckFile.replaceKeyword( "ZCORN", newZcorn, true ) )
    {
        return std::unexpected( "Failed to update ZCORN keyword" );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend COORD array by adjusting pillar Z values
//--------------------------------------------------------------------------------------------------
std::vector<double> RigPadModel::extendCoord( const std::vector<double>& coord, int nx, int ny, double topZ, double bottomZ )
{
    std::vector<double> newCoord = coord;

    // COORD array stores pillar coordinates: (nx+1)*(ny+1) pillars, 6 values each
    // Each pillar: (x_top, y_top, z_top, x_bottom, y_bottom, z_bottom)
    for ( int j = 0; j <= ny; j++ )
    {
        for ( int i = 0; i <= nx; i++ )
        {
            int idx = ( j * ( nx + 1 ) + i ) * 6;
            // Adjust Z values at top and bottom of pillars
            newCoord[idx + 2] = topZ; // z_top
            newCoord[idx + 5] = bottomZ; // z_bottom
        }
    }

    return newCoord;
}

//--------------------------------------------------------------------------------------------------
/// Extend ZCORN array by adding Z corners for new layers
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RigPadModel::extendZcorn( const std::vector<double>& zcorn, int nx, int ny, int nz, int nzUpper, int nzLower, double topZ, double bottomZ )
{
    int newNz = nz + nzUpper + nzLower;

    // ZCORN has 8 corners per cell, but stored as 2*nx * 2*ny per k-layer (top and bottom)
    // Total size: nx * ny * nz * 8 = 2*nx * 2*ny * 2*nz
    size_t              newSize = static_cast<size_t>( nx * ny * newNz * 8 );
    std::vector<double> newZcorn( newSize );

    // Calculate layer thickness for upper and lower padding
    // Find the topmost Z value from the original ZCORN (first layer, top corners)
    double originalTopZ    = std::numeric_limits<double>::max();
    double originalBottomZ = std::numeric_limits<double>::lowest();

    // In ZCORN, the first 2*nx*2*ny values are the top corners of layer 0
    int xyCorners = 2 * nx * 2 * ny;
    for ( int i = 0; i < xyCorners; i++ )
    {
        originalTopZ = std::min( originalTopZ, zcorn[i] );
    }

    // Bottom corners of last layer
    int bottomOffset = ( 2 * nz - 1 ) * xyCorners;
    for ( int i = 0; i < xyCorners; i++ )
    {
        originalBottomZ = std::max( originalBottomZ, zcorn[bottomOffset + i] );
    }

    double upperLayerThickness = ( nzUpper > 0 ) ? ( originalTopZ - topZ ) / nzUpper : 0.0;
    double lowerLayerThickness = ( nzLower > 0 ) ? ( bottomZ - originalBottomZ ) / nzLower : 0.0;

    // Fill upper padding layers
    for ( int k = 0; k < nzUpper; k++ )
    {
        double layerTopZ    = topZ + k * upperLayerThickness;
        double layerBottomZ = topZ + ( k + 1 ) * upperLayerThickness;

        // Top corners of layer k
        int topOffset = k * 2 * xyCorners;
        for ( int i = 0; i < xyCorners; i++ )
        {
            newZcorn[topOffset + i] = layerTopZ;
        }

        // Bottom corners of layer k
        int botOffset = ( k * 2 + 1 ) * xyCorners;
        for ( int i = 0; i < xyCorners; i++ )
        {
            newZcorn[botOffset + i] = layerBottomZ;
        }
    }

    // Copy original ZCORN data (shifted by nzUpper layers)
    int originalOffset = nzUpper * 2 * xyCorners;
    for ( size_t i = 0; i < zcorn.size(); i++ )
    {
        newZcorn[originalOffset + i] = zcorn[i];
    }

    // Fill lower padding layers
    for ( int k = 0; k < nzLower; k++ )
    {
        int    newK         = nzUpper + nz + k;
        double layerTopZ    = originalBottomZ + k * lowerLayerThickness;
        double layerBottomZ = originalBottomZ + ( k + 1 ) * lowerLayerThickness;

        // Top corners of layer newK
        int topOffset = newK * 2 * xyCorners;
        for ( int i = 0; i < xyCorners; i++ )
        {
            newZcorn[topOffset + i] = layerTopZ;
        }

        // Bottom corners of layer newK
        int botOffset = ( newK * 2 + 1 ) * xyCorners;
        for ( int i = 0; i < xyCorners; i++ )
        {
            newZcorn[botOffset + i] = layerBottomZ;
        }
    }

    return newZcorn;
}

//--------------------------------------------------------------------------------------------------
/// Extend ACTNUM array with active cells for padding layers
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigPadModel::extendActnum( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower )
{
    // Extract existing ACTNUM if present
    auto actnumKw = deckFile.findKeyword( "ACTNUM" );
    if ( !actnumKw.has_value() )
    {
        // No ACTNUM keyword - all cells are active by default
        // Create ACTNUM with all 1s for the extended grid
        int              totalCells = nx * ny * ( nz + nzUpper + nzLower );
        std::vector<int> newActnum( totalCells, 1 );

        // No existing ACTNUM to replace, so we don't need to update
        return {};
    }

    auto actnumResult = extractIntArray( deckFile, "ACTNUM" );
    if ( !actnumResult.has_value() )
    {
        return std::unexpected( actnumResult.error() );
    }
    std::vector<int> actnum = actnumResult.value();

    // All padding cells are active (value = 1)
    std::vector<int> newActnum = extendIntPropertyArray( actnum, nx, ny, nz, nzUpper, nzLower, 1, 1 );

    if ( !deckFile.replaceKeyword( "ACTNUM", newActnum, true ) )
    {
        return std::unexpected( "Failed to update ACTNUM keyword" );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend property arrays (PORO, PERMX, PERMY, PERMZ, NTG)
//--------------------------------------------------------------------------------------------------
std::expected<void, QString>
    RigPadModel::extendGridSection( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower, double upperPorosity )
{
    // List of property keywords to extend
    struct PropertyInfo
    {
        std::string keyword;
        double      upperDefault;
        double      lowerDefault;
    };

    // PORO uses the configured upper porosity
    // PERMX/Y/Z use 0 for impermeable padding layers
    // NTG (net-to-gross) uses 1.0 for full rock
    std::vector<PropertyInfo> properties = { { "PORO", upperPorosity, 0.0 },
                                             { "PERMX", 0.0, 0.0 },
                                             { "PERMY", 0.0, 0.0 },
                                             { "PERMZ", 0.0, 0.0 },
                                             { "NTG", 1.0, 1.0 } };

    for ( const auto& prop : properties )
    {
        auto kwOpt = deckFile.findKeyword( prop.keyword );
        if ( !kwOpt.has_value() )
        {
            // Keyword not present - skip
            continue;
        }

        auto dataResult = extractDoubleArray( deckFile, prop.keyword );
        if ( !dataResult.has_value() )
        {
            RiaLogging::warning(
                QString( "Could not extract %1 data: %2" ).arg( QString::fromStdString( prop.keyword ) ).arg( dataResult.error() ) );
            continue;
        }

        std::vector<double> data    = dataResult.value();
        std::vector<double> newData = extendPropertyArray( data, nx, ny, nz, nzUpper, nzLower, prop.upperDefault, prop.lowerDefault );

        if ( !deckFile.replaceKeyword( prop.keyword, newData, true ) )
        {
            RiaLogging::warning( QString( "Failed to update %1 keyword" ).arg( QString::fromStdString( prop.keyword ) ) );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend region arrays (EQUILNUM, SATNUM, PVTNUM)
//--------------------------------------------------------------------------------------------------
std::expected<void, QString>
    RigPadModel::extendRegions( RifOpmFlowDeckFile& deckFile, int nx, int ny, int nz, int nzUpper, int nzLower, int upperEquilnum )
{
    // Region keywords to extend
    struct RegionInfo
    {
        std::string keyword;
        int         upperDefault;
        int         lowerDefault;
    };

    // Upper padding uses the configured EQUILNUM
    // Lower padding copies from the bottom layer (default 1)
    std::vector<RegionInfo> regions = { { "EQUILNUM", upperEquilnum, 1 }, { "SATNUM", 1, 1 }, { "PVTNUM", 1, 1 } };

    for ( const auto& region : regions )
    {
        auto kwOpt = deckFile.findKeyword( region.keyword );
        if ( !kwOpt.has_value() )
        {
            // Keyword not present - skip
            continue;
        }

        auto dataResult = extractIntArray( deckFile, region.keyword );
        if ( !dataResult.has_value() )
        {
            RiaLogging::warning(
                QString( "Could not extract %1 data: %2" ).arg( QString::fromStdString( region.keyword ) ).arg( dataResult.error() ) );
            continue;
        }

        std::vector<int> data = dataResult.value();

        // For lower padding, use the value from the bottom layer
        int lowerDefault = region.lowerDefault;
        if ( !data.empty() && nz > 0 )
        {
            // Get value from first cell of bottom layer
            int bottomLayerOffset = nx * ny * ( nz - 1 );
            if ( bottomLayerOffset < static_cast<int>( data.size() ) )
            {
                lowerDefault = data[bottomLayerOffset];
            }
        }

        std::vector<int> newData = extendIntPropertyArray( data, nx, ny, nz, nzUpper, nzLower, region.upperDefault, lowerDefault );

        if ( !deckFile.replaceKeyword( region.keyword, newData, true ) )
        {
            RiaLogging::warning( QString( "Failed to update %1 keyword" ).arg( QString::fromStdString( region.keyword ) ) );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend a property array with default values for padding layers
//--------------------------------------------------------------------------------------------------
std::vector<double> RigPadModel::extendPropertyArray( const std::vector<double>& original,
                                                      int                        nx,
                                                      int                        ny,
                                                      int                        nz,
                                                      int                        nzUpper,
                                                      int                        nzLower,
                                                      double                     upperDefault,
                                                      double                     lowerDefault )
{
    int    xyCount = nx * ny;
    int    newNz   = nz + nzUpper + nzLower;
    size_t newSize = static_cast<size_t>( xyCount ) * newNz;

    std::vector<double> result;
    result.reserve( newSize );

    // Upper padding layers - use upperDefault
    for ( int k = 0; k < nzUpper; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( upperDefault );
        }
    }

    // Original data
    result.insert( result.end(), original.begin(), original.end() );

    // Lower padding layers - use lowerDefault
    for ( int k = 0; k < nzLower; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( lowerDefault );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Extend an integer property array with default values for padding layers
//--------------------------------------------------------------------------------------------------
std::vector<int> RigPadModel::extendIntPropertyArray( const std::vector<int>& original,
                                                      int                     nx,
                                                      int                     ny,
                                                      int                     nz,
                                                      int                     nzUpper,
                                                      int                     nzLower,
                                                      int                     upperDefault,
                                                      int                     lowerDefault )
{
    int    xyCount = nx * ny;
    int    newNz   = nz + nzUpper + nzLower;
    size_t newSize = static_cast<size_t>( xyCount ) * newNz;

    std::vector<int> result;
    result.reserve( newSize );

    // Upper padding layers - use upperDefault
    for ( int k = 0; k < nzUpper; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( upperDefault );
        }
    }

    // Original data
    result.insert( result.end(), original.begin(), original.end() );

    // Lower padding layers - use lowerDefault
    for ( int k = 0; k < nzLower; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( lowerDefault );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Make all pillars vertical (X,Y same at top and bottom)
//--------------------------------------------------------------------------------------------------
void RigPadModel::makeVerticalPillars( std::vector<double>& coord, int nx, int ny )
{
    // COORD: (nx+1)*(ny+1) pillars, 6 values each: (x_top, y_top, z_top, x_bot, y_bot, z_bot)
    for ( int j = 0; j <= ny; j++ )
    {
        for ( int i = 0; i <= nx; i++ )
        {
            int idx        = ( j * ( nx + 1 ) + i ) * 6;
            coord[idx + 3] = coord[idx]; // x_bottom = x_top
            coord[idx + 4] = coord[idx + 1]; // y_bottom = y_top
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Ensure Z values increase monotonically within each pillar column
//--------------------------------------------------------------------------------------------------
void RigPadModel::enforceMonotonicZcorn( std::vector<double>& zcorn, int nx, int ny, int nz, double minDist )
{
    // ZCORN layout: 2*nx * 2*ny values per half-layer (top/bottom of each cell)
    // Total: 2*nz layers of 2*nx*2*ny values
    int xyCorners = 2 * nx * 2 * ny;

    // Process each column of corners (there are 2*nx * 2*ny columns)
    for ( int cornerCol = 0; cornerCol < xyCorners; cornerCol++ )
    {
        // Collect all Z values for this corner column
        std::vector<double> zValues( 2 * nz );
        for ( int layer = 0; layer < 2 * nz; layer++ )
        {
            zValues[layer] = zcorn[layer * xyCorners + cornerCol];
        }

        // Enforce monotonic increase with minimum distance
        for ( int layer = 1; layer < 2 * nz; layer++ )
        {
            if ( zValues[layer] < zValues[layer - 1] + minDist )
            {
                zValues[layer] = zValues[layer - 1] + minDist;
            }
        }

        // Write back
        for ( int layer = 0; layer < 2 * nz; layer++ )
        {
            zcorn[layer * xyCorners + cornerCol] = zValues[layer];
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Fill gaps between cell layers by averaging adjacent Z values
//--------------------------------------------------------------------------------------------------
void RigPadModel::fillZcornGaps( std::vector<double>& zcorn, int nx, int ny, int nz )
{
    // ZCORN layout: for each cell k, top corners at 2k, bottom corners at 2k+1
    // Gaps occur between bottom of layer k and top of layer k+1
    int xyCorners = 2 * nx * 2 * ny;

    for ( int k = 0; k < nz - 1; k++ )
    {
        // Bottom of layer k is at position (2k+1)
        // Top of layer k+1 is at position (2*(k+1)) = 2k+2
        int botK      = ( 2 * k + 1 ) * xyCorners;
        int topKplus1 = ( 2 * k + 2 ) * xyCorners;

        for ( int corner = 0; corner < xyCorners; corner++ )
        {
            double zBot = zcorn[botK + corner];
            double zTop = zcorn[topKplus1 + corner];
            double avgZ = ( zBot + zTop ) / 2.0;

            zcorn[botK + corner]      = avgZ;
            zcorn[topKplus1 + corner] = avgZ;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Extract double array from deck keyword
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<double>, QString> RigPadModel::extractDoubleArray( RifOpmFlowDeckFile& deckFile, const std::string& keyword )
{
    auto kwOpt = deckFile.findKeyword( keyword );
    if ( !kwOpt.has_value() )
    {
        return std::unexpected( QString( "Keyword %1 not found" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& kw = kwOpt.value();
    if ( kw.size() == 0 )
    {
        return std::unexpected( QString( "Keyword %1 has no records" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& record = kw.getRecord( 0 );
    if ( record.size() == 0 )
    {
        return std::unexpected( QString( "Keyword %1 record has no items" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& item = record.getItem( 0 );

    std::vector<double> data;
    data.reserve( item.data_size() );

    for ( size_t i = 0; i < item.data_size(); i++ )
    {
        data.push_back( item.get<double>( i ) );
    }

    return data;
}

//--------------------------------------------------------------------------------------------------
/// Extract integer array from deck keyword
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<int>, QString> RigPadModel::extractIntArray( RifOpmFlowDeckFile& deckFile, const std::string& keyword )
{
    auto kwOpt = deckFile.findKeyword( keyword );
    if ( !kwOpt.has_value() )
    {
        return std::unexpected( QString( "Keyword %1 not found" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& kw = kwOpt.value();
    if ( kw.size() == 0 )
    {
        return std::unexpected( QString( "Keyword %1 has no records" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& record = kw.getRecord( 0 );
    if ( record.size() == 0 )
    {
        return std::unexpected( QString( "Keyword %1 record has no items" ).arg( QString::fromStdString( keyword ) ) );
    }

    const auto& item = record.getItem( 0 );

    std::vector<int> data;
    data.reserve( item.data_size() );

    for ( size_t i = 0; i < item.data_size(); i++ )
    {
        data.push_back( item.get<int>( i ) );
    }

    return data;
}
