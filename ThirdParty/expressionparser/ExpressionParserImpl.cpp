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

#include <QRegularExpression>

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
            if (indexAndName.second.compare("delta", Qt::CaseInsensitive) == 0) continue;

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
QString ExpressionParserImpl::expandStatements(const QString& expressionText)
{
    QString expandedExpressionText = expressionText;

    if (expandedExpressionText.contains("if("))
    {
        expandedExpressionText = ExpressionParserImpl::expandIfStatements(expandedExpressionText);
    }

    if (expandedExpressionText.contains("delta"))
    {
        expandedExpressionText = ExpressionParserImpl::expandDeltaFunction(expandedExpressionText);
    }

    return expandedExpressionText;
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

//--------------------------------------------------------------------------------------------------
// 
// The parser do not support single-line if statements on a vector.
// Expand a single line if-statement to a for loop over all items in the referenced 
// vectors. The script will add '[i]' as postfix to all variables, assuming all variables
// to be vectors. This statement will be put in a for loop over all elements counting
// up to size of the vector with minimum items.
//
// Single line statement :
//   c := if((a > 13), a, b)
//
// Intended parser script text
//
//    for (var i : = 0; i < min(c[], a[], b[]); i += 1)
//    {
//        c[i] : = if ((a[i] > 13), a[i], b[i]);
//    }
//
//--------------------------------------------------------------------------------------------------
QString ExpressionParserImpl::expandIfStatements(const QString& expressionText)
{
    if (!expressionText.contains("if(")) return expressionText;

    QString expandedText;
    {
        QString textWithVectorBrackets = expressionText;

        QString listOfVars;
        auto allVectorVariables = detectReferencedVariables(expressionText);
        for (const QString& var : allVectorVariables)
        {
            listOfVars += QString("%1[],").arg(var);
            
            QString regexpText = QString("\\b%1\\b").arg(var);
            QRegularExpression regexp(regexpText);

            QString varWithBrackets = var + "[i]";
            textWithVectorBrackets = textWithVectorBrackets.replace(regexp, varWithBrackets);
        }

        listOfVars = listOfVars.left(listOfVars.size() - 1);

        expandedText = QString("for (var i := 0; i < min(%1); i += 1)\n").arg(listOfVars);
        expandedText += "{\n";
        expandedText += QString("    %1;\n").arg(textWithVectorBrackets);
        expandedText += "}\n";
    }

    return expandedText;
}

//--------------------------------------------------------------------------------------------------
// 
//  Use a macro to implement function 'delta(a, b)'
//  Input vector a will be used to compute delta vector b 
//
// Can be used for summary data to create derived data
// delta(a, b) 
// a : existing summary curve
// b : delta values to be computed
//    
// b[0] : = 0;
// for (var i : = 1; i < a[]; i += 1)
// {
//     b[i] : = a[i] - a[i-1];
// }
//
//--------------------------------------------------------------------------------------------------
QString ExpressionParserImpl::expandDeltaFunction(const QString& expressionText)
{
    QString expandedText;

    QStringList lines = expressionText.split("\n");

    for (const auto& line : lines)
    {
        QString lineNoWhitespace = line;
        lineNoWhitespace = lineNoWhitespace.replace(" ", "");
        QStringList words = lineNoWhitespace.split(":=");
        if (words.size() == 2)
        {
            auto deltaVariable = words[0].trimmed();

            auto sourceVariable = words[1].trimmed();
            sourceVariable.replace("delta(", "");
            sourceVariable.replace(")", "");

            QString deltaLoopText = QString("%1[0] := 0\n").arg(deltaVariable);
            deltaLoopText += QString("for (var i := 1; i < %1[]; i += 1)\n").arg(sourceVariable);
            deltaLoopText += "{\n";
            deltaLoopText += QString("  %1[i] := %2[i] - %2[i-1];\n").arg(deltaVariable).arg(sourceVariable);
            deltaLoopText += "}\n";

            expandedText += deltaLoopText;
        }
        else
        {
            expandedText += line;
            expandedText += "\n";
        }
    }

    return expandedText;
}
