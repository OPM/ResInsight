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

#include "cafPdmUiDoubleSliderEditor.h"
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
    m_bandOpacity.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_bandPolygonOffsetUnit,
                       "BandPolygonOffsetUnit",
                       -5.0,
                       "Depth Offset",
                       "",
                       "Larger Value Closer to Camera",
                       "" );
    m_bandPolygonOffsetUnit.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_surfaces, "Surfaces", "Band Surfaces", "", "", "" );
    m_surfaces.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name", "", "", "" );
    m_nameProxy.registerGetMethod( this, &RimSurfaceIntersectionBand::objectName );
    m_nameProxy.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::setSurfaces( RimSurface* surface1, RimSurface* surface2 )
{
    m_surfaces.clear();

    m_surfaces.push_back( surface1 );
    m_surfaces.push_back( surface2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::setBandColor( const cvf::Color3f& color )
{
    m_bandColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::setBandOpacity( double opacity )
{
    m_bandOpacity = opacity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionBand::setPolygonOffsetUnit( double offset )
{
    m_bandPolygonOffsetUnit = offset;
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
double RimSurfaceIntersectionBand::polygonOffsetUnit() const
{
    // The value in user interface is [0..1]
    // An offsetUnitValue in the range -5..-1000 seems to give good visual results

    const double minimumValue = 5.0;
    auto         value        = minimumValue + m_bandPolygonOffsetUnit * 1000.0;

    return -value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceIntersectionBand::surface1() const
{
    auto surfaces = m_surfaces.ptrReferencedObjects();
    if ( !surfaces.empty() ) return surfaces[0];

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceIntersectionBand::surface2() const
{
    auto surfaces = m_surfaces.ptrReferencedObjects();
    if ( surfaces.size() > 1 ) return surfaces[1];

    return nullptr;
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
            // Keep first and last surface to make it possible to select a new surface by using a single click
            auto firstAndLast = { surfaces.front(), surfaces.back() };
            surfaces          = firstAndLast;
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
void RimSurfaceIntersectionBand::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_bandPolygonOffsetUnit )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 1.0;
        }
    }
    else if ( field == &m_bandOpacity )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0.0;
            myAttr->m_maximum = 1.0;
        }
    }
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

        if ( surfaces[0] )
            text += "( " + surfaces[0]->userDescription() + " - " + surfaces[1]->userDescription() + " )";
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
