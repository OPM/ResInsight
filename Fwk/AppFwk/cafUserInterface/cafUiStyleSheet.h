//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include <QString>
#include <map>

class QWidget;

namespace caf
{
//==================================================================================================
// Class used to apply stylesheets to Qt widgets without "bleeding" into other widgets
//==================================================================================================
class UiStyleSheet
{
public:
    class State
    {
    public:
        enum Type
        {
            // Qt Pseudo-states applied automatically by Qt: https://doc.qt.io/qt-5/stylesheet-reference.html
            PseudoState,
            // State set by property syntax
            PropertyState
        };
        State( const QString& stateTag, Type type );
        Type    type() const;
        void    set( const QString& key, const QString& value );
        QString get( const QString& key ) const;

    private:
        friend class UiStyleSheet;
        QString fullText( const QString& className, const QString& objectName, bool applyToSubClasses ) const;

    private:
        Type                       m_type;
        QString                    m_stateTag;
        std::map<QString, QString> m_content;
    };

public:
    UiStyleSheet();

    void    set( const QString& key, const QString& value );
    QString get( const QString& key ) const;

    State& property( QString stateTag );
    State& pseudoState( QString stateTag );

    void        applyToWidget( QWidget* widget, bool applyToSubClasses = true ) const;
    void        applyToWidgetAndChildren( QWidget* widget );
    static void clearWidgetStates( QWidget* widget );
    void        setWidgetState( QWidget* widget, QString stateTag, bool on = true ) const;
    void        stashWidgetStates();
    void        restoreWidgetStates();

private:
    QString     fullText( const QString& className, const QString& objectName, bool applyToSubClasses ) const;
    static void refreshWidget( QWidget* widget );

private:
    std::map<QString, State> m_states;
    std::map<QString, State> m_stashedStates;
};
} // namespace caf
