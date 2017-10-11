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

#include "ExpressionParser.h"

#include "ExpressionParserImpl.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ExpressionParser::ExpressionParser()
{
    m_expressionParserImpl = std::unique_ptr<ExpressionParserImpl>(new ExpressionParserImpl);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ExpressionParser::~ExpressionParser()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExpressionParser::setExpression(const QString& expression)
{
    m_expressionParserImpl->setExpression(expression);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> ExpressionParser::detectReferencedVariables(const QString& expression)
{
    return ExpressionParserImpl::detectReferencedVariables(expression);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExpressionParser::assignVector(const QString& variableName, std::vector<double>& vector)
{
    m_expressionParserImpl->assignVector(variableName, vector);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ExpressionParser::evaluate()
{
    return m_expressionParserImpl->evaluate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString ExpressionParser::errorText()
{
    return m_expressionParserImpl->errorText();
}
