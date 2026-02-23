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

#include <QString>

#include <expected>
#include <memory>
#include <vector>

class ExpressionParserImpl;

//==================================================================================================
///
//==================================================================================================
class ExpressionParser
{
public:
    ExpressionParser();
    ~ExpressionParser();

    static std::vector<QString> detectReferencedVariables( const QString& expression );

    void assignVector( const QString& variableName, std::vector<double>& vector );

    // Returns an empty expected on success, or an unexpected containing the error message on failure.
    std::expected<void, QString> evaluate( const QString& expressionText );
    std::expected<void, QString> expandIfStatementsAndEvaluate( const QString& expressionText );

private:
    std::unique_ptr<ExpressionParserImpl> m_expressionParserImpl;
};
