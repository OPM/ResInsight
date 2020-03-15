/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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
#include "cafPdmScriptIOMessages.h"
#include <QTextStream>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::addWarning( const QString& message )
{
    m_messages.push_back(
        std::make_pair( MESSAGE_WARNING, "Line " + QString::number( m_currentLineNumber ) + ": " + message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::addError( const QString& message )
{
    m_messages.push_back(
        std::make_pair( MESSAGE_ERROR, "Line " + QString::number( m_currentLineNumber ) + ": " + message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::skipWhiteSpaceWithLineNumberCount( QTextStream& inputStream )
{
    while ( !inputStream.atEnd() )
    {
        QChar ch = readCharWithLineNumberCount( inputStream );
        if ( !ch.isSpace() )
        {
            inputStream.seek( inputStream.pos() - 1 );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QChar PdmScriptIOMessages::readCharWithLineNumberCount( QTextStream& inputStream )
{
    QChar ch;
    inputStream >> ch;
    if ( ch == QChar( '\n' ) )
    {
        m_currentLineNumber++;
    }
    return ch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QChar PdmScriptIOMessages::peekNextChar( QTextStream& inputStream )
{
    QChar ch;
    if ( !inputStream.atEnd() )
    {
        inputStream >> ch;
        inputStream.seek( inputStream.pos() - 1 );
    }
    return ch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::skipLineWithLineNumberCount( QTextStream& inputStream )
{
    inputStream.readLine();
    m_currentLineNumber++;
}
