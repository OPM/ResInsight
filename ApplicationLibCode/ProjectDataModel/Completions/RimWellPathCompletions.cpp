/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellPathCompletions.h"

#include "RiaStdStringTools.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimWellPath.h"
#include "RimWellPathComponentInterface.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathValve.h"

#include "cvfAssert.h"

#include "cafPdmDoubleStringValidator.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimWellPathCompletions, "WellPathCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletions::RimWellPathCompletions()
{
    CAF_PDM_InitScriptableObject( "Completions", ":/CompletionsSymbol16x16.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_perforationCollection, "Perforations", "Perforations" );
    m_perforationCollection = new RimPerforationCollection;

    CAF_PDM_InitScriptableFieldNoDefault( &m_fishbonesCollection, "Fishbones", "Fishbones" );
    m_fishbonesCollection = new RimFishbonesCollection;

    CAF_PDM_InitFieldNoDefault( &m_fractureCollection, "Fractures", "Fractures" );
    m_fractureCollection = new RimWellPathFractureCollection;

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModelCollection, "StimPlanModels", "StimPlan Models" );
    m_stimPlanModelCollection = new RimStimPlanModelCollection;

    CAF_PDM_InitField( &m_wellNameForExport_OBSOLETE, "WellNameForExport", QString(), "Well Name" );
    m_wellNameForExport_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_wellGroupName_OBSOLETE, "WellGroupNameForExport", QString(), "Well Group Name" );
    m_wellGroupName_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_referenceDepth_OBSOLETE, "ReferenceDepthForExport", QString(), "Reference Depth for BHP" );
    m_referenceDepth_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitFieldNoDefault( &m_preferredFluidPhase_OBSOLETE, "WellTypeForExport", "Preferred Fluid Phase" );
    m_preferredFluidPhase_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_drainageRadiusForPI_OBSOLETE, "DrainageRadiusForPI", QString( "0.0" ), "Drainage Radius for PI" );
    m_drainageRadiusForPI_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitFieldNoDefault( &m_gasInflowEquation_OBSOLETE, "GasInflowEq", "Gas Inflow Equation" );
    m_gasInflowEquation_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitFieldNoDefault( &m_automaticWellShutIn_OBSOLETE, "AutoWellShutIn", "Automatic well shut-in" );
    m_automaticWellShutIn_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_allowWellCrossFlow_OBSOLETE, "AllowWellCrossFlow", true, "Allow Well Cross-Flow" );
    m_allowWellCrossFlow_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_wellBoreFluidPVTTable_OBSOLETE, "WellBoreFluidPVTTable", 0, "Wellbore Fluid PVT table" );
    m_wellBoreFluidPVTTable_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitFieldNoDefault( &m_hydrostaticDensity_OBSOLETE, "HydrostaticDensity", "Hydrostatic Density" );
    m_hydrostaticDensity_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_fluidInPlaceRegion_OBSOLETE, "FluidInPlaceRegion", 0, "Fluid In-Place Region" );
    m_fluidInPlaceRegion_OBSOLETE.xmlCapability()->setIOWritable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RimWellPathCompletions::fishbonesCollection() const
{
    CVF_ASSERT( m_fishbonesCollection );

    return m_fishbonesCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RimWellPathCompletions::perforationCollection() const
{
    CVF_ASSERT( m_perforationCollection );

    return m_perforationCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RimWellPathCompletions::fractureCollection() const
{
    CVF_ASSERT( m_fractureCollection );

    return m_fractureCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCollection* RimWellPathCompletions::stimPlanModelCollection() const
{
    CVF_ASSERT( m_stimPlanModelCollection );

    return m_stimPlanModelCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathValve*> RimWellPathCompletions::valves() const
{
    return descendantsIncludingThisOfType<RimWellPathValve>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathCompletions::allFractures() const
{
    if ( m_fractureCollection->isChecked() )
    {
        return m_fractureCollection->allFractures();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathCompletions::activeFractures() const
{
    if ( m_fractureCollection->isChecked() )
    {
        return m_fractureCollection->activeFractures();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathComponentInterface*> RimWellPathCompletions::allCompletionsNoConst() const
{
    std::vector<RimWellPathComponentInterface*> completions;

    for ( RimWellPathFracture* fracture : m_fractureCollection->allFractures() )
    {
        completions.push_back( fracture );
    }
    for ( RimFishbones* fishbones : m_fishbonesCollection->allFishbonesSubs() )
    {
        completions.push_back( fishbones );
    }
    for ( RimPerforationInterval* perforation : m_perforationCollection->perforationsNoConst() )
    {
        completions.push_back( perforation );
    }

    std::vector<RimWellPathValve*> allValves = valves();
    for ( RimWellPathValve* valve : allValves )
    {
        completions.push_back( valve );
    }

    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimWellPathComponentInterface*> RimWellPathCompletions::allCompletions() const
{
    std::vector<const RimWellPathComponentInterface*> completions;

    for ( const RimWellPathFracture* fracture : m_fractureCollection->allFractures() )
    {
        completions.push_back( fracture );
    }
    for ( const RimFishbones* fishbones : m_fishbonesCollection->allFishbonesSubs() )
    {
        completions.push_back( fishbones );
    }
    for ( const RimPerforationInterval* perforation : m_perforationCollection->perforations() )
    {
        completions.push_back( perforation );
    }

    std::vector<RimWellPathValve*> allValves = valves();
    for ( const RimWellPathValve* valve : allValves )
    {
        completions.push_back( valve );
    }

    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathCompletions::hasCompletions() const
{
    if ( !m_fractureCollection->allFractures().empty() || !m_stimPlanModelCollection->allStimPlanModels().empty() )
    {
        return true;
    }

    return !m_fishbonesCollection->allFishbonesSubs().empty() || !m_perforationCollection->perforations().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::setUnitSystemSpecificDefaults()
{
    m_fishbonesCollection->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );

    if ( !m_perforationCollection->perforations().empty() )
    {
        uiTreeOrdering.add( &m_perforationCollection );
    }

    if ( !m_fishbonesCollection->allFishbonesSubs().empty() )
    {
        uiTreeOrdering.add( &m_fishbonesCollection );
    }

    if ( !m_fractureCollection->allFractures().empty() )
    {
        uiTreeOrdering.add( &m_fractureCollection );
    }

    if ( !m_stimPlanModelCollection->allStimPlanModels().empty() )
    {
        uiTreeOrdering.add( &m_stimPlanModelCollection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2020.10.1.2" ) )
    {
        std::vector<RimWellPath*> wellPathHierarchy = allAncestorsOrThisOfType<RimWellPath>();
        RimWellPath*              topLevelWellPath  = wellPathHierarchy.back();
        auto                      settings          = topLevelWellPath->completionSettings();

        applyToSettings( settings );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::applyToSettings( gsl::not_null<RimWellPathCompletionSettings*> settings )
{
    settings->m_wellNameForExport     = m_wellNameForExport_OBSOLETE;
    settings->m_referenceDepth        = m_referenceDepth_OBSOLETE;
    settings->m_preferredFluidPhase   = m_preferredFluidPhase_OBSOLETE;
    settings->m_drainageRadiusForPI   = m_drainageRadiusForPI_OBSOLETE;
    settings->m_gasInflowEquation     = m_gasInflowEquation_OBSOLETE;
    settings->m_automaticWellShutIn   = m_automaticWellShutIn_OBSOLETE;
    settings->m_allowWellCrossFlow    = m_allowWellCrossFlow_OBSOLETE;
    settings->m_wellBoreFluidPVTTable = m_wellBoreFluidPVTTable_OBSOLETE;
    settings->m_hydrostaticDensity    = m_hydrostaticDensity_OBSOLETE;
    settings->m_fluidInPlaceRegion    = m_fluidInPlaceRegion_OBSOLETE;
}
