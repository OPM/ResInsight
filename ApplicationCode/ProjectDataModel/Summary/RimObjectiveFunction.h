/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include <QString>
#include <QVariant>

#include <vector>

//==================================================================================================
///
//==================================================================================================
class ObjectiveFunction
{
public:
    enum class Type
    {
        NONE,
        WCT,
        GOR,
        BHP
    };

    QString               uiName() const { return name; };
    QString               name;
    Type                  type;
    std::vector<QVariant> values;
    double                minValue;
    double                maxValue;

    ObjectiveFunction()
        : type( Type::WCT )
        , minValue( std::numeric_limits<double>::infinity() )
        , maxValue( -std::numeric_limits<double>::infinity() )
    {
    }

    bool   isValid() const { return !name.isEmpty() && type != Type::NONE; }
    double normalizedStdDeviation() const;

    bool operator<( const ObjectiveFunction& other ) const;

private:
    double stdDeviation() const;
};
