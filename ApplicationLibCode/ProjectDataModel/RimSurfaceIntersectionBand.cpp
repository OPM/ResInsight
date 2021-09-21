////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSurfaceIntersectionBand.h"

#include "RimAnnotationLineAppearance.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceIntersectionCurve.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSurfaceIntersectionBand, "RimSurfaceIntersectionBand" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceIntersectionBand::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionBand::RimSurfaceIntersectionBand()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "SurfaceIntersectionBand", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_lineAppearance, "LineAppearance", "Line Appearance", "", "", "" );
    m_lineAppearance = new RimAnnotationLineAppearance;
    m_lineAppearance->objectChanged.connect( this, &RimSurfaceIntersectionBand::onObjectChanged );
    uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_bandColor, "BandColor", cvf::Color3f( cvf::Color3f::BLACK ), "Band Color", "", "", "" );
    CAF_PDM_InitField( &m_bandOpacity, "BandOpacity", 0.8, "Band Opacity", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_surfaces, "Surfaces", "Band Surfaces", "", "", "" );
    m_surfaces.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name", "", "", "" );
    m_nameProxy.registerGetMethod( this, &RimSurfaceIntersectionBand::objectName );
    m_nameProxy.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance* RimSurfaceIntersectionBand::lineAppearance() const
{
    return m_lineAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSurfaceIntersectionBand::bandColor() const
{
    return m_bandColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimSurfaceIntersectionBand::bandOpacity() const
{
    return m_bandOpacity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<RimSurface*, 2> RimSurfaceIntersectionBand::surfaces() const
{
    std::array<RimSurface*, 2> surfs;
    surfs[0] = nullptr;
    surfs[1] = nullptr;

    auto surfaces = m_surfaces.ptrReferencedObjects();

    if ( surfaces.size() > 0 ) surfs[0] = surfaces[0];
    if ( surfaces.size() > 1 ) surfs[1] = surfaces[1];

    return surfs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    if ( changedField == &m_surfaces )
    {
        auto surfaces = m_surfaces.ptrReferencedObjects();
        if ( surfaces.size() > 2 )
        {
            surfaces.erase( surfaces.begin() + 1, surfaces.end() - 1 );
        }

        m_surfaces.setValue( surfaces );
    }

    onObjectChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSurfaceIntersectionBand::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_surfaces )
    {
        RimSurfaceCollection* surfColl = RimProject::current()->activeOilField()->surfaceCollection();

        RimSurfaceIntersectionCurve::appendOptionItemsForSources( 0, surfColl, false, options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::onObjectChanged( const caf::SignalEmitter* emitter )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceIntersectionBand::objectName() const
{
    auto surfaces = m_surfaces.ptrReferencedObjects();

    if ( surfaces.size() == 2 )
    {
        QString text;

        auto                  firstSurface = surfaces[0];
        RimSurfaceCollection* surfColl     = nullptr;
        firstSurface->firstAncestorOfType( surfColl );
        if ( surfColl )
        {
            text += surfColl->collectionName();
        }

        if ( surfaces[0] ) text += "(" + surfaces[0]->userDescription() + " - " + surfaces[1]->userDescription() + ")";
        return text;
    }

    return "Surface Band";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Band Appearance" );
        group->add( &m_bandColor );
        group->add( &m_bandOpacity );
    }
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Line Appearance" );
        m_lineAppearance->uiOrdering( uiConfigName, *group );
    }
}
