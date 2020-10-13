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

#include <cafPdmUiTreeOrdering.h>

#include <QString>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RimCustomObjectiveFunction, "RimCustomObjectiveFunction" );

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static std::vector<RimCustomObjectiveFunctionWeight*> _removedWeights;

static void garbageCollectWeights();

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunction::RimCustomObjectiveFunction()
    : objectiveFunctionChanged( this )
{
    CAF_PDM_InitObject( "Objective Function", ":/ObjectiveFunction.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_functionTitle, "FunctionTitle", "Title", "", "", "" );
    m_functionTitle.registerGetMethod( this, &RimCustomObjectiveFunction::title );
    m_functionTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_weights, "Weights", "", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionWeight* RimCustomObjectiveFunction::addWeight()
{
    garbageCollectWeights();

    auto newWeight = new RimCustomObjectiveFunctionWeight();
    m_weights.push_back( newWeight );
    objectiveFunctionChanged.send();
    return newWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::removeWeight( RimCustomObjectiveFunctionWeight* weight )
{
    garbageCollectWeights();

    size_t sizeBefore = m_weights.size();
    m_weights.removeChildObject( weight );
    size_t sizeAfter = m_weights.size();
    objectiveFunctionChanged.send();
    if ( sizeAfter < sizeBefore ) _removedWeights.push_back( weight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCustomObjectiveFunctionWeight*> RimCustomObjectiveFunction::weights()
{
    return m_weights.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimCustomObjectiveFunction::values() const
{
    std::vector<double>       values;
    RimSummaryCaseCollection* caseCollection = parentCurveSet()->summaryCaseCollection();
    for ( auto weight : m_weights )
    {
        std::vector<double> functionValues =
            caseCollection->objectiveFunction( weight->objectiveFunction() )->values( weight->summaryAddress() );
        if ( values.size() == 0 )
        {
            for ( size_t i = 0; i < functionValues.size(); i++ )
            {
                values.push_back( weight->weightValue() * functionValues[i] );
            }
        }
        else
        {
            for ( size_t i = 0; i < functionValues.size(); i++ )
            {
                if ( values.size() > i + 1 )
                {
                    values[i] += weight->weightValue() * functionValues[i];
                }
            }
        }
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimCustomObjectiveFunction::value( RimSummaryCase* summaryCase ) const
{
    double                    value          = 0.0;
    RimSummaryCaseCollection* caseCollection = parentCurveSet()->summaryCaseCollection();
    for ( auto weight : m_weights )
    {
        double functionValue =
            caseCollection->objectiveFunction( weight->objectiveFunction() )->value( summaryCase, weight->summaryAddress() );

        value += weight->weightValue() * functionValue;
    }
    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimCustomObjectiveFunction::minMaxValues()
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto value : values() )
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
bool RimCustomObjectiveFunction::weightContainsFunctionType( ObjectiveFunction::FunctionType functionType ) const
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
    return m_weights.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunction::weightUpdated()
{
    objectiveFunctionChanged.send();
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
RimEnsembleCurveSet* RimCustomObjectiveFunction::parentCurveSet() const
{
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType( curveSet );
    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void garbageCollectWeights()
{
    for ( auto weight : _removedWeights )
    {
        delete weight;
    }
    _removedWeights.clear();
}
