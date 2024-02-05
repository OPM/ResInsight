/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonInView.h"
#include "Rim3dView.h"
#include "RimPolygon.h"
#include "RimTools.h"

#include "RivPolylinePartMgr.h"

#include "cafDisplayCoordTransform.h"

#include "RigPolyLinesData.h"
#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimPolygonInView, "RimPolygonInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView::RimPolygonInView()

{
    CAF_PDM_InitObject( "Polygon", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygon, "Polygon", "Polygon" );
    m_polygon.uiCapability()->setUiReadOnly( true );

    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView::~RimPolygonInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimPolygonInView::polygon() const
{
    return m_polygon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::setPolygon( RimPolygon* polygon )
{
    m_polygon = polygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::appendPartsToModel( cvf::ModelBasicList*        model,
                                           caf::DisplayCoordTransform* scaleTransform,
                                           const cvf::BoundingBox&     boundingBox )
{
    auto view = firstAncestorOfType<Rim3dView>();

    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, this, this );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, scaleTransform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimPolygonInView::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;

    if ( m_polygon )
    {
        pld->setPolyLine( polygon()->pointsInDomainCoords() );
    }

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    updateNameField();

    uiOrdering.add( m_polygon );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonInView::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_polygon )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        RimTools::polygonOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updateNameField()
{
    QString name = "Undefined";
    if ( m_polygon() )
    {
        name = m_polygon->name();
    }

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    updateNameField();
}
