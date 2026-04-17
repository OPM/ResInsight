/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "RimRefinementRegionCollection.h"

#include "RigNoRefinement.h"
#include "RigNonUniformRefinement.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimRefinementRegion.h"

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimRefinementRegionCollection, "RefinementRegionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRefinementRegionCollection::RimRefinementRegionCollection()
{
    CAF_PDM_InitObject( "Refinement Regions", ":/CellFilter_Range.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Show Regions in 3D View" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_regions, "Regions", "Regions" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRefinementRegionCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRefinementRegion*> RimRefinementRegionCollection::regions() const
{
    return m_regions.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRefinementRegion*> RimRefinementRegionCollection::activeRegions() const
{
    std::vector<RimRefinementRegion*> result;
    for ( auto r : m_regions )
    {
        if ( r && r->isActive() ) result.push_back( r );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRefinementRegion* RimRefinementRegionCollection::addNewRegion( RimEclipseCase* eclipseCase )
{
    auto* region = new RimRefinementRegion();
    region->setRegionName( QString( "Region %1" ).arg( m_regions.size() + 1 ) );
    m_regions.push_back( region );

    // Defaults depend on the case's grid; set after the region is inserted into the tree so
    // that view/case ancestor lookups work inside the region.
    region->setDefaultsFromCase( eclipseCase );

    updateConnectedEditors();
    return region;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegionCollection::addRegion( RimRefinementRegion* region )
{
    if ( region ) m_regions.push_back( region );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegionCollection::removeRegion( RimRefinementRegion* region )
{
    if ( !region ) return;
    m_regions.removeChild( region );
    delete region;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimRefinementRegionCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegionCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( auto view = firstAncestorOrThisOfType<Rim3dView>() )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::unique_ptr<RigRefinement>, QString>
    RimRefinementRegionCollection::combineRefinements( const caf::VecIjk0&                      sectorMin,
                                                       const caf::VecIjk0&                      sectorMax,
                                                       const std::vector<RimRefinementRegion*>& regions )
{
    cvf::Vec3st sectorSize( sectorMax.x() - sectorMin.x() + 1, sectorMax.y() - sectorMin.y() + 1, sectorMax.z() - sectorMin.z() + 1 );

    if ( regions.empty() )
    {
        return std::unique_ptr<RigRefinement>( std::make_unique<RigNoRefinement>( sectorSize ) );
    }

    // Sector bounds, 1-based, for error messages and validation
    int sectorMinI = static_cast<int>( sectorMin.x() ) + 1;
    int sectorMinJ = static_cast<int>( sectorMin.y() ) + 1;
    int sectorMinK = static_cast<int>( sectorMin.z() ) + 1;
    int sectorMaxI = static_cast<int>( sectorMax.x() ) + 1;
    int sectorMaxJ = static_cast<int>( sectorMax.y() ) + 1;
    int sectorMaxK = static_cast<int>( sectorMax.z() ) + 1;

    for ( auto* r : regions )
    {
        if ( !r ) continue;
        auto err = r->validateWithinSector( sectorMinI, sectorMinJ, sectorMinK, sectorMaxI, sectorMaxJ, sectorMaxK );
        if ( !err.isEmpty() ) return std::unexpected( err );
    }

    auto result = std::make_unique<RigNonUniformRefinement>( sectorSize );

    auto fractionsEqual = []( const std::vector<double>& a, const std::vector<double>& b )
    {
        if ( a.size() != b.size() ) return false;
        for ( size_t i = 0; i < a.size(); ++i )
        {
            if ( std::abs( a[i] - b[i] ) > 1e-9 ) return false;
        }
        return true;
    };

    const char*  dimLabel[3]   = { "I", "J", "K" };
    const size_t sectorMin0[3] = { sectorMin.x(), sectorMin.y(), sectorMin.z() };

    // For each dimension, track which region "owns" each sector index so we can give
    // meaningful error messages on overlap.
    std::vector<RimRefinementRegion*>  ownerI( sectorSize.x(), nullptr );
    std::vector<RimRefinementRegion*>  ownerJ( sectorSize.y(), nullptr );
    std::vector<RimRefinementRegion*>  ownerK( sectorSize.z(), nullptr );
    std::vector<RimRefinementRegion*>* owners[3] = { &ownerI, &ownerJ, &ownerK };

    size_t regionStarts[3];
    size_t regionEnds[3];

    for ( auto* region : regions )
    {
        if ( !region ) continue;
        auto regionRefinement = region->effectiveRefinement();
        if ( !regionRefinement ) continue;

        auto regMin = region->ijkMin();
        auto regMax = region->ijkMax();

        regionStarts[0] = regMin.x();
        regionStarts[1] = regMin.y();
        regionStarts[2] = regMin.z();
        regionEnds[0]   = regMax.x();
        regionEnds[1]   = regMax.y();
        regionEnds[2]   = regMax.z();

        for ( size_t d = 0; d < 3; ++d )
        {
            auto dim = static_cast<RigRefinement::Dimension>( d );

            for ( size_t absIdx = regionStarts[d]; absIdx <= regionEnds[d]; ++absIdx )
            {
                size_t sectorRelIdx = absIdx - sectorMin0[d];
                size_t regionRelIdx = absIdx - regionStarts[d];

                const auto& newFracs = regionRefinement->cumulativeFractions( dim, regionRelIdx );

                // If this cell is still default identity {1.0}, accept the region's fractions unconditionally.
                const auto& existing           = result->cumulativeFractions( dim, sectorRelIdx );
                bool        existingIsIdentity = ( existing.size() == 1 && std::abs( existing[0] - 1.0 ) < 1e-12 );

                if ( existingIsIdentity )
                {
                    result->setCumulativeFractions( dim, sectorRelIdx, newFracs );
                    ( *owners[d] )[sectorRelIdx] = region;
                }
                else
                {
                    // Already claimed by another region. Allow if the refinement matches exactly.
                    if ( !fractionsEqual( existing, newFracs ) )
                    {
                        auto* prevOwner = ( *owners[d] )[sectorRelIdx];
                        return std::unexpected( QString( "Refinement regions '%1' and '%2' both refine %3-index %4 with "
                                                         "different subdivisions. Regions must have disjoint projections on each "
                                                         "axis (or identical refinement on shared indices)." )
                                                    .arg( prevOwner ? prevOwner->regionName() : QString( "<unknown>" ) )
                                                    .arg( region->regionName() )
                                                    .arg( dimLabel[d] )
                                                    .arg( static_cast<int>( absIdx ) + 1 ) );
                    }
                }
            }
        }
    }

    if ( !result->hasRefinement() )
    {
        return std::unique_ptr<RigRefinement>( std::make_unique<RigNoRefinement>( sectorSize ) );
    }

    return std::unique_ptr<RigRefinement>( std::move( result ) );
}
