/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "exprtk/exprtk.hpp"

#include <QString>

//==================================================================================================
/// 
//==================================================================================================
class ExpressionParserImpl
{
    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double>     expression_t;
    typedef exprtk::parser<double>             parser_t;

public:
    ExpressionParserImpl();

    void setExpression(const QString& expression);

    static std::vector<QString> detectReferencedVariables(const QString& expression);

    void assignVector(const QString& variableName, std::vector<double>& vector);

    bool evaluate();

    QString errorText() const;

private:
    QString m_expression;

    symbol_table_t m_symbol_table;
    expression_t   expression;
    mutable parser_t       parser;
};
