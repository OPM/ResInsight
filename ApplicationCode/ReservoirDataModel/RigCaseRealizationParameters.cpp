/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RigCaseRealizationParameters.h"
#include <limits>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseRealizationParameters::Value::Value() : m_valueType(TYPE_NONE), m_numericValue(std::numeric_limits<double>::infinity())
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseRealizationParameters::Value::Value(double value) : Value()
{
    setValue(value);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseRealizationParameters::Value::Value(const QString& value) : Value()
{
    setValue(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseRealizationParameters::Value::setValue(double value)
{
    m_valueType = TYPE_NUMERIC;
    m_numericValue = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseRealizationParameters::Value::setValue(const QString& value)
{
    m_valueType = TYPE_TEXT;
    m_textValue = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCaseRealizationParameters::Value::numericValue() const
{
    return m_numericValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RigCaseRealizationParameters::Value::textValue() const
{
    return m_textValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseRealizationParameters::addParameter(const QString& name, double value)
{
    m_parameters[name].setValue(value);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseRealizationParameters::addParameter(const QString& name, const QString& value)
{
    m_parameters[name].setValue(value);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseRealizationParameters::Value RigCaseRealizationParameters::parameterValue(const QString& name)
{
    if (m_parameters.count(name) == 0) return Value();
    return m_parameters[name];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, RigCaseRealizationParameters::Value> RigCaseRealizationParameters::parameters() const
{
    return m_parameters;
}
