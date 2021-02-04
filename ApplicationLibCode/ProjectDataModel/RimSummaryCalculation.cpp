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

#include "RiaCurveMerger.h"
#include "RiaLogging.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimSummaryCalculation, "RimSummaryCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation::RimSummaryCalculation()
{
    CAF_PDM_InitObject( "RimSummaryCalculation", ":/octave.png", "Calculation", "" );

    CAF_PDM_InitField( &m_expression, "Expression", QString( "" ), "Expression", "", "", "" );
    m_expression.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_unit, "Unit", QString( "" ), "Unit", "", "", "" );
    m_unit.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_variables, "Variables", "Variables", "", "", "" );

    m_exprContextMenuMgr = std::make_unique<RiuExpressionContextMenuManager>();

    m_isDirty = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::isDirty() const
{
    return m_isDirty;
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
RimSummaryCalculationVariable* RimSummaryCalculation::addVariable( const QString& name )
{
    RimSummaryCalculationVariable* v = new RimSummaryCalculationVariable;
    v->setName( name );

    m_variables.push_back( v );

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::deleteVariable( RimSummaryCalculationVariable* calcVariable )
{
    m_variables.removeChildObject( calcVariable );

    delete calcVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::setExpression( const QString& expr )
{
    m_expression = expr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::expression() const
{
    return m_expression;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::unitName() const
{
    return m_unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::parseExpression()
{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( m_expression );
    if ( leftHandSideVariableName.isEmpty() )
    {
        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", "Failed to detect left hand side of equation" );

        return false;
    }

    std::vector<QString> variableNames = ExpressionParser::detectReferencedVariables( m_expression );
    if ( variableNames.empty() )
    {
        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", "Failed to detect any variable names" );

        return false;
    }

    // Remove variables not present in expression
    {
        std::vector<RimSummaryCalculationVariable*> toBeDeleted;
        for ( RimSummaryCalculationVariable* v : m_variables )
        {
            if ( std::find( variableNames.begin(), variableNames.end(), v->name() ) == variableNames.end() )
            {
                toBeDeleted.push_back( v );
            }

            if ( leftHandSideVariableName == v->name() )
            {
                toBeDeleted.push_back( v );
            }
        }

        for ( RimSummaryCalculationVariable* v : toBeDeleted )
        {
            deleteVariable( v );
        }
    }

    for ( const auto& variableName : variableNames )
    {
        if ( leftHandSideVariableName != variableName )
        {
            if ( !findByName( variableName ) )
            {
                this->addVariable( variableName );
            }
        }
    }

    setDescription( buildCalculationName() );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::calculate()
{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( m_expression );

    RiaTimeHistoryCurveMerger timeHistoryCurveMerger;

    for ( RimSummaryCalculationVariable* v : m_variables )
    {
        if ( !v->summaryCase() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary case defined for variable : %1" ).arg( v->name() ) );

            return false;
        }

        if ( !v->summaryAddress() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary address defined for variable : %1" ).arg( v->name() ) );

            return false;
        }

        RiaSummaryCurveDefinition curveDef( v->summaryCase(), v->summaryAddress()->address(), false );

        std::vector<double> curveValues;
        RiaSummaryCurveDefinition::resultValues( curveDef, &curveValues );

        std::vector<time_t> curveTimeSteps = RiaSummaryCurveDefinition::timeSteps( curveDef );

        if ( !curveTimeSteps.empty() && !curveValues.empty() )
        {
            timeHistoryCurveMerger.addCurveData( curveTimeSteps, curveValues );
        }
    }

    timeHistoryCurveMerger.computeInterpolatedValues();

    ExpressionParser parser;
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = m_variables[i];

        parser.assignVector( v->name(), timeHistoryCurveMerger.interpolatedYValuesForAllXValues( i ) );
    }

    std::vector<double> resultValues;
    resultValues.resize( timeHistoryCurveMerger.allXValues().size() );

    parser.assignVector( leftHandSideVariableName, resultValues );

    QString errorText;
    bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

    if ( evaluatedOk )
    {
        setTimeSteps( {} );
        setValues( {} );

        if ( !timeHistoryCurveMerger.validIntervalsForAllXValues().empty() )
        {
            size_t firstValidTimeStep = timeHistoryCurveMerger.validIntervalsForAllXValues().front().first;
            size_t lastValidTimeStep  = timeHistoryCurveMerger.validIntervalsForAllXValues().back().second + 1;

            if ( lastValidTimeStep > firstValidTimeStep && lastValidTimeStep <= timeHistoryCurveMerger.allXValues().size() )
            {
                std::vector<time_t> validTimeSteps( timeHistoryCurveMerger.allXValues().begin() + firstValidTimeStep,
                                                    timeHistoryCurveMerger.allXValues().begin() + lastValidTimeStep );

                std::vector<double> validValues( resultValues.begin() + firstValidTimeStep,
                                                 resultValues.begin() + lastValidTimeStep );

                setTimeSteps( validTimeSteps );
                setValues( validValues );
            }
        }

        m_isDirty = false;
    }
    else
    {
        QString s = "The following error message was received from the parser library : \n\n";
        s += errorText;

        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", s );
    }

    return evaluatedOk;
}

//--------------------------------------------------------------------------------------------------
/// Find the last assignment using := and interpret the text before the := as LHS
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::findLeftHandSide( const QString& expression )
{
    int index = expression.lastIndexOf( ":=" );
    if ( index > 0 )
    {
        QString s = expression.left( index ).simplified();

        QStringList words = s.split( " " );

        if ( !words.empty() )
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
    for ( auto e : m_expression.uiCapability()->connectedEditors() )
    {
        auto* textEditor = dynamic_cast<caf::PdmUiTextEditor*>( e );
        if ( !textEditor ) continue;

        QWidget* containerWidget = textEditor->editorWidget();
        if ( !containerWidget ) continue;

        for ( auto qObj : containerWidget->children() )
        {
            auto* textEdit = dynamic_cast<QTextEdit*>( qObj );
            if ( textEdit )
            {
                m_exprContextMenuMgr->attachTextEdit( textEdit );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::isCumulative() const
{
    for ( RimSummaryCalculationVariable* v : m_variables() )
    {
        if ( !v->summaryAddress()->address().hasAccumulatedData() )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    m_isDirty = true;

    PdmObject::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable* RimSummaryCalculation::findByName( const QString& name ) const
{
    for ( RimSummaryCalculationVariable* v : m_variables )
    {
        if ( v->name() == name )
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

    QString lhs = RimSummaryCalculation::findLeftHandSide( m_expression );
    if ( !lhs.isEmpty() )
    {
        name = lhs;

        name += " ( ";

        for ( RimSummaryCalculationVariable* v : m_variables )
        {
            name += v->summaryAddressDisplayString();

            if ( v != m_variables[m_variables.size() - 1] )
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
void RimSummaryCalculation::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_expression )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->heightHint = -1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::updateDependentCurvesAndPlots()
{
    RimSummaryCalculationCollection* calcColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( calcColl );
    calcColl->rebuildCaseMetaData();

    RimSummaryPlotCollection* summaryPlotCollection = RiaSummaryTools::summaryPlotCollection();
    for ( RimSummaryPlot* sumPlot : summaryPlotCollection->plots() )
    {
        bool plotContainsCalculatedCurves = false;

        for ( RimSummaryCurve* sumCurve : sumPlot->summaryCurves() )
        {
            if ( sumCurve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
            {
                sumCurve->updateConnectedEditors();

                plotContainsCalculatedCurves = true;
            }
        }

        if ( plotContainsCalculatedCurves )
        {
            sumPlot->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculationVariable*> RimSummaryCalculation::allVariables() const
{
    std::vector<RimSummaryCalculationVariable*> outVariables;
    for ( RimSummaryCalculationVariable* v : m_variables )
        outVariables.push_back( v );

    return outVariables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_ABSTRACT_SOURCE_INIT( RimSummaryCalculationBase, "RimSummaryCalculationBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationBase::setValues( const std::vector<double>& values )
{
    m_calculatedValues = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationBase::setTimeSteps( const std::vector<time_t>& timeSteps )
{
    m_timesteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationBase::RimSummaryCalculationBase()
{
    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description", "", "", "" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_calculatedValues, "CalculatedValues", "Calculated Values", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_timesteps, "TimeSteps", "Time Steps", "", "", "" );

    CAF_PDM_InitField( &m_id, "Id", -1, "Id", "", "", "" );
    m_id.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationBase::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculationBase::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationBase::setId( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryCalculationBase::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimSummaryCalculationBase::values() const
{
    return m_calculatedValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCalculationBase::timeSteps() const
{
    return m_timesteps();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCalculationBase::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT( RimSummaryValuesFromScript, "RimSummaryValuesFromScript" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryValuesFromScript::RimSummaryValuesFromScript()
{
    CAF_PDM_InitObject( "RimSummaryValuesFromScript", "", "RimSummaryValuesFromScript", "" );

    CAF_PDM_InitField( &m_isCumulative, "IsCumulative", true, "IsCumulative", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryValuesFromScript::setValuesAndTimeSteps( const std::vector<double>& values,
                                                        const std::vector<time_t>& timeSteps )
{
    setValues( values );
    setTimeSteps( timeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryValuesFromScript::setCumulative( bool enable )
{
    m_isCumulative = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryValuesFromScript::isCumulative() const
{
    return m_isCumulative;
}
