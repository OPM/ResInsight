/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotCurveDefines.h"

#include "RifEclipseSummaryAddressDefines.h"

#include <QDateTime>
#include <QLocale>
#include <QString>

//==================================================================================================
///
//==================================================================================================
class RifAsciiDataParseOptions
{
public:
    RifAsciiDataParseOptions()
        : useCustomDateTimeFormat( false )
        , assumeNumericDataColumns( false )
        , curveSymbolSkipDistance( 0.0f )
        , defaultCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
        , cellSeparator( ";" )
    {
    }

    QString plotTitle;
    QString curvePrefix;
    QString decimalSeparator;
    QLocale locale;

    bool    useCustomDateTimeFormat;
    QString dateFormat;
    QString timeFormat;
    QString fallbackDateTimeFormat;
    QString dateTimeFormat;
    QString cellSeparator;
    QString timeSeriesColumnName;

    QDateTime startDateTime;
    bool      assumeNumericDataColumns;

    RifEclipseSummaryAddressDefines::SummaryCategory defaultCategory;

    RiuQwtPlotCurveDefines::LineStyleEnum curveLineStyle;
    RiuPlotCurveSymbol::PointSymbolEnum   curveSymbol;
    float                                 curveSymbolSkipDistance;
};
