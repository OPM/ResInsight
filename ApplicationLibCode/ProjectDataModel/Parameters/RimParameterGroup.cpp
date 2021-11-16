/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimParameterGroup.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTableViewEditor.h"

#include "RimDoubleParameter.h"
#include "RimGenericParameter.h"
#include "RimIntegerParameter.h"
#include "RimListParameter.h"
#include "RimParameterList.h"
#include "RimStringParameter.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimParameterGroup, "ParameterGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterGroup::RimParameterGroup()
{
    CAF_PDM_InitObject( "Parameter Group", ":/Bullet.png", "", "" );
    uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_parameters, "Parameters", "Parameters" );
    m_parameters.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_parameters.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_parameters.uiCapability()->setCustomContextMenuEnabled( true );
    m_parameters.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    m_name.uiCapability()->setUiHidden( true );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_label, "Label", "Label" );
    m_label.uiCapability()->setUiHidden( true );
    m_label.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_comment, "Comment", "Comment" );
    m_comment.uiCapability()->setUiHidden( true );
    m_comment.uiCapability()->setUiReadOnly( true );
    m_comment.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_showExpanded, "Expanded", "Expanded" );
    m_showExpanded.uiCapability()->setUiHidden( true );
    m_showExpanded.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_labelProxy, "LabelProxy", "Label Proxy" );
    m_labelProxy.registerGetMethod( this, &RimParameterGroup::labelOrName );
    m_labelProxy.uiCapability()->setUiReadOnly( true );
    m_labelProxy.uiCapability()->setUiHidden( true );
    m_labelProxy.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_lists, "ParameterLists", "Parameter Lists" );
    m_lists.uiCapability()->setUiHidden( true );
    m_lists.uiCapability()->setUiTreeHidden( true );
    m_lists.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterGroup::~RimParameterGroup()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimParameterGroup::userDescriptionField()
{
    return &m_labelProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterGroup::labelOrName() const
{
    if ( m_label().isEmpty() ) return m_name;
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::addParameter( RimGenericParameter* parameter )
{
    m_parameters.push_back( parameter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::addParameter( QString name, int value )
{
    RimIntegerParameter* p = new RimIntegerParameter();
    p->setName( name );
    p->setLabel( name );
    p->setValue( value );

    addParameter( p );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::addParameter( QString name, QString value )
{
    RimStringParameter* p = new RimStringParameter();
    p->setName( name );
    p->setLabel( name );
    p->setValue( value );

    addParameter( p );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::addParameter( QString name, double value )
{
    RimDoubleParameter* p = new RimDoubleParameter();
    p->setName( name );
    p->setLabel( name );
    p->setValue( value );

    addParameter( p );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::addList( RimParameterList* paramList )
{
    m_lists.push_back( paramList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::appendParametersToList( std::list<RimGenericParameter*>& parameterList )
{
    for ( auto p : m_parameters() )
    {
        parameterList.push_back( p );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_parameters )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( label() );
    if ( !m_comment().isEmpty() ) group->add( &m_comment );
    group->add( &m_parameters );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setName( QString name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setLabel( QString label )
{
    m_label = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setComment( QString comment )
{
    m_comment = comment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setExpanded( bool expand )
{
    m_showExpanded = expand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimParameterGroup::isExpanded() const
{
    return m_showExpanded;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterGroup::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterGroup::comment() const
{
    return m_comment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterGroup::label() const
{
    return labelOrName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimParameterGroup::isListParameter( QString paramName ) const
{
    for ( auto& list : m_lists )
    {
        if ( list->containsParameter( paramName ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGenericParameter*> RimParameterGroup::parameters() const
{
    std::vector<RimGenericParameter*> retParams;

    for ( const auto& p : m_parameters.childObjects() )
    {
        if ( isListParameter( p->name() ) ) continue;
        retParams.push_back( p );
    }

    for ( const auto& list : m_lists )
    {
        retParams.push_back( list->getAsListParameter( m_parameters.childObjects() ) );
    }

    return retParams;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setParameterValue( QString name, int value )
{
    setParameterValue( name, QString::number( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setParameterValue( QString name, double value )
{
    setParameterValue( name, QString::number( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroup::setParameterValue( QString name, QString value )
{
    RimGenericParameter* p = parameter( name );
    if ( p != nullptr ) p->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericParameter* RimParameterGroup::parameter( QString name ) const
{
    for ( auto& p : m_parameters.childObjects() )
    {
        if ( p->name() == name )
        {
            return p;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RimParameterGroup::parameterValue( QString name ) const
{
    RimGenericParameter* p = parameter( name );
    if ( p )
    {
        return p->variantValue();
    }
    return QVariant();
}
