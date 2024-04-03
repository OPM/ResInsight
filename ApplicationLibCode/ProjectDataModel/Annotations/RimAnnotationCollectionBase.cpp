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

#include "RimAnnotationCollectionBase.h"

#include "RimPolylinesAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimTextAnnotation.h"

#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimGridView.h"
#include "RimProject.h"

#include <QString>

CAF_PDM_SOURCE_INIT( RimAnnotationCollectionBase, "RimAnnotationCollectionBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase::RimAnnotationCollectionBase()
{
    CAF_PDM_InitObject( "Annotations", ":/WellCollection.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Is Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_textAnnotations, "TextAnnotations", "Text Annotations" );

    m_textAnnotations = new RimAnnotationGroupCollection();
    m_textAnnotations->uiCapability()->setUiName( RimAnnotationGroupCollection::TEXT_ANNOTATION_UI_NAME );
    m_textAnnotations->uiCapability()->setUiIconFromResourceString( ":/TextAnnotation16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase::~RimAnnotationCollectionBase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationCollectionBase::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::addAnnotation( RimTextAnnotation* annotation )
{
    m_textAnnotations->addAnnotation( annotation );
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::removeAnnotation( RimTextAnnotation* annotation )
{
    m_textAnnotations->removeAnnotation( annotation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimAnnotationCollectionBase::textAnnotations() const
{
    std::vector<RimTextAnnotation*> annotations;
    for ( auto& a : m_textAnnotations->annotations() )
    {
        annotations.push_back( dynamic_cast<RimTextAnnotation*>( a ) );
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::updateViewAnnotationCollections()
{
    // Default implementation: No op
}

//--------------------------------------------------------------------------------------------------
/// At least one annotation have been deleted. Typically by the generic delete command
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::onAnnotationDeleted()
{
    scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::scheduleRedrawOfRelevantViews()
{
    auto views = viewsContainingAnnotations();
    if ( !views.empty() )
    {
        for ( auto& view : views )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimAnnotationCollectionBase::viewsContainingAnnotations() const
{
    RimProject* project = RimProject::current();
    if ( !project ) return {};

    return project->allViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isActive )
    {
        updateUiIconFromToggleField();
        scheduleRedrawOfRelevantViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnnotationCollectionBase::objectToggleField()
{
    return &m_isActive;
}
