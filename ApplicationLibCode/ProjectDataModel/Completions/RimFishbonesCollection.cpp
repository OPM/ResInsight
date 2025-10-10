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

#include "Well/RigWellPath.h"

#include "RimFishbones.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QColor>

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimFishbonesCollection, "FishbonesCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Fishbones", ":/FishBones16x16.png", "", "", "FishbonesCollection", "" );

    nameField()->uiCapability()->setUiHidden( true );
    setName( "Fishbones" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_fishbones, "FishbonesSubs", "fishbones", "fishbonesSubs" );

    // The following non-scriptable fields can be modified from scripting functions in RimcFishbonesCollection.cpp
    CAF_PDM_InitField( &m_startMDAuto, "StartMDAuto", RimFishbonesDefines::ValueSource::AUTOMATIC, "Start MD Mode" );
    CAF_PDM_InitField( &m_startMD, "StartMD", 0.0, "Start [MD]" );

    CAF_PDM_InitField( &m_endMDAuto, "EndMDAuto", RimFishbonesDefines::ValueSource::AUTOMATIC, "End MD Mode" );
    CAF_PDM_InitField( &m_endMD, "EndMD", 0.0, "End [MD]" );

    CAF_PDM_InitScriptableField( &m_mainBoreDiameter, "MainBoreDiameter", 0.216, "Main Bore Diameter" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "MainBoreSkinFactor", 0., "Main Bore Skin Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimProject* proj = RimProject::current();
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
    computeStartAndEndLocation();

    if ( auto wellPath = firstAncestorOrThisOfType<RimWellPath>() )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_startMD.uiCapability()->setUiName( "  Start [MD m]" );
            m_endMD.uiCapability()->setUiName( "  End [MD m]" );
            m_mainBoreDiameter.uiCapability()->setUiName( "Main Bore Diameter [m]" );
        }
        else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_startMD.uiCapability()->setUiName( "  Start [MD ft]" );
            m_endMD.uiCapability()->setUiName( "  End [MD ft]" );
            m_mainBoreDiameter.uiCapability()->setUiName( "Main Bore Diameter [ft]" );
        }
    }

    caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup( "Mainbore Well Properties" );
    wellGroup->add( &m_startMDAuto );
    wellGroup->add( &m_startMD );
    m_startMD.uiCapability()->setUiReadOnly( m_startMDAuto == RimFishbonesDefines::ValueSource::AUTOMATIC );

    wellGroup->add( &m_endMDAuto );
    wellGroup->add( &m_endMD );
    m_endMD.uiCapability()->setUiReadOnly( m_endMDAuto == RimFishbonesDefines::ValueSource::AUTOMATIC );

    wellGroup->add( &m_mainBoreDiameter );
    wellGroup->add( &m_skinFactor );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2024.12.2" ) )
    {
        double epsilon = 1.0;
        if ( std::fabs( calculateStartMD() - m_startMD() ) > epsilon )
        {
            m_startMDAuto = RimFishbonesDefines::ValueSource::FIXED;
        }

        if ( std::fabs( calculateEndMD() - m_endMD() ) > epsilon )
        {
            m_endMDAuto = RimFishbonesDefines::ValueSource::FIXED;
        }
    }

    computeStartAndEndLocation();
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

    computeStartAndEndLocation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbones* RimFishbonesCollection::appendFishbonesSubsAtLocations( const std::vector<double>&        subLocations,
                                                                      RimFishbonesDefines::DrillingType drillingType )
{
    auto* obj = new RimFishbones;
    obj->setValveLocations( subLocations );

    appendFishbonesSubs( obj );

    RimFishbonesDefines::RicFishbonesSystemParameters customParameters = RimFishbonesDefines::drillingStandardParameters();
    if ( drillingType == RimFishbonesDefines::DrillingType::EXTENDED )
    {
        customParameters = RimFishbonesDefines::drillingExtendedParameters();
    }
    else if ( drillingType == RimFishbonesDefines::DrillingType::ACID_JETTING )
    {
        customParameters = RimFishbonesDefines::acidJettingParameters();
    }

    // NB: Setting the system parameters must be done after appendFishbonesSubs, as the latter sets some default values
    obj->setSystemParameters( customParameters.lateralsPerSub,
                              customParameters.lateralLength,
                              customParameters.holeDiameter,
                              customParameters.buildAngle,
                              customParameters.icdsPerSub );

    return obj;
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
    return m_fishbones.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::setFixedStartMD( double startMD )
{
    m_startMD     = startMD;
    m_startMDAuto = RimFishbonesDefines::ValueSource::FIXED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::setFixedEndMD( double endMD )
{
    m_endMD     = endMD;
    m_endMDAuto = RimFishbonesDefines::ValueSource::FIXED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFishbonesCollection::nextFishbonesColor() const
{
    auto          wellPath = firstAncestorOrThisOfType<RimWellPath>();
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
double RimFishbonesCollection::calculateStartMD() const
{
    double minStartMD = HUGE_VAL;

    for ( const RimFishbones* sub : m_fishbones() )
    {
        for ( const auto& subAndLateralIndex : sub->installedLateralIndices() )
        {
            minStartMD = std::min( minStartMD, sub->measuredDepth( subAndLateralIndex.first ) - 13.0 );
        }
    }

    return minStartMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::calculateEndMD() const
{
    double value = -HUGE_VAL;

    for ( const RimFishbones* sub : m_fishbones() )
    {
        value = std::max( value, sub->endMD() );
    }

    return value;
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
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::mainBoreSkinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::mainBoreDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_mainBoreDiameter() );
    }
    else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
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
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::computeStartAndEndLocation()
{
    if ( m_startMDAuto() == RimFishbonesDefines::ValueSource::AUTOMATIC )
    {
        m_startMD = calculateStartMD();
    }

    if ( m_endMDAuto() == RimFishbonesDefines::ValueSource::AUTOMATIC )
    {
        m_endMD = calculateEndMD();
    }
}
