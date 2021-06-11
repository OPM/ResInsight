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

#include "RiaFeatureCommandContext.h"

#include <QVariant>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext::RiaFeatureCommandContext()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext::~RiaFeatureCommandContext()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFeatureCommandContext::setObject( QObject* object )
{
    m_pointerToQObject = object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QObject* RiaFeatureCommandContext::object() const
{
    return m_pointerToQObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFeatureCommandContext::titleString() const
{
    if ( m_pointerToQObject )
    {
        QVariant variant = m_pointerToQObject->property( titleStringIdentifier().data() );

        return variant.toString();
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFeatureCommandContext::contentString() const
{
    if ( m_pointerToQObject )
    {
        QVariant variant = m_pointerToQObject->property( contentStringIdentifier().data() );

        return variant.toString();
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaFeatureCommandContext::titleStringIdentifier()
{
    return "titleStringIdentifier";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaFeatureCommandContext::contentStringIdentifier()
{
    return "contentStringIdentifier";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext* RiaFeatureCommandContext::instance()
{
    static RiaFeatureCommandContext* commandFileExecutorInstance = new RiaFeatureCommandContext();
    return commandFileExecutorInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextHelper::RiaFeatureCommandContextHelper( QObject* externalObject )
{
    RiaFeatureCommandContext::instance()->setObject( externalObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextHelper::~RiaFeatureCommandContextHelper()
{
    RiaFeatureCommandContext::instance()->setObject( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextTextHelper::RiaFeatureCommandContextTextHelper( const QString& title, const QString& text )
{
    m_object = new QObject;

    m_object->setProperty( RiaFeatureCommandContext::titleStringIdentifier().data(), title );
    m_object->setProperty( RiaFeatureCommandContext::contentStringIdentifier().data(), text );

    RiaFeatureCommandContext::instance()->setObject( m_object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextTextHelper::~RiaFeatureCommandContextTextHelper()
{
    if ( m_object )
    {
        m_object->deleteLater();
        m_object = nullptr;
    }
}
