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

#include "RimTextAnnotation.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationTextAppearance.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "AnnotationCommands/RicTextAnnotation3dEditor.h"
#include "RicVec3dPickEventHandler.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiPickableLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <QAction>
#include <QPointer>

CAF_PDM_SOURCE_INIT( RimTextAnnotation, "RimTextAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTextAnnotation::RimTextAnnotation()
{
    CAF_PDM_InitObject( "TextAnnotation", ":/TextAnnotation16x16.png", "", "" );
    this->setUi3dEditorTypeName( RicTextAnnotation3dEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_anchorPointXyd, "AnchorPointXyd", Vec3d::ZERO, "Anchor Point", "", "", "" );
    m_anchorPointXyd.uiCapability()->setUiEditorTypeName( caf::PdmUiPickableLineEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_anchorPointPickEnabledButtonField, "AnchorPointPick", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_anchorPointPickEnabledButtonField );
    m_anchorPointPickEnabledButtonField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitField( &m_labelPointXyd, "LabelPointXyd", Vec3d::ZERO, "Label Point", "", "", "" );
    m_labelPointXyd.uiCapability()->setUiEditorTypeName( caf::PdmUiPickableLineEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_labelPointPickEnabledButtonField, "LabelPointPick", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_labelPointPickEnabledButtonField );
    m_labelPointPickEnabledButtonField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitField( &m_text, "Text", QString( "(New text)" ), "Text", "", "", "" );
    m_text.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Is Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_textAppearance, "TextAppearance", "Text Appearance", "", "", "" );
    m_textAppearance = new RimAnnotationTextAppearance();
    m_textAppearance.uiCapability()->setUiTreeHidden( true );
    m_textAppearance.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy", "", "", "" );
    m_nameProxy.registerGetMethod( this, &RimTextAnnotation::extractNameFromText );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.xmlCapability()->disableIO();

    m_anchorPointPickEventHandler.reset( new RicVec3dPickEventHandler( &m_anchorPointXyd ) );
    m_labelPointPickEventHandler.reset( new RicVec3dPickEventHandler( &m_labelPointXyd, 0.1 ) );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTextAnnotation::~RimTextAnnotation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimTextAnnotation::anchorPoint() const
{
    auto pos = m_anchorPointXyd();
    pos.z()  = -pos.z();
    return pos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::setAnchorPoint( const Vec3d& pointXyz )
{
    m_anchorPointXyd = pointXyz;
    m_anchorPointXyd.v().z() *= -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimTextAnnotation::labelPoint() const
{
    auto pos = m_labelPointXyd();
    pos.z()  = -pos.z();
    return pos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::setLabelPoint( const Vec3d& pointXyz )
{
    m_labelPointXyd = pointXyz;
    m_labelPointXyd.v().z() *= -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::setText( const QString& text )
{
    m_text = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimTextAnnotation::text() const
{
    return m_text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_anchorPointXyd );
    uiOrdering.add( &m_anchorPointPickEnabledButtonField, false );
    uiOrdering.add( &m_labelPointXyd );
    uiOrdering.add( &m_labelPointPickEnabledButtonField, false );

    uiOrdering.add( &m_text );

    auto appearanceGroup = uiOrdering.addNewGroup( "Text Appearance" );
    m_textAppearance->uiOrdering( uiConfigName, *appearanceGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( changedField == &m_anchorPointXyd )
    {
        m_anchorPointPickEnabledButtonField = false;
        if ( m_labelPointXyd().isZero() )
        {
            m_labelPointXyd = m_anchorPointXyd;
        }
        this->updateConnectedEditors();
    }
    if ( changedField == &m_labelPointXyd )
    {
        m_labelPointPickEnabledButtonField = false;
        this->updateConnectedEditors();
    }
    if ( changedField == &m_anchorPointPickEnabledButtonField || changedField == &m_labelPointPickEnabledButtonField )
    {
        this->updateConnectedEditors();
    }

    RimAnnotationCollectionBase* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( annColl );

    if ( annColl ) annColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTextAnnotation::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTextAnnotation::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
/// Returns true if annotation can be displayed due to all toggles that affect this annotation
//--------------------------------------------------------------------------------------------------
bool RimTextAnnotation::isVisible() const
{
    RimAnnotationGroupCollection* coll;
    firstAncestorOrThisOfType( coll );

    bool visible = true;
    if ( coll ) visible = coll->isVisible();
    if ( visible ) visible = m_isActive;

    return visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::enablePicking( bool enable )
{
    m_anchorPointPickEnabledButtonField = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationTextAppearance* RimTextAnnotation::appearance() const
{
    return m_textAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTextAnnotation::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiPickableLineEditorAttribute* attr = dynamic_cast<caf::PdmUiPickableLineEditorAttribute*>( attribute );

    if ( attr && field == &m_anchorPointXyd )
    {
        attr->pickEventHandler = m_anchorPointPickEventHandler;
        attr->enablePicking    = m_anchorPointPickEnabledButtonField;
    }
    else if ( attr && field == &m_labelPointXyd )
    {
        attr->pickEventHandler = m_labelPointPickEventHandler;
        attr->enablePicking    = m_labelPointPickEnabledButtonField;
    }

    if ( field == &m_anchorPointPickEnabledButtonField || field == &m_labelPointPickEnabledButtonField )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            auto boolField = static_cast<const caf::PdmField<bool>*>( field );
            if ( boolField->v() )
            {
                pbAttribute->m_buttonText = "Stop";
            }
            else
            {
                pbAttribute->m_buttonText = "Pick";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTextAnnotation::extractNameFromText() const
{
    return m_text().split( "\n" ).front();
}
