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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include <QString>

#include <map>
#include <set>
#include <memory>

//==================================================================================================
//
//
//==================================================================================================
class RigCaseRealizationParameters
{
public:
    // Internal class
    class Value
    {
        enum ValueType { TYPE_NONE, TYPE_NUMERIC, TYPE_TEXT };

    public:
        Value();
        Value(double value);
        Value(const QString& value);

        void setValue(double value);
        void setValue(const QString& value);

        bool isValid() const { return m_valueType != TYPE_NONE; }
        bool isNumeric() const { return m_valueType == TYPE_NUMERIC; }
        bool isText() const { return m_valueType == TYPE_TEXT; }

        double          numericValue() const;
        const QString&  textValue() const;

    private:
        ValueType   m_valueType;
        double      m_numericValue;
        QString     m_textValue;
    };

    RigCaseRealizationParameters() : m_parametersHash(0) { }

    void                        addParameter(const QString& name, double value);
    void                        addParameter(const QString& name, const QString& value);
    Value                       parameterValue(const QString& name);

    std::map<QString, Value>    parameters() const;
    std::set<QString>           parameterNames() const;

    size_t                      parameterHash(const QString& name) const;
    size_t                      parametersHash();

    void                        clearParametersHash();
    void                        calculateParametersHash(const std::set<QString>& paramNames = std::set<QString>());

private:
    std::map<QString, Value>    m_parameters;
    size_t                      m_parametersHash;
};
