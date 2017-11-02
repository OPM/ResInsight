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

#include "RimSummaryCalculation.h"

#include "expressionparser/ExpressionParser.h"

#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiTextEditor.h"

#include <QMessageBox>

#include <algorithm>
#include "QMenu"


CAF_PDM_SOURCE_INIT(RimSummaryCalculation, "RimSummaryCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation::RimSummaryCalculation()
{
    CAF_PDM_InitObject("RimSummaryCalculation", ":/octave.png", "Calculation", "");

    CAF_PDM_InitFieldNoDefault(&m_description, "Description", "Description", "", "", "");
    m_description.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_expression, "Expression", QString("variableName := a"), "Expression", "", "", "");
    m_expression.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_variables, "Variables", "Variables", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_calculatedValues, "CalculatedValues", "Calculated Values", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_timesteps, "TimeSteps", "Time Steps", "", "", "");

    m_exprContextMenuMgr = std::unique_ptr<RiuExpressionContextMenuManager>(new RiuExpressionContextMenuManager());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::setDescription(const QString& description)
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* RimSummaryCalculation::variables()
{
    return &m_variables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable* RimSummaryCalculation::addVariable(const QString& name)
{
    RimSummaryCalculationVariable* v = new RimSummaryCalculationVariable;
    v->setName(name);

    m_variables.push_back(v);

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::deleteVariable(RimSummaryCalculationVariable* calcVariable)
{
    m_variables.removeChildObject(calcVariable);

    delete calcVariable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimSummaryCalculation::values() const
{
    return m_calculatedValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCalculation::timeSteps() const
{
    return m_timesteps();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCalculation::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::parseExpression()
{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide(m_expression);
    if (leftHandSideVariableName.isEmpty())
    {
        QMessageBox::warning(nullptr, "Expression Parser", "Failed to detect left hand side of equation");

        return false;
    }

    std::vector<QString> variableNames = ExpressionParser::detectReferencedVariables(m_expression);
    if (variableNames.size() < 1)
    {
        QMessageBox::warning(nullptr, "Expression Parser", "Failed to detect any variable names");

        return false;
    }

    // Remove variables not present in expression
    {
        std::vector<RimSummaryCalculationVariable*> toBeDeleted;
        for (RimSummaryCalculationVariable* v : m_variables)
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

        for (RimSummaryCalculationVariable* v : toBeDeleted)
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
                this->addVariable(variableName);
            }
        }
    }

    m_description = buildCalculationName();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::calculate()
{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide(m_expression);

    ExpressionParser parser;

    std::vector<std::vector<double>> variableValues;
    variableValues.resize(m_variables.size());

    std::vector<time_t> sourceTimeSteps;

    size_t itemCount = 0;

    for (size_t i = 0; i < m_variables.size(); i++)
    {
        RimSummaryCalculationVariable* v = m_variables[i];

        if (!v->summaryCase())
        {
            QMessageBox::warning(nullptr, "Expression Parser", QString("No summary case defined for variable : %1").arg(v->name()));

            return false;
        }

        if (!v->summaryAddress())
        {
            QMessageBox::warning(nullptr, "Expression Parser", QString("No summary address defined for variable : %1").arg(v->name()));

            return false;
        }

        RimSummaryAddress* sumAdr = v->summaryAddress();
        RiaSummaryCurveDefinition curveDef(v->summaryCase(), v->summaryAddress()->address());

        std::vector<double>& curveValues = variableValues[i];
        RiaSummaryCurveDefinition::resultValues(curveDef, &curveValues);

        if (sourceTimeSteps.size() == 0)
        {
            sourceTimeSteps = RiaSummaryCurveDefinition::timeSteps(curveDef);
        }

        if (itemCount == 0)
        {
            itemCount = curveValues.size();
        }
        else
        {
            if (itemCount != curveValues.size())
            {
                QMessageBox::warning(nullptr, "Expression Parser", QString("Detected varying number of time steps in input vectors. Only vectors with identical number of time steps is supported."));

                return false;
            }
        }

        parser.assignVector(v->name(), curveValues);
    }

    if (itemCount == 0)
    {
        QMessageBox::warning(nullptr, "Expression Parser", QString("Detected zero time steps in input vectors, which is not supported."));

        return false;
    }

    m_calculatedValues.v().resize(itemCount);
    parser.assignVector(leftHandSideVariableName, m_calculatedValues.v());

    QString errorText;
    bool evaluatedOk = parser.evaluate(m_expression, &errorText);

    if (evaluatedOk)
    {
        // Copy time vector from source
        m_timesteps = sourceTimeSteps;
    }
    else
    {
        QString s = "The following error message was received from the parser library : \n\n";
        s += errorText;

        QMessageBox::warning(nullptr, "Expression Parser", s);
    }

    return evaluatedOk;
}

//--------------------------------------------------------------------------------------------------
/// Find the last assignment using := and interpret the text before the := as LHS
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::findLeftHandSide(const QString& expresion)
{
    int index = expresion.lastIndexOf(":=");
    if (index > 0)
    {
        QString s = expresion.left(index).simplified();

        QStringList words = s.split(" ");

        if (words.size() > 0)
        {
            return words.back();
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::attachToWidget()
{
    for (auto e : m_expression.uiCapability()->connectedEditors())
    {
        caf::PdmUiTextEditor* textEditor = dynamic_cast<caf::PdmUiTextEditor*>(e);
        if (!textEditor) continue;

        QWidget* containerWidget = textEditor->editorWidget();
        if (!containerWidget) continue;

        for (auto qObj : containerWidget->children())
        {
            QTextEdit* textEdit = dynamic_cast<QTextEdit*>(qObj);
            if (textEdit)
            {
                m_exprContextMenuMgr->attachTextEdit(textEdit);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable* RimSummaryCalculation::findByName(const QString& name) const
{
    for (RimSummaryCalculationVariable* v : m_variables)
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
QString RimSummaryCalculation::buildCalculationName() const
{
    QString name = "Default Calculation Name";

    QString lhs = RimSummaryCalculation::findLeftHandSide(m_expression);
    if (!lhs.isEmpty())
    {
        name = lhs;

        name += " ( ";

        for (RimSummaryCalculationVariable* v : m_variables)
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
void RimSummaryCalculation::updateDependentCurvesAndPlots()
{
    RimSummaryCalculationCollection* calcColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(calcColl);
    calcColl->rebuildCaseMetaData();

    RimSummaryPlotCollection* summaryPlotCollection = RiaSummaryTools::summaryPlotCollection();
    for (RimSummaryPlot* sumPlot : summaryPlotCollection->summaryPlots())
    {
        bool plotContainsCalculatedCurves = false;

        for (RimSummaryCurve* sumCurve : sumPlot->summaryCurves())
        {
            if (sumCurve->summaryAddress().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
            {
                sumCurve->updateConnectedEditors();

                plotContainsCalculatedCurves = true;
            }
        }

        if (plotContainsCalculatedCurves)
        {
            sumPlot->loadDataAndUpdate();
        }
    }
}
