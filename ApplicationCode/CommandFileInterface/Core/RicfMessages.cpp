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
#include "RicfMessages.h"
#include <QTextStream>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfMessages::addWarning(const QString& message)
{
    m_messages.push_back(std::make_pair(WARNING, "Line " + QString::number(m_currentLineNumber) +": " + message));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfMessages::addError(const QString& message)
{
    m_messages.push_back(std::make_pair(ERROR, "Line " + QString::number(m_currentLineNumber) +": " + message));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfMessages::skipWhiteSpaceWithLineNumberCount(QTextStream& inputStream)
{
    while ( !inputStream.atEnd() )
    {
        QChar ch = readCharWithLineNumberCount(inputStream);
        if ( !ch.isSpace() )
        {
            inputStream.seek(inputStream.pos()-1);
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QChar RicfMessages::readCharWithLineNumberCount(QTextStream& inputStream)
{
    QChar ch;
    inputStream >> ch;
    if ( ch == QChar('\n') )
    {
        m_currentLineNumber++;
    }
    return ch;
}
