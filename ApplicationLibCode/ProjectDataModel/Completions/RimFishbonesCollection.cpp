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

#include "RimFishbonesCollection.h"

#include "RiaEclipseUnitTools.h"

#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "RimFishbones.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include <QColor>

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimFishbonesCollection, "FishbonesCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitObject( "Fishbones", ":/FishBones16x16.png", "", "" );

    nameField()->uiCapability()->setUiHidden( true );
    this->setName( "Fishbones" );

    CAF_PDM_InitFieldNoDefault( &m_fishbones, "FishbonesSubs", "fishbonesSubs", "", "", "" );

    m_fishbones.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_startMD, "StartMD", HUGE_VAL, "Start MD", "", "", "" );
    CAF_PDM_InitField( &m_mainBoreDiameter, "MainBoreDiameter", 0.216, "Main Bore Diameter", "", "", "" );
    CAF_PDM_InitField( &m_skinFactor, "MainBoreSkinFactor", 0., "Main Bore Skin Factor [0..1]", "", "", "" );
    manuallyModifiedStartMD = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( changedField == &m_startMD )
    {
        manuallyModifiedStartMD = true;
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    if ( changedField == &m_isChecked )
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType( wellPath );
        if ( wellPath )
        {
            if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
            {
                m_startMD.uiCapability()->setUiName( "Start MD [m]" );
                m_mainBoreDiameter.uiCapability()->setUiName( "Main Bore Diameter [m]" );
            }
            else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
            {
                m_startMD.uiCapability()->setUiName( "Start MD [ft]" );
                m_mainBoreDiameter.uiCapability()->setUiName( "Main Bore Diameter [ft]" );
            }
        }
    }

    caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup( "Fishbone Well Properties" );
    wellGroup->add( &m_startMD );
    wellGroup->add( &m_mainBoreDiameter );
    wellGroup->add( &m_skinFactor );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::appendFishbonesSubs( RimFishbones* subs )
{
    subs->fishbonesColor = nextFishbonesColor();
    m_fishbones.push_back( subs );

    subs->setUnitSystemSpecificDefaults();
    subs->recomputeLateralLocations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFishbonesCollection::hasFishbones() const
{
    return !m_fishbones.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFishbones*> RimFishbonesCollection::activeFishbonesSubs() const
{
    std::vector<RimFishbones*> active;

    if ( isChecked() )
    {
        for ( const auto& f : allFishbonesSubs() )
        {
            if ( f->isActive() )
            {
                active.push_back( f );
            }
        }
    }

    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFishbones*> RimFishbonesCollection::allFishbonesSubs() const
{
    return m_fishbones.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFishbonesCollection::nextFishbonesColor() const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType( wellPath );
    cvf::Color3ub wellPathColor( wellPath->wellPathColor() );
    QColor        qWellPathColor = QColor( wellPathColor.r(), wellPathColor.g(), wellPathColor.b() );

    if ( qWellPathColor.value() == 0 )
    {
        // If the color is black, using `lighter` or `darker` will not have any effect, since they multiply `value` by a
        // percentage. In this case, `value` is set specifically to make `lighter`/`darker` possible.
        qWellPathColor.setHsl( qWellPathColor.hue(), qWellPathColor.saturation(), 25 );
    }

    QColor qFishbonesColor;

    int newIndex = static_cast<int>( m_fishbones.size() );

    if ( qWellPathColor.lightnessF() < 0.5 )
    {
        qFishbonesColor = qWellPathColor.lighter( 150 + 50 * newIndex );
    }
    else
    {
        qFishbonesColor = qWellPathColor.darker( 150 + 50 * newIndex );
    }

    return cvf::Color3f::fromByteColor( qFishbonesColor.red(), qFishbonesColor.green(), qFishbonesColor.blue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::recalculateStartMD()
{
    double minStartMD = HUGE_VAL;

    for ( const RimFishbones* sub : m_fishbones() )
    {
        for ( const auto& subAndLateralIndex : sub->installedLateralIndices() )
        {
            minStartMD = std::min( minStartMD, sub->measuredDepth( subAndLateralIndex.first ) - 13.0 );
        }
    }

    if ( !manuallyModifiedStartMD || minStartMD < m_startMD() )
    {
        m_startMD = minStartMD;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::endMD() const
{
    double endMD = m_startMD;
    if ( !m_fishbones.empty() )
    {
        auto lastFishbone = m_fishbones.childObjects().back();
        CVF_ASSERT( lastFishbone );
        endMD = lastFishbone->endMD();
    }
    return endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::mainBoreDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted( wellPath );
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD &&
         unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_mainBoreDiameter() );
    }
    else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC &&
              unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( m_mainBoreDiameter() );
    }
    return m_mainBoreDiameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType( wellPath );
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_mainBoreDiameter = 0.216;
        }
        else
        {
            m_mainBoreDiameter = 0.708;
        }
    }
}
