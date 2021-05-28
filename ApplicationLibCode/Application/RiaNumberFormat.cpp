/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaNumberFormat.h"

#include "cafAppEnum.h"

template <>
void caf::AppEnum<RiaNumberFormat::NumberFormatType>::setUp()
{
    addItem( RiaNumberFormat::NumberFormatType::AUTO, "AUTO", "Automatic" );
    addItem( RiaNumberFormat::NumberFormatType::FIXED, "FIXED", "Fixed, decimal" );
    addItem( RiaNumberFormat::NumberFormatType::SCIENTIFIC, "SCIENTIFIC", "Scientific notation" );
    setDefault( RiaNumberFormat::NumberFormatType::FIXED );
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNumberFormat::valueToText( double value, RiaNumberFormat::NumberFormatType numberFormat, int precision )
{
    QString valueString;

    switch ( numberFormat )
    {
        case RiaNumberFormat::NumberFormatType::FIXED:
            valueString = QString::number( value, 'f', precision );
            break;
        case RiaNumberFormat::NumberFormatType::SCIENTIFIC:
            valueString = QString::number( value, 'e', precision );
            break;
        default:
            valueString = QString::number( value );
            break;
    }

    return valueString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaNumberFormat::sprintfFormat( RiaNumberFormat::NumberFormatType numberFormat, int precision )
{
    switch ( numberFormat )
    {
        case RiaNumberFormat::NumberFormatType::FIXED:
            return QString( "%.%1f" ).arg( precision );
        case RiaNumberFormat::NumberFormatType::SCIENTIFIC:
            return QString( "%.%1e" ).arg( precision );
        default:
            return QString( "%.%1g" ).arg( precision );
    }
}
