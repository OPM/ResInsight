/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimPolylinesAnnotation.h"

#include "QFile"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationLineAppearance.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"

#include "QFileInfo"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimPolylinesAnnotation, "RimPolylinesAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::RimPolylinesAnnotation()
{
    CAF_PDM_InitObject( "PolylineAnnotation", ":/WellCollection.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Is Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_closePolyline, "ClosePolyline", false, "Close Polyline", "", "", "" );
    CAF_PDM_InitField( &m_showLines, "ShowLines", true, "Show Lines", "", "", "" );
    CAF_PDM_InitField( &m_showSpheres, "ShowSpheres", false, "Show Spheres", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_appearance, "Appearance", "Appearance", "", "", "" );

    m_appearance = new RimPolylineAppearance();
    m_appearance.uiCapability()->setUiTreeHidden( true );
    m_appearance.uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::~RimPolylinesAnnotation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::isVisible()
{
    RimAnnotationCollectionBase* coll;
    firstAncestorOrThisOfType( coll );

    return coll && coll->isActive() && m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::closePolyline() const
{
    return m_closePolyline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::showLines() const
{
    return m_showLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::showSpheres() const
{
    return m_showSpheres;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineAppearance* RimPolylinesAnnotation::appearance() const
{
    return m_appearance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPolylinesAnnotation::objectToggleField()
{
    return &m_isActive;
}
