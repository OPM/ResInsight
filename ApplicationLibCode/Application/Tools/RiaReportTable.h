/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifTextDataTableFormatter.h"

#include <QDateTime>
#include <QString>

#include <functional>
#include <variant>
#include <vector>

// One typed cell value; formatting is done by the report window, not the producer
using RiaReportCellValue = std::variant<std::monostate, double, int, QString, QDateTime>;

//==================================================================================================
///
//==================================================================================================
struct RiaReportColumn
{
    QString                          title{};
    QString                          unit{}; // optional, shown as sub-title
    RifTextDataTableDoubleFormatting doubleFormat{}; // default precision hint, user can override
    RifTextDataTableAlignment        alignment = RIGHT;
};

//==================================================================================================
///
//==================================================================================================
struct RiaReportTable
{
    QString                                      title{};
    QString                                      description{}; // free text shown above the table
    std::vector<RiaReportColumn>                 columns{};
    std::vector<std::vector<RiaReportCellValue>> rows{};
};

//==================================================================================================
/// Maps to a tab in the report window
//==================================================================================================
struct RiaReportSection
{
    QString                     title{};
    std::vector<RiaReportTable> tables{};

    // Optional, used for expensive data: the tables are created by the factory the first time they are needed.
    // The factory must handle that the producer object is deleted before it is called.
    std::function<std::vector<RiaReportTable>()> tableFactory{};

    bool hasPendingTables() const { return tableFactory != nullptr; }

    void loadTables()
    {
        if ( !tableFactory ) return;
        tables       = tableFactory();
        tableFactory = nullptr;
    }
};
