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

#include "RimSurfaceIntersectionCurve.h"

#include "RimAnnotationLineAppearance.h"
#include "RimEnsembleSurface.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <QStringList>

CAF_PDM_SOURCE_INIT( RimSurfaceIntersectionCurve, "RimSurfaceIntersectionCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCurve::RimSurfaceIntersectionCurve()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "SurfaceIntersectionCurve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_lineAppearance, "LineAppearance", "Line Appearance" );
    m_lineAppearance = new RimAnnotationLineAppearance;
    m_lineAppearance->objectChanged.connect( this, &RimSurfaceIntersectionCurve::onObjectChanged );
    uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_surfaces, "Surfaces", "Surfaces" );
    m_surfaces.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_surfaces.registerKeywordAlias( "Surface1" );

    CAF_PDM_InitField( &m_useCustomColor, "UseCustomColor", false, "Custom Color" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name" );
    m_nameProxy.registerGetMethod( this, &RimSurfaceIntersectionCurve::objectName );
    m_nameProxy.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimSurfaceIntersectionCurve::surfaces() const
{
    return m_surfaces.ptrReferencedObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance* RimSurfaceIntersectionCurve::lineAppearance() const
{
    return m_lineAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSurfaceIntersectionCurve::colorForSurface( const RimSurface* surface ) const
{
    if ( !m_useCustomColor() && surface ) return surface->color();

    return m_lineAppearance->color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceIntersectionCurve::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_surfaces || changedField == &m_useCustomColor )
    {
        updateColorFromSurface();
        updateConnectedEditors();
    }

    onObjectChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSurfaceIntersectionCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_surfaces )
    {
        RimSurfaceCollection* surfColl = RimTools::surfaceCollection();

        appendOptionItemsForSources( 0, surfColl, true, options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_surfaces );

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Line Appearance" );
    group->add( &m_useCustomColor );

    // The color defined in the Surfaces collection is used unless the user asks for a custom color
    updateColorFromSurface();
    m_lineAppearance->setColorReadOnly( !m_useCustomColor() );
    m_lineAppearance->uiOrdering( uiConfigName, *group );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::initAfterRead()
{
    updateColorFromSurface();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::onObjectChanged( const caf::SignalEmitter* emitter )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Keep the color of the line appearance in sync with the first surface, so the read-only color field
/// shows the color used to draw the curve
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::updateColorFromSurface()
{
    if ( m_useCustomColor() ) return;

    auto surfaces = m_surfaces.ptrReferencedObjectsByType();
    if ( surfaces.empty() || !surfaces.front() ) return;

    m_lineAppearance->setColor( surfaces.front()->color() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceIntersectionCurve::objectName() const
{
    auto nameForSurface = []( const RimSurface* surface ) -> QString
    {
        auto ensembleSurface = surface->firstAncestorOfType<RimEnsembleSurface>();
        if ( ensembleSurface )
        {
            return ensembleSurface->collectionName() + "( " + surface->fullName() + " )";
        }

        return surface->fullName();
    };

    QStringList names;
    for ( auto surface : m_surfaces.ptrReferencedObjectsByType() )
    {
        if ( surface ) names.push_back( nameForSurface( surface ) );
    }

    if ( names.isEmpty() ) return "Surface Curve";

    return names.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::appendOptionItemsForSources( int                            currentLevel,
                                                               RimSurfaceCollection*          currentCollection,
                                                               bool                           showEnsembleSurfaces,
                                                               QList<caf::PdmOptionItemInfo>& options )
{
    if ( !currentCollection ) return;

    caf::IconProvider surfaceIcon( ":/ReservoirSurface16x16.png" );

    options.push_back( caf::PdmOptionItemInfo::createHeader( currentCollection->collectionName(), true ) );

    for ( auto surf : currentCollection->surfaces() )
    {
        auto itemInfo = caf::PdmOptionItemInfo( surf->fullName(), surf, false, surfaceIcon );
        itemInfo.setLevel( currentLevel + 1 );
        options.push_back( itemInfo );
    }

    auto ensembleSurface = dynamic_cast<RimEnsembleSurface*>( currentCollection );
    if ( !ensembleSurface || ( showEnsembleSurfaces && ensembleSurface ) )
    {
        for ( auto subColl : currentCollection->subCollections() )
        {
            appendOptionItemsForSources( currentLevel, subColl, showEnsembleSurfaces, options );
        }
    }
}
