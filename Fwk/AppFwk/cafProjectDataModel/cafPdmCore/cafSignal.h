//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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

#include "cafAssert.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>

namespace caf
{
class SignalObserver
{
public:
    virtual ~SignalObserver() = default;
};

class SignalEmitter
{
public:
    virtual ~SignalEmitter() = default;
};

//==================================================================================================
/// General signal class.
/// Connect any member function with the signature void(const Signal*, const SignalData* data)
/// Connect with .connect(this, &Class::nameOfMethod)
/// The method should accept that data may be nullptr
//==================================================================================================
template <typename... Args>
class Signal
{
public:
    using MemberCallback = std::function<void( const SignalEmitter*, Args... args )>;

public:
    Signal( const SignalEmitter* emitter )
        : m_emitter( emitter )
    {
    }
    virtual ~Signal() = default;

    template <typename ClassType>
    void connect( ClassType* observer, void ( ClassType::*method )( const SignalEmitter*, Args... args ) )
    {
        static_assert( std::is_convertible<ClassType*, SignalObserver*>::value,
                       "Only classes that inherit SignalObserver can connect as an observer of a Signal." );
        MemberCallback lambda = [=]( const SignalEmitter* emitter, Args... args ) {
            // Call method
            ( observer->*method )( emitter, args... );
        };
        m_observerCallbacks[observer] = lambda;
    }

    void disconnect( SignalObserver* observer ) { m_observerCallbacks.erase( observer ); }
    void send( Args... args )
    {
        for ( auto observerCallbackPair : m_observerCallbacks )
        {
            observerCallbackPair.second( m_emitter, args... );
        }
    }

private:
    Signal( const Signal& rhs ) = default;
    Signal& operator=( const Signal& rhs ) = default;

    std::map<SignalObserver*, MemberCallback> m_observerCallbacks;
    const SignalEmitter*                      m_emitter;
};
} // namespace caf
