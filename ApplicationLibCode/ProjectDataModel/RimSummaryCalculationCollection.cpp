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

#include "RimSummaryCalculationCollection.h"

#include "RimCalculatedSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCalculation.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryCalculationCollection, "RimSummaryCalculationCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationCollection::RimSummaryCalculationCollection()
{
    CAF_PDM_InitObject( "Calculation Collection", ":/chain.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_calculations, "Calculations", "Calculations", "", "", "" );
    m_calculations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_calcuationSummaryCase, "CalculationsSummaryCase", "Calculations Summary Case", "", "", "" );
    m_calcuationSummaryCase.xmlCapability()->disableIO();
    m_calcuationSummaryCase = new RimCalculatedSummaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RimSummaryCalculationCollection::addCalculation()
{
    RimSummaryCalculation* calculation = new RimSummaryCalculation;
    RimProject::current()->assignCalculationIdToCalculation( calculation );

    QString varName = QString( "Calculation_%1" ).arg( calculation->id() );
    calculation->setDescription( varName );
    calculation->setExpression( varName + " := x + y" );
    calculation->parseExpression();

    m_calculations.push_back( calculation );

    rebuildCaseMetaData();

    return calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationBase* RimSummaryCalculationCollection::addCalculationWithValues( const QString& description,
                                                                                      const std::vector<double>& values,
                                                                                      const std::vector<time_t>& timeSteps )
{
    RimSummaryValuesFromScript* calculation = new RimSummaryValuesFromScript;
    RimProject::current()->assignCalculationIdToCalculation( calculation );

    calculation->setDescription( description );
    calculation->setValuesAndTimeSteps( values, timeSteps );

    m_calculations.push_back( calculation );

    rebuildCaseMetaData();

    return calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RimSummaryCalculationCollection::addCalculationCopy( const RimSummaryCalculation* sourceCalculation )
{
    RimSummaryCalculation* calcCopy = dynamic_cast<RimSummaryCalculation*>(
        sourceCalculation->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( calcCopy );

    std::set<QString> calcNames;
    for ( const auto& calcBase : m_calculations )
    {
        auto calc = dynamic_cast<RimSummaryCalculation*>( calcBase.p() );
        if ( calc )
        {
            calcNames.insert( calc->findLeftHandSide( calc->expression() ) );
        }
    }

    QString expression  = calcCopy->expression();
    QString currVarName = calcCopy->findLeftHandSide( expression );

    QString newVarName = currVarName;
    while ( calcNames.count( newVarName ) > 0 )
    {
        newVarName += "_copy";
    }

    expression.replace( currVarName, newVarName );
    calcCopy->setExpression( expression );

    m_calculations.push_back( calcCopy );

    calcCopy->resolveReferencesRecursively();
    rebuildCaseMetaData();
    calcCopy->parseExpression();

    return calcCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::deleteCalculation( RimSummaryCalculationBase* calculation )
{
    m_calculations.removeChildObject( calculation );

    rebuildCaseMetaData();

    delete calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculationBase*> RimSummaryCalculationCollection::calculations() const
{
    return m_calculations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculation*> RimSummaryCalculationCollection::textExpressionCalculations() const
{
    std::vector<RimSummaryCalculation*> expressionObjects;

    for ( const auto& calcBase : m_calculations )
    {
        auto calc = dynamic_cast<RimSummaryCalculation*>( calcBase.p() );
        if ( calc )
        {
            expressionObjects.push_back( calc );
        }
    }

    return expressionObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationBase* RimSummaryCalculationCollection::findCalculationById( int id ) const
{
    for ( auto calc : m_calculations )
    {
        if ( calc->id() == id )
        {
            return calc.p();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCalculationCollection::calculationSummaryCase()
{
    return m_calcuationSummaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::deleteAllContainedObjects()
{
    m_calculations.deleteAllChildObjects();

    rebuildCaseMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::rebuildCaseMetaData()
{
    for ( auto calculation : m_calculations )
    {
        if ( calculation->id() == -1 )
        {
            RimProject::current()->assignCalculationIdToCalculation( calculation );
        }
    }

    m_calcuationSummaryCase->buildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::initAfterRead()
{
    rebuildCaseMetaData();
}
