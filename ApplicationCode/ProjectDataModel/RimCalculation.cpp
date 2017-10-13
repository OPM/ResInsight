/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimCalculation.h"

#include "expressionparser/ExpressionParser.h"

#include "RiaLogging.h"
#include "RimCalculationVariable.h"

#include "cafPdmUiTextEditor.h"

#include <numeric>
#include <algorithm>


CAF_PDM_SOURCE_INIT(RimCalculation, "RimCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculation::RimCalculation()
{
    CAF_PDM_InitObject("RimCalculation", ":/octave.png", "Calculation", "");

    CAF_PDM_InitFieldNoDefault(&m_description,      "Description",      "Description", "", "", "");
    m_description.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_expression,       "Expression",       "Expression", "", "", "");
    m_expression.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_variables,        "Variables",        "Variables", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_calculatedValues, "CalculatedValues", "Calculated Values", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculation::setDescription(const QString& description)
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCalculation::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* RimCalculation::variables()
{
    return &m_variables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationVariable* RimCalculation::addVariable()
{
    RimCalculationVariable* v = new RimCalculationVariable;
    m_variables.push_back(v);

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculation::deleteVariable(RimCalculationVariable* calcVariable)
{
    m_variables.removeChildObject(calcVariable);

    delete calcVariable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimCalculation::values() const
{
    return m_calculatedValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCalculation::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCalculation::parseExpression()
{
    // The expression parser handles only variables in lower case
    QString lowerVariant = m_expression().toLower();
    m_expression = lowerVariant;

    QString leftHandSideVariableName = RimCalculation::findLeftHandSide(m_expression);
    if (leftHandSideVariableName.isEmpty()) return false;

    std::vector<QString> variableNames = ExpressionParser::detectReferencedVariables(m_expression);

    // Remove variables not present in expression
    {
        std::vector<RimCalculationVariable*> toBeDeleted;
        for (RimCalculationVariable* v : m_variables)
        {
            if (std::find(variableNames.begin(), variableNames.end(), v->name()) == variableNames.end())
            {
                toBeDeleted.push_back(v);
            }

            if (leftHandSideVariableName == v->name())
            {
                toBeDeleted.push_back(v);
            }
        }

        for (RimCalculationVariable* v : toBeDeleted)
        {
            deleteVariable(v);
        }
    }

    for (auto variableName : variableNames)
    {
        if (leftHandSideVariableName != variableName)
        {
            if (!findByName(variableName))
            {
                auto v = this->addVariable();
                v->setName(variableName);
            }
        }
    }

    this->updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCalculation::calculate()
{
    if (!parseExpression()) return false;

    QString leftHandSideVariableName = RimCalculation::findLeftHandSide(m_expression);

    ExpressionParser parser;

    std::vector<double> a(10);
    std::iota(a.begin(), a.end(), 0);
    
    size_t itemCount = 0;

    for (RimCalculationVariable* v : m_variables)
    {
        itemCount = a.size();
        parser.assignVector(v->name(), a);
    }

    if (itemCount == 0)
    {
        RiaLogging::error("Not able to evaluate expression with no data.");

        return false;
    }

    m_calculatedValues.v().resize(itemCount);
    parser.assignVector(leftHandSideVariableName, m_calculatedValues.v());

    QString errorText;
    bool evaluatedOk = parser.evaluate(m_expression, &errorText);

    if (evaluatedOk)
    {
        QString txt;
        for (auto v : m_calculatedValues())
        {
            txt += QString::number(v);
            txt += " ";
        }

        RiaLogging::info(txt);
    }
    else
    {
        RiaLogging::error(errorText);
    }

    return evaluatedOk;
}

//--------------------------------------------------------------------------------------------------
/// Find the last assignment using := and interpret the text before the := as LHS
//--------------------------------------------------------------------------------------------------
QString RimCalculation::findLeftHandSide(const QString& expresion)
{
    QString exprWithSpace = expresion;
    exprWithSpace.replace("\n", " ");

    QStringList words = exprWithSpace.split(" ", QString::SkipEmptyParts);

    int index = words.lastIndexOf(":=");
    if (index > 0)
    {
        return words[index - 1];
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationVariable* RimCalculation::findByName(const QString& name) const
{
    for (RimCalculationVariable* v : m_variables)
    {
        if (v->name() == name)
        {
            return v;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCalculation::buildCalculationName() const
{
    QString name = "Default Calculation Name";

    QString lhs = RimCalculation::findLeftHandSide(m_expression);
    if (!lhs.isEmpty())
    {
        name = lhs;

        name += " ( ";

        for (RimCalculationVariable* v : m_variables)
        {
            name += v->summaryAddressDisplayString();

            if (v != m_variables[m_variables.size() - 1])
            {
                name += ", ";
            }
        }

        name += " )";
    }

    return name;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_description = buildCalculationName();
}
