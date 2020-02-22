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

#include "ExpressionParserImpl.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ExpressionParserImpl::ExpressionParserImpl()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> ExpressionParserImpl::detectReferencedVariables(const QString& expression)
{
    std::vector<QString> referencedVariables;
    {
        std::vector<std::string> variable_list;
        exprtk::collect_variables(expression.toStdString(), variable_list);

        std::vector<std::pair<int, QString>> indexAndNamePairs;

        for (const auto& s : variable_list)
        {
            QString variableNameLowerCase = QString::fromStdString(s);

            // ExprTk reports always in lower case

            int index = expression.indexOf(variableNameLowerCase, 0, Qt::CaseInsensitive);
            if (index > -1)
            {
                indexAndNamePairs.push_back(std::make_pair(index, expression.mid(index, variableNameLowerCase.size())));
            }
        }

        // Sort the variable names in the order they first appear in the expression
        std::sort(indexAndNamePairs.begin(), indexAndNamePairs.end());

        for (const auto& indexAndName : indexAndNamePairs)
        {
            referencedVariables.push_back(indexAndName.second);
        }
    }

    return referencedVariables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExpressionParserImpl::assignVector(const QString& variableName, std::vector<double>& vector)
{
    m_symbol_table.add_vector(variableName.toStdString(), vector);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ExpressionParserImpl::evaluate(const QString& expressionText, QString* errorText)
{
    expression_t expression;

    expression.register_symbol_table(m_symbol_table);

    parser_t parser;
    if (!parser.compile(expressionText.toStdString(), expression))
    {
        if (errorText)
        {
            *errorText = parserErrorText(parser);
        }

        return false;
    }

    // Trigger evaluation
    expression.value();

    if (errorText)
    {
        *errorText = parserErrorText(parser);
    }

    return (parser.error_count() == 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString ExpressionParserImpl::parserErrorText(parser_t& parser)
{
    QString txt;

    for (size_t i = 0; i < parser.error_count(); i++)
    {
        auto error = parser.get_error(i);

        QString errorMsg = QString("Position: %1   Type: [%2]   Msg: %3\n")
            .arg(static_cast<int>(error.token.position))
            .arg(exprtk::parser_error::to_str(error.mode).c_str())
            .arg(error.diagnostic.c_str());
        
        txt += errorMsg;
    }

    return txt;
}
