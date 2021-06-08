/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCustomObjectiveFunction.h"

#include "RimCustomObjectiveFunctionWeight.h"
#include "RimEnsembleCurveSet.h"
#include "RimSummaryCaseCollection.h"

#include <cafPdmUiLineEditor.h>
#include <cafPdmUiTreeOrdering.h>

#include <QString>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RimCustomObjectiveFunction, "RimCustomObjectiveFunction" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunction::RimCustomObjectiveFunction()
{
    CAF_PDM_InitObject( "Objective Function", ":/ObjectiveFunction.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_functionTitle, "FunctionTitle", "Title", "", "", "" );
    m_functionTitle.registerGetMethod( this, &RimCustomObjectiveFunction::title );
    m_functionTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_customFunctionTitle, "CustomFunctionTitle", "Title", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_weights, "Weights", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunctions, "ObjectiveFunctions", "Objective Functions", "", "", "" );

    {
        auto objFunc1 = new RimObjectiveFunction();
        objFunc1->setFunctionType( RimObjectiveFunction::FunctionType::F1 );
        m_objectiveFunctions.push_back( objFunc1 );
    }
    {
        auto objFunc1 = new RimObjectiveFunction();
        objFunc1->setFunctionType( RimObjectiveFunction::FunctionType::F2 );
        m_objectiveFunctions.push_back( objFunc1 );
    }

    m_isValid = true;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionWeight* RimCustomObjectiveFunction::addWeight()
{
    auto newWeight = new RimCustomObjectiveFunctionWeight();
    m_weights.push_back( newWeight );
    return newWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCustomObjectiveFunctionWeight*> RimCustomObjectiveFunction::weights() const
{
    return m_weights.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimCustomObjectiveFunction::functionValueForAllCases() const
{
    if ( m_weights.empty() ) return {};

    RimSummaryCaseCollection* caseCollection = parentCurveSet()->summaryCaseCollection();
    if ( !caseCollection || caseCollection->allSummaryCases().empty() ) return {};

    std::vector<double> values;

    for ( auto sumCase : caseCollection->allSummaryCases() )
    {
        auto functionValue = value( sumCase );
        values.push_back( functionValue );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimCustomObjectiveFunction::value( RimSummaryCase* summaryCase ) const
{
    RimSummaryCaseCollection* caseCollection = parentCurveSet()->summaryCaseCollection();

    if ( m_functionValueForAllCases.count( summaryCase ) > 0 )
    {
        return m_functionValueForAllCases[summaryCase];
    }

    double value = 0.0;
    for ( auto weight : m_weights )
    {
        double functionValue =
            objectiveFunction( weight->objectiveFunction() )
                ->value( summaryCase, weight->summaryAddresses(), parentCurveSet()->objectiveFunctionTimeConfig() );

        value += weight->weightValue() * functionValue;
    }

    m_functionValueForAllCases[summaryCase] = value;

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimCustomObjectiveFunction::minMaxValues() const
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto value : functionValueForAllCases() )
    {
        if ( value != std::numeric_limits<double>::infinity() )
        {
            if ( value < minValue ) minValue = value;
            if ( value > maxValue ) maxValue = value;
        }
    }
    return std::make_pair( minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomObjectiveFunction::weightContainsFunctionType( RimObjectiveFunction::FunctionType functionType ) const
{
    for ( auto weight : m_weights )
    {
        if ( weight->objectiveFunction() == functionType )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomObjectiveFunction::title() const
{
    if ( m_customFunctionTitle().isEmpty() )
    {
        return autoGeneratedTitle();
    }
    else
    {
        return m_customFunctionTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomObjectiveFunction::autoGeneratedTitle() const
{
    QStringList titles;
    for ( auto weight : m_weights )
    {
        titles << weight->title();
    }
    if ( titles.count() == 0 )
    {
        titles << QString( "New Objective Function" );
    }
    return titles.join( " + " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomObjectiveFunction::isValid() const
{
    return m_weights.size() > 0 && m_isValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::onWeightChanged()
{
    m_functionValueForAllCases.clear();
    parentCollection()->onObjectiveFunctionChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::invalidate()
{
    m_isValid = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomObjectiveFunction::formulaString( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const
{
    QString formula = "Custom Objective Function = ";
    if ( !m_customFunctionTitle().isEmpty() )
    {
        formula = m_customFunctionTitle();
    }
    QStringList weightFormulae;
    for ( auto weight : weights() )
    {
        weightFormulae
            << QString( "%0 * %1" )
                   .arg( weight->weightValue() )
                   .arg( objectiveFunction( weight->objectiveFunction() )->formulaString( vectorSummaryAddresses ) );
    }
    formula += weightFormulae.join( " + " );
    return formula;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::clearCache()
{
    m_functionValueForAllCases.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCustomObjectiveFunction::userDescriptionField()
{
    return &m_functionTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                       QString                 uiConfigName /* = "" */ )
{
    for ( auto weight : weights() )
    {
        uiTreeOrdering.add( weight );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_customFunctionTitle )
    {
        caf::PdmUiLineEditorAttribute* attrib = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->placeholderText = autoGeneratedTitle();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimCustomObjectiveFunction::parentCurveSet() const
{
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType( curveSet );
    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionCollection* RimCustomObjectiveFunction::parentCollection() const
{
    RimCustomObjectiveFunctionCollection* collection;
    firstAncestorOrThisOfType( collection );
    return collection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction* RimCustomObjectiveFunction::objectiveFunction( RimObjectiveFunction::FunctionType functionType ) const
{
    for ( auto objectiveFunc : m_objectiveFunctions.childObjects() )
    {
        if ( objectiveFunc->functionType() == functionType )
        {
            return objectiveFunc;
        }
    }
    return nullptr;
}
