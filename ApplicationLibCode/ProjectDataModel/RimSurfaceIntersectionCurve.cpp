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

    CAF_PDM_InitFieldNoDefault( &m_surface1, "Surface1", "Surface 1" );
    m_surface1.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name" );
    m_nameProxy.registerGetMethod( this, &RimSurfaceIntersectionCurve::objectName );
    m_nameProxy.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceIntersectionCurve::surface() const
{
    return m_surface1();
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
caf::PdmFieldHandle* RimSurfaceIntersectionCurve::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    onObjectChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSurfaceIntersectionCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_surface1 )
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
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Line Appearance" );
    m_lineAppearance->uiOrdering( uiConfigName, *group );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::onObjectChanged( const caf::SignalEmitter* emitter )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceIntersectionCurve::objectName() const
{
    if ( m_surface1() )
    {
        auto ensembleSurface = m_surface1()->firstAncestorOfType<RimEnsembleSurface>();
        if ( ensembleSurface )
        {
            QString text;
            text += ensembleSurface->collectionName();
            text += "( " + m_surface1()->fullName() + " )";
            return text;
        }

        return m_surface1()->fullName();
    }

    return "Surface Curve";
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
