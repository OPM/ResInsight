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

#pragma once

#include <QObject>
#include <QPointer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaFeatureCommandContext
{
    friend class RiaFeatureCommandContextHelper;

public:
    static RiaFeatureCommandContext* instance();

    RiaFeatureCommandContext( const RiaFeatureCommandContext& ) = delete;

    QObject* object() const;
    void     setObject( QObject* object );

    QString titleString() const;
    QString contentString() const;

    static std::string titleStringIdentifier();
    static std::string contentStringIdentifier();

private:
    RiaFeatureCommandContext();
    ~RiaFeatureCommandContext();

private:
    QPointer<QObject> m_pointerToQObject;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaFeatureCommandContextHelper
{
public:
    explicit RiaFeatureCommandContextHelper( QObject* externalObject );
    ~RiaFeatureCommandContextHelper();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaFeatureCommandContextTextHelper
{
public:
    RiaFeatureCommandContextTextHelper( const QString& title, const QString& text );
    ~RiaFeatureCommandContextTextHelper();

private:
    QObject* m_object;
};
