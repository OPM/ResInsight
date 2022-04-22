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

#include "RimUserDefinedCalculation.h"

#include "expressionparser/ExpressionParser.h"

#include "RiaLogging.h"

#include "RimProject.h"
#include "RimUserDefinedCalculationVariable.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"

#include <algorithm>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimUserDefinedCalculation, "RimUserDefinedCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculation::RimUserDefinedCalculation()
{
    CAF_PDM_InitObject( "RimUserDefinedCalculation", ":/octave.png", "Calculation", "" );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_expression, "Expression", QString( "" ), "Expression" );
    m_expression.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_unit, "Unit", QString( "" ), "Unit" );
    m_unit.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_variables, "Variables", "Variables" );
    CAF_PDM_InitFieldNoDefault( &m_calculatedValues, "CalculatedValues", "Calculated Values" );

    CAF_PDM_InitFieldNoDefault( &m_timesteps, "TimeSteps", "Time Steps" );
    CAF_PDM_InitField( &m_id, "Id", -1, "Id" );
    m_id.uiCapability()->setUiHidden( true );

    m_exprContextMenuMgr = std::unique_ptr<RiuExpressionContextMenuManager>( new RiuExpressionContextMenuManager() );

    m_isDirty = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculation::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedCalculation::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculation::setId( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimUserDefinedCalculation::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimUserDefinedCalculation::isDirty() const
{
    return m_isDirty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* RimUserDefinedCalculation::variables()
{
    return &m_variables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationVariable* RimUserDefinedCalculation::addVariable( const QString& name )
{
    RimUserDefinedCalculationVariable* v = createVariable();
    v->setName( name );

    m_variables.push_back( v );

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculation::deleteVariable( RimUserDefinedCalculationVariable* calcVariable )
{
    m_variables.removeChildObject( calcVariable );

    delete calcVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimUserDefinedCalculation::values() const
{
    return m_calculatedValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimUserDefinedCalculation::timeSteps() const
{
    return m_timesteps();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculation::setExpression( const QString& expr )
{
    m_expression = expr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedCalculation::expression() const
{
    return m_expression;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedCalculation::unitName() const
{
    return m_unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimUserDefinedCalculation::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimUserDefinedCalculation::parseExpression()
{
    QString leftHandSideVariableName = RimUserDefinedCalculation::findLeftHandSide( m_expression );
    if ( leftHandSideVariableName.isEmpty() )
    {
        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", "Failed to detect left hand side of equation" );

        return false;
    }

    std::vector<QString> variableNames = ExpressionParser::detectReferencedVariables( m_expression );
    if ( variableNames.size() < 1 )
    {
        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", "Failed to detect any variable names" );

        return false;
    }

    // Remove variables not present in expression
    {
        std::vector<RimUserDefinedCalculationVariable*> toBeDeleted;
        for ( RimUserDefinedCalculationVariable* v : m_variables )
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

        for ( RimUserDefinedCalculationVariable* v : toBeDeleted )
        {
            deleteVariable( v );
        }
    }

    for ( auto variableName : variableNames )
    {
        if ( leftHandSideVariableName != variableName )
        {
            if ( !findByName( variableName ) )
            {
                this->addVariable( variableName );
            }
        }
    }

    m_description = buildCalculationName();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Find the last assignment using := and interpret the text before the := as LHS
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedCalculation::findLeftHandSide( const QString& expression )
{
    int index = expression.lastIndexOf( ":=" );
    if ( index > 0 )
    {
        QString s = expression.left( index ).simplified();

        QStringList words = s.split( " " );

        if ( words.size() > 0 )
        {
            return words.back();
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculation::attachToWidget()
{
    for ( auto e : m_expression.uiCapability()->connectedEditors() )
    {
        caf::PdmUiTextEditor* textEditor = dynamic_cast<caf::PdmUiTextEditor*>( e );
        if ( !textEditor ) continue;

        QWidget* containerWidget = textEditor->editorWidget();
        if ( !containerWidget ) continue;

        for ( auto qObj : containerWidget->children() )
        {
            QTextEdit* textEdit = dynamic_cast<QTextEdit*>( qObj );
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
void RimUserDefinedCalculation::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    m_isDirty = true;

    PdmObject::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationVariable* RimUserDefinedCalculation::findByName( const QString& name ) const
{
    for ( RimUserDefinedCalculationVariable* v : m_variables )
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
QString RimUserDefinedCalculation::buildCalculationName() const
{
    QString name = "Default Calculation Name";

    QString lhs = RimUserDefinedCalculation::findLeftHandSide( m_expression );
    if ( !lhs.isEmpty() )
    {
        name = lhs;

        name += " ( ";

        for ( RimUserDefinedCalculationVariable* v : m_variables )
        {
            name += v->displayString();

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
void RimUserDefinedCalculation::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_expression )
    {
        caf::PdmUiTextEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->heightHint = -1;
        }
    }
    else if ( field == &m_variables )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->enableDropTarget = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimUserDefinedCalculationVariable*> RimUserDefinedCalculation::allVariables() const
{
    std::vector<RimUserDefinedCalculationVariable*> outVariables;
    for ( RimUserDefinedCalculationVariable* v : m_variables )
        outVariables.push_back( v );

    return outVariables;
}
