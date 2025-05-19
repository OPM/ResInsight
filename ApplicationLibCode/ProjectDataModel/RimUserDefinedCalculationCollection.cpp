/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimUserDefinedCalculationCollection.h"

#include "RimProject.h"
#include "RimUserDefinedCalculation.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimUserDefinedCalculationCollection, "RimUserDefinedCalculationCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection::RimUserDefinedCalculationCollection()
{
    CAF_PDM_InitObject( "Calculation Collection", ":/chain.png" );

    CAF_PDM_InitFieldNoDefault( &m_calculations, "Calculations", "Calculations" );
    m_calculations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculation* RimUserDefinedCalculationCollection::addCalculation( bool addDefaultExpression )
{
    RimUserDefinedCalculation* calculation = createCalculation();
    assignCalculationIdToCalculation( calculation );

    if ( addDefaultExpression )
    {
        QString varName = QString( "Calculation_%1" ).arg( calculation->id() );
        calculation->setDescription( varName );
        calculation->setExpression( varName + " := x + y" );
        calculation->parseExpression();
    }

    m_calculations.push_back( calculation );

    rebuildCaseMetaData();

    return calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculation* RimUserDefinedCalculationCollection::addCalculationCopy( const RimUserDefinedCalculation* sourceCalculation )
{
    auto calcCopy = sourceCalculation->copyObject<RimUserDefinedCalculation>();
    CVF_ASSERT( calcCopy );

    std::set<QString> calcNames;
    for ( const auto& calc : m_calculations )
    {
        calcNames.insert( RimUserDefinedCalculation::findLeftHandSide( calc->expression() ) );
    }

    QString expression  = calcCopy->expression();
    QString currVarName = RimUserDefinedCalculation::findLeftHandSide( expression );

    QString newVarName = currVarName;
    while ( calcNames.count( newVarName ) > 0 )
    {
        newVarName += "_copy";
    }

    expression.replace( currVarName, newVarName );
    calcCopy->setExpression( expression );

    assignCalculationIdToCalculation( calcCopy );

    m_calculations.push_back( calcCopy );

    calcCopy->resolveReferencesRecursively();
    rebuildCaseMetaData();
    calcCopy->parseExpression();

    return calcCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculationCollection::deleteCalculation( RimUserDefinedCalculation* calculation )
{
    m_calculations.removeChild( calculation );

    // Call this function after the object is removed from the collection
    calculation->removeDependentObjects();

    rebuildCaseMetaData();

    delete calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimUserDefinedCalculation*> RimUserDefinedCalculationCollection::calculations() const
{
    return m_calculations.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculation* RimUserDefinedCalculationCollection::findCalculationById( int id ) const
{
    for ( RimUserDefinedCalculation* calc : m_calculations )
    {
        if ( calc->id() == id )
        {
            return calc;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculationCollection::deleteAllContainedObjects()
{
    m_calculations.deleteChildren();

    rebuildCaseMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculationCollection::ensureValidCalculationIds()
{
    for ( RimUserDefinedCalculation* calculation : m_calculations )
    {
        if ( calculation->id() == -1 )
        {
            assignCalculationIdToCalculation( calculation );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculationCollection::assignCalculationIdToCalculation( RimUserDefinedCalculation* calculation ) const
{
    int nextValidCalculationId = 1;
    for ( RimUserDefinedCalculation* existingCalculation : calculations() )
    {
        nextValidCalculationId = std::max( nextValidCalculationId, existingCalculation->id() + 1 );
    }

    calculation->setId( nextValidCalculationId++ );
}
