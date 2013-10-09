//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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


#include "cvfBase.h"
#include "cvfLogEvent.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::LogEvent
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogEvent::LogEvent()
:   m_level(Logger::LL_ERROR)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogEvent::LogEvent(const String& source, const String& message, Logger::Level level, const CodeLocation& codeLocation)
:   m_source(source),
    m_message(message),
    m_level(level),
    m_codeLocation(codeLocation)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogEvent::LogEvent(const LogEvent& other)
:   m_source(other.m_source),
    m_message(other.m_message),
    m_level(other.m_level),
    m_codeLocation(other.m_codeLocation)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const LogEvent& LogEvent::operator=(LogEvent rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const String& LogEvent::source() const
{
    return m_source;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Logger::Level LogEvent::level() const
{
    return m_level;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const String& LogEvent::message() const
{
    return m_message;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const CodeLocation& LogEvent::location() const
{
    return m_codeLocation;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogEvent::swap(LogEvent& other)
{
    m_source.swap(other.m_source);
    m_message.swap(other.m_message);
    std::swap(m_level, other.m_level);
    m_codeLocation.swap(other.m_codeLocation);
}

} // namespace cvf

