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

#include "cafAppEnum.h"

#include <QString>

#include <map>

class RiuWidgetStyleSheetManager;
class QWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuWidgetStyleSheet
{
public:
    enum StateTag
    {
        DEFAULT = 0x0000,
        // Dynamic Properties:
        SELECTED           = 0x0001,
        DRAG_TARGET_BEFORE = 0x0002,
        DRAG_TARGET_AFTER  = 0x0004,
        DRAG_TARGET_INTO   = 0x0008,
        // Pseudo States:
        PSEUDO_STATE_LIMIT = 0x1000,
        HOVER              = 0x1000
    };
    typedef caf::AppEnum<StateTag> StateTagEnum;

    class State
    {
    public:
        State( const QString& stateString );
        void    set( const QString& key, const QString& value );
        QString get( const QString& key ) const;
        QString fullText( const QString& className, const QString& objectName ) const;

    private:
        QString                    m_stateString;
        std::map<QString, QString> m_content;
    };

public:
    RiuWidgetStyleSheet();

    void    set( const QString& key, const QString& value );
    QString get( const QString& key ) const;

    State& state( StateTag stateTag );

    static QString propertyName( StateTag stateTag );

    void applyToWidget( QWidget* widget ) const;
    void refreshWidget( QWidget* widget ) const;
    void setWidgetState( QWidget* widget, StateTag widgetState ) const;

private:
    friend class RiuWidgetStyleSheetManager;
    QString fullText( const QString& className, const QString& objectName ) const;

    static QString buildStateString( StateTag stateTag );

private:
    std::map<StateTag, State> m_states;
};
