/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RiuWidgetStyleSheet.h"

#include "cafAssert.h"

#include <QDebug>
#include <QStringList>
#include <QStyle>
#include <QWidget>

namespace caf
{
template <>
void RiuWidgetStyleSheet::StateTagEnum::setUp()
{
    addItem( RiuWidgetStyleSheet::DEFAULT, "", "" );
    addItem( RiuWidgetStyleSheet::SELECTED, "selected", "selected" );
    addItem( RiuWidgetStyleSheet::DRAG_TARGET_BEFORE, "dragTargetBefore", "drop before" );
    addItem( RiuWidgetStyleSheet::DRAG_TARGET_AFTER, "dragTargetAfter", "drop after" );
    addItem( RiuWidgetStyleSheet::DRAG_TARGET_INTO, "dragTargetInto", "drop into" );
    addItem( RiuWidgetStyleSheet::HOVER, "hover", "hover" );
    setDefault( RiuWidgetStyleSheet::DEFAULT );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWidgetStyleSheet::State::State( const QString& stateString )
    : m_stateString( stateString )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetStyleSheet::State::set( const QString& key, const QString& value )
{
    m_content[key] = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::State::get( const QString& key ) const
{
    auto it = m_content.find( key );
    return it != m_content.end() ? it->second : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::State::fullText( const QString& className, const QString& objectName ) const
{
    QString styleContent;
    for ( auto keyValuePair : m_content )
    {
        styleContent += QString( "  %1: %2;\n" ).arg( keyValuePair.first ).arg( keyValuePair.second );
    }

    QString objectNameTag = !objectName.isEmpty() ? QString( "#%1" ).arg( objectName ) : "";

    QString fullStyleSheet =
        QString( "%1%2%3\n{\n%4}" ).arg( className ).arg( objectNameTag ).arg( m_stateString ).arg( styleContent );
    return fullStyleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWidgetStyleSheet::RiuWidgetStyleSheet()
{
    // Add the default state
    m_states.insert( std::make_pair( DEFAULT, State( "" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetStyleSheet::set( const QString& key, const QString& value )
{
    state( DEFAULT ).set( key, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::get( const QString& key ) const
{
    auto it = m_states.find( DEFAULT );
    return it != m_states.end() ? it->second.get( key ) : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWidgetStyleSheet::State& RiuWidgetStyleSheet::state( StateTag stateTag )
{
    auto itBoolPair = m_states.insert( std::make_pair( stateTag, State( buildStateString( stateTag ) ) ) );
    auto it         = itBoolPair.first;

    return it->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::propertyName( StateTag state )
{
    if ( state < PSEUDO_STATE_LIMIT )
    {
        return StateTagEnum::text( state );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetStyleSheet::applyToWidget( QWidget* widget ) const
{
    QString completeStyleSheet = fullText( QString( widget->metaObject()->className() ), widget->objectName() );
    // qDebug().noquote() << completeStyleSheet;
    widget->setStyleSheet( completeStyleSheet );
    refreshWidget( widget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetStyleSheet::refreshWidget( QWidget* widget ) const
{
    widget->style()->unpolish( widget );
    widget->style()->polish( widget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetStyleSheet::setWidgetState( QWidget* widget, StateTag widgetState ) const
{
    // Set all existing dynamic properties to false
    for ( QByteArray existingProperty : widget->dynamicPropertyNames() )
    {
        widget->setProperty( existingProperty, false );
    }

    // Set current property state to true
    QString propertyName = RiuWidgetStyleSheet::propertyName( widgetState );
    if ( !propertyName.isEmpty() )
    {
        widget->setProperty( propertyName.toLatin1(), true );
    }

    // Trigger style update
    this->refreshWidget( widget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::fullText( const QString& className, const QString& objectName ) const
{
    QStringList textForAllStates;
    for ( auto it = m_states.begin(); it != m_states.end(); ++it )
    {
        textForAllStates.push_back( it->second.fullText( className, objectName ) );
    }
    return textForAllStates.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWidgetStyleSheet::buildStateString( StateTag state )
{
    QString stateString;
    if ( state > PSEUDO_STATE_LIMIT )
    {
        stateString += ":" + StateTagEnum::uiText( state );
    }
    else
    {
        stateString += QString( "[%1=true]" ).arg( propertyName( state ) );
    }
    return stateString;
}
