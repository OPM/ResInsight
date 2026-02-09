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

#include "RimWellLogWellPathAttributeSettings.h"

#include "RimTools.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathAttributeCollection.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellLogWellPathAttributeSettings, "RimWellLogWellPathAttributeSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogWellPathAttributeSettings::RimWellLogWellPathAttributeSettings()
{
    CAF_PDM_InitObject( "Well Path Attribute Settings" );

    CAF_PDM_InitField( &m_showWellPathAttributes, "ShowWellPathAttributes", false, "Show Well Attributes" );
    CAF_PDM_InitField( &m_wellPathAttributesInLegend, "WellPathAttributesInLegend", true, "Attributes in Legend" );
    CAF_PDM_InitField( &m_showWellPathCompletions, "ShowWellPathCompletions", true, "Show Well Completions" );
    CAF_PDM_InitField( &m_wellPathCompletionsInLegend, "WellPathCompletionsInLegend", true, "Completions in Legend" );
    CAF_PDM_InitField( &m_showWellPathComponentsBothSides, "ShowWellPathAttrBothSides", true, "Show Both Sides" );
    CAF_PDM_InitField( &m_showWellPathComponentLabels, "ShowWellPathAttrLabels", false, "Show Labels" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathComponentSource, "AttributesWellPathSource", "Well Path" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathAttributeCollection, "AttributesCollection", "Well Attributes" );

    CAF_PDM_InitField( &m_overburdenHeight, "OverburdenHeight", 0.0, "Overburden Height" );
    m_overburdenHeight.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_underburdenHeight, "UnderburdenHeight", 0.0, "Underburden Height" );
    m_underburdenHeight.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogWellPathAttributeSettings::~RimWellLogWellPathAttributeSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showWellPathAttributes() const
{
    return m_showWellPathAttributes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowWellPathAttributes( bool show )
{
    m_showWellPathAttributes = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showWellPathCompletions() const
{
    return m_showWellPathCompletions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowWellPathCompletions( bool show )
{
    m_showWellPathCompletions = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showBothSides() const
{
    return m_showWellPathComponentsBothSides();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowBothSides( bool show )
{
    m_showWellPathComponentsBothSides = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showComponentLabels() const
{
    return m_showWellPathComponentLabels();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowComponentLabels( bool show )
{
    m_showWellPathComponentLabels = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showAttributesInLegend() const
{
    return m_wellPathAttributesInLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowAttributesInLegend( bool show )
{
    m_wellPathAttributesInLegend = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWellPathAttributeSettings::showCompletionsInLegend() const
{
    return m_wellPathCompletionsInLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setShowCompletionsInLegend( bool show )
{
    m_wellPathCompletionsInLegend = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogWellPathAttributeSettings::wellPathComponentSource() const
{
    return m_wellPathComponentSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setWellPathComponentSource( RimWellPath* wellPath )
{
    m_wellPathComponentSource = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCollection* RimWellLogWellPathAttributeSettings::wellPathAttributeCollection() const
{
    return m_wellPathAttributeCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setWellPathAttributeCollection( RimWellPathAttributeCollection* collection )
{
    m_wellPathAttributeCollection = collection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogWellPathAttributeSettings::overburdenHeight() const
{
    return m_overburdenHeight();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setOverburdenHeight( double height )
{
    m_overburdenHeight = height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogWellPathAttributeSettings::underburdenHeight() const
{
    return m_underburdenHeight();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::setUnderburdenHeight( double height )
{
    m_underburdenHeight = height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showWellPathAttributes );
    uiOrdering.add( &m_showWellPathCompletions );
    uiOrdering.add( &m_wellPathAttributesInLegend );
    uiOrdering.add( &m_wellPathCompletionsInLegend );
    uiOrdering.add( &m_showWellPathComponentsBothSides );
    uiOrdering.add( &m_showWellPathComponentLabels );
    uiOrdering.add( &m_wellPathComponentSource );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWellPathAttributeSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                            const QVariant&            oldValue,
                                                            const QVariant&            newValue )
{
    auto track = firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( track )
    {
        track->loadDataAndUpdate();
        track->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogWellPathAttributeSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellPathComponentSource )
    {
        RimTools::wellPathOptionItems( &options );
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}
