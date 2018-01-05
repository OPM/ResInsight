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
#pragma once

#include <vector>
#include <QString>

class QTextStream;

class RicfMessages
{
public:
    RicfMessages() : m_currentLineNumber(1) {}

    enum MessageType
    {
        MESSAGE_WARNING,
        MESSAGE_ERROR
    };

    void    addWarning(const QString& message);
    void    addError(const QString& message);

    void    skipWhiteSpaceWithLineNumberCount(QTextStream& inputStream);
    QChar   readCharWithLineNumberCount(QTextStream& inputStream);
    QChar   peekNextChar(QTextStream& inputStream);

    QString currentCommand;
    QString currentArgument;
    std::vector<std::pair<MessageType, QString> > m_messages;

private:
    int m_currentLineNumber;
};


