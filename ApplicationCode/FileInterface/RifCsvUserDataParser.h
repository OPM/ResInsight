/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseUserDataParserTools.h"

#include "../Commands/SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include <QString>
#include <QPointer>
#include <QStringList>
#include <QDateTime>

#include <vector>

class ColumnInfo;

//==================================================================================================
/// 
//==================================================================================================
class RifCsvUserDataParser
{
public:
    RifCsvUserDataParser(QString* errorText = nullptr);


    bool                parse(const QString& data, const AsciiDataParseOptions& parseOptions);
    const TableData&    tableData() const;

    const ColumnInfo*   columnInfo(size_t columnIndex) const;

private:
    bool        parseData(const QString& data, const AsciiDataParseOptions& parseOptions);
    QStringList splitLineAndTrim(const QString& list, const QString& separator);
    QDateTime   tryParseDateTime(const std::string& colData, const QString& format);

private:
    TableData               m_tableData;
    QString*                m_errorText;
};
