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
std::expected<void, QString> ExpressionParserImpl::evaluate( const QString& expressionText )
{
    expression_t expression;
    expression.register_symbol_table( m_symbol_table );

    parser_t parser;
    if ( !parser.compile( expressionText.toStdString(), expression ) )
        return std::unexpected( parserErrorText( parser ) );

    // Trigger evaluation
    expression.value();

    if ( parser.error_count() > 0 )
        return std::unexpected( parserErrorText( parser ) );

    return {};
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
// When aggregating functions (min, max, sum, avg) are applied to a single vector variable,
// they compute a scalar result over the whole vector. These must be pre-computed before the
// element-wise for loop, otherwise the variable gets replaced with its indexed version (e.g.
// min(b) -> min(b[i])) which changes the semantics: min(b[i]) returns b[i], not min of all b.
//
// Example with pre-computation:
//   c := if(a > min(b), a, b)
//
// Expanded to:
//   var ri_agg_0 := min(b);
//   for (var i := 0; i < min(c[], a[], b[]); i += 1)
//   {
//       c[i] := if(a[i] > ri_agg_0, a[i], b[i]);
//   }
//
// When an aggregating function wraps an entire if expression, the result is a scalar assigned
// to all elements of the LHS vector. This requires a two-pass expansion:
//
//   c := min(if(a > 13, a, b))
//
// Expanded to:
//   var ri_agg_0 := 1.7976931348623158e+308;
//   for (var i := 0; i < min(a[], b[]); i += 1)
//   {
//       var ri_elem := if(a[i] > 13, a[i], b[i]);
//       ri_agg_0 := min(ri_agg_0, ri_elem);
//   }
//   for (var i := 0; i < min(c[]); i += 1)
//   {
//       c[i] := ri_agg_0;
//   }
//
//--------------------------------------------------------------------------------------------------
QString ExpressionParserImpl::expandIfStatements(const QString& expressionText)
{
    // Check for pattern: c := agg_func(if(condition, true_expr, false_expr))
    // Computes a scalar aggregate of the element-wise if-result and assigns it as a constant
    // to all elements of c. The two closing parens )) close the if() and agg_func() respectively.
    QRegularExpression outerAggIfPattern( R"(\b(\w+)\s*:=\s*(min|max|sum|avg)\(if\((.+)\)\)\s*$)",
                                          QRegularExpression::CaseInsensitiveOption |
                                              QRegularExpression::DotMatchesEverythingOption );

    auto outerMatch = outerAggIfPattern.match( expressionText.trimmed() );
    if ( outerMatch.hasMatch() )
    {
        QString lhsVar  = outerMatch.captured( 1 );
        QString aggFunc = outerMatch.captured( 2 ).toLower();
        QString ifArgs  = outerMatch.captured( 3 );

        // Detect referenced variables, excluding the outer aggregating function name itself.
        // exprtk::collect_variables may treat the function name (e.g. "avg") as a variable
        // when it appears in an expression context it doesn't fully recognise (avg(if(...))).
        static const QStringList knownFunctions = { "min", "max", "sum", "avg" };
        auto                     allVars        = detectReferencedVariables( expressionText );
        allVars.erase( std::remove_if( allVars.begin(),
                                       allVars.end(),
                                       [&]( const QString& v )
                                       { return knownFunctions.contains( v, Qt::CaseInsensitive ); } ),
                       allVars.end() );

        QString listOfNonLhsVars;
        for ( const QString& var : allVars )
        {
            if ( var != lhsVar )
                listOfNonLhsVars += QString( "%1[]," ).arg( var );
        }
        listOfNonLhsVars = listOfNonLhsVars.left( listOfNonLhsVars.size() - 1 );

        // Build element-wise version of the if expression
        QString indexedIfExpr = QString( "if(%1)" ).arg( ifArgs );
        for ( const QString& var : allVars )
        {
            if ( var == lhsVar ) continue;
            QString            regexpText = QString( "\\b%1\\b" ).arg( var );
            QRegularExpression regexp( regexpText );
            indexedIfExpr = indexedIfExpr.replace( regexp, var + "[i]" );
        }

        QString preVarName = "ri_agg_0";
        QString initVal;
        QString extraVarDecl;
        QString updateExpr;

        if ( aggFunc == "min" )
        {
            initVal    = "1.7976931348623158e+308";
            updateExpr = QString( "%1 := min(%1, ri_elem);" ).arg( preVarName );
        }
        else if ( aggFunc == "max" )
        {
            initVal    = "-1.7976931348623158e+308";
            updateExpr = QString( "%1 := max(%1, ri_elem);" ).arg( preVarName );
        }
        else if ( aggFunc == "sum" )
        {
            initVal    = "0";
            updateExpr = QString( "%1 := %1 + ri_elem;" ).arg( preVarName );
        }
        else // avg: Welford online algorithm to avoid a post-loop assignment
        {
            // ri_agg_0 accumulates the running mean; ri_k tracks the count.
            // After the loop ri_agg_0 holds the average with no statement needed outside the loop.
            initVal      = "0";
            extraVarDecl = "var ri_k := 0;\n";
            updateExpr   = "ri_k := ri_k + 1;\n"
                           "    ri_agg_0 := ri_agg_0 + (ri_elem - ri_agg_0) / ri_k;";
        }

        QString expandedText;
        expandedText += QString( "var %1 := %2;\n" ).arg( preVarName, initVal );
        expandedText += extraVarDecl;
        expandedText += QString( "for (var i := 0; i < min(%1); i += 1)\n" ).arg( listOfNonLhsVars );
        expandedText += "{\n";
        expandedText += QString( "    var ri_elem := %1;\n" ).arg( indexedIfExpr );
        expandedText += QString( "    %1\n" ).arg( updateExpr );
        expandedText += "}\n";

        expandedText += QString( "for (var i := 0; i < min(%1[]); i += 1)\n" ).arg( lhsVar );
        expandedText += "{\n";
        expandedText += QString( "    %1[i] := %2;\n" ).arg( lhsVar, preVarName );
        expandedText += "}\n";

        return expandedText;
    }

    auto allVectorVariables = detectReferencedVariables(expressionText);

    QString textWithVectorBrackets = expressionText;
    QString precomputedStatements;

    // Pre-compute aggregating function calls with a single vector variable argument.
    // For example, min(b) should be computed as a scalar before the element-wise loop.
    // Without pre-computation, min(b) would expand to min(b[i]) which just returns b[i].
    static const QStringList aggregatingFunctions = { "min", "max", "sum", "avg" };

    int precomputedIndex = 0;
    for ( const QString& func : aggregatingFunctions )
    {
        for ( const QString& var : allVectorVariables )
        {
            // Match func(var) where var is the sole argument (bounded by word boundaries)
            QString patternStr = QString( "\\b%1\\(\\s*\\b%2\\b\\s*\\)" ).arg( func, var );
            QRegularExpression pattern( patternStr, QRegularExpression::CaseInsensitiveOption );

            if ( pattern.match( textWithVectorBrackets ).hasMatch() )
            {
                // Create a unique scalar variable name for the pre-computed value.
                // Must start with a letter (exprtk requirement). Use ri_agg_N prefix.
                QString preVarName = QString( "ri_agg_%1" ).arg( precomputedIndex++ );
                precomputedStatements += QString( "var %1 := %2(%3);\n" ).arg( preVarName, func, var );
                textWithVectorBrackets.replace( pattern, preVarName );
            }
        }
    }

    QString listOfVars;
    for ( const QString& var : allVectorVariables )
    {
        listOfVars += QString( "%1[]," ).arg( var );

        QString            regexpText = QString( "\\b%1\\b" ).arg( var );
        QRegularExpression regexp( regexpText );

        QString varWithBrackets = var + "[i]";
        textWithVectorBrackets  = textWithVectorBrackets.replace( regexp, varWithBrackets );
    }

    listOfVars = listOfVars.left( listOfVars.size() - 1 );

    QString expandedText = precomputedStatements;
    expandedText += QString( "for (var i := 0; i < min(%1); i += 1)\n" ).arg( listOfVars );
    expandedText += "{\n";
    expandedText += QString( "    %1;\n" ).arg( textWithVectorBrackets );
    expandedText += "}\n";

    return expandedText;
}
