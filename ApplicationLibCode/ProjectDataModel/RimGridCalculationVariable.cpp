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

#include "RimGridCalculationVariable.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"

#include "RimGridCalculation.h"
#include "RiuDragDrop.h"

#include "RigCaseCellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultAddress.h"
#include "RimEclipseView.h"
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimGridCalculationVariable, "RimGridCalculationVariable" );

namespace caf
{
template <>
void caf::AppEnum<RimGridCalculationVariable::DefaultValueType>::setUp()
{
    addItem( RimGridCalculationVariable::DefaultValueType::POSITIVE_INFINITY, "POSITIVE_INFINITY", "Inf" );
    addItem( RimGridCalculationVariable::DefaultValueType::FROM_PROPERTY, "FROM_PROPERTY", "Property Value" );
    addItem( RimGridCalculationVariable::DefaultValueType::USER_DEFINED, "USER_DEFINED", "User Defined" );
    setDefault( RimGridCalculationVariable::DefaultValueType::POSITIVE_INFINITY );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::RimGridCalculationVariable()
{
    CAF_PDM_InitObject( "RimGridCalculationVariable", ":/octave.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultType, "ResultType", "Type" );
    CAF_PDM_InitField( &m_resultVariable, "ResultVariable", RiaResultNames::undefinedResultName(), "Variable" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseGridCase", "Grid Case" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", allTimeStepsValue(), "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility" );
    CAF_PDM_InitFieldNoDefault( &m_defaultValueType, "DefaultValueType", "Default Value Type" );
    CAF_PDM_InitField( &m_defaultValue, "DefaultValue", 0.0, "Default Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCalculationVariable::displayString() const
{
    QStringList nameComponents;

    if ( m_eclipseCase() ) nameComponents.append( m_eclipseCase()->uiName() );

    nameComponents.append( m_resultVariable() );
    return nameComponents.join( " - " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.add( &m_resultType );
    uiOrdering.add( &m_resultVariable );
    uiOrdering.add( &m_timeStep );
    uiOrdering.add( &m_cellFilterView );
    uiOrdering.add( &m_defaultValueType );
    uiOrdering.add( &m_defaultValue );

    uiOrdering.skipRemainingFields();

    m_resultType.uiCapability()->setUiReadOnly( m_eclipseCase == nullptr );
    m_timeStep.uiCapability()->setUiReadOnly( m_resultType == RiaDefines::ResultCatType::STATIC_NATIVE );

    // Can have only one variable with cell filter at a time.
    // Set read-only state based on the state of the sibling variables.
    RimGridCalculation* calculation = nullptr;
    firstAncestorOfType( calculation );

    bool hasOtherVariableWithFilter = false;
    for ( auto variable : calculation->allVariables() )
    {
        auto v = dynamic_cast<RimGridCalculationVariable*>( variable );
        if ( variable != this && v->cellFilterView() )
        {
            hasOtherVariableWithFilter = true;
        }
    }

    m_cellFilterView.uiCapability()->setUiReadOnly( m_eclipseCase == nullptr || hasOtherVariableWithFilter );
    m_defaultValueType.uiCapability()->setUiReadOnly( m_cellFilterView == nullptr || hasOtherVariableWithFilter );
    m_defaultValue.uiCapability()->setUiReadOnly(
        m_cellFilterView == nullptr ||
        defaultValueType() != RimGridCalculationVariable::DefaultValueType::USER_DEFINED || hasOtherVariableWithFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGridCalculationVariable::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultType )
    {
        std::vector<RiaDefines::ResultCatType> resultCategories = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                    RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                                    RiaDefines::ResultCatType::GENERATED };

        for ( auto c : resultCategories )
        {
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ResultCatType>( c ).uiText(), c ) );
        }
    }
    else if ( fieldNeedingOptions == &m_resultVariable )
    {
        auto results = currentGridCellResults();
        if ( results )
        {
            for ( const QString& s : getResultNamesForResultType( m_resultType(), results ) )
            {
                options.push_back( caf::PdmOptionItemInfo( s, s ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        options.push_back( caf::PdmOptionItemInfo( "All timesteps", allTimeStepsValue() ) );

        RimTools::timeStepsForCase( m_eclipseCase(), &options );
    }
    else if ( fieldNeedingOptions == &m_cellFilterView )
    {
        if ( m_eclipseCase )
        {
            options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );
            for ( RimEclipseView* view : m_eclipseCase->reservoirViews.children() )
            {
                CVF_ASSERT( view && "Really always should have a valid view pointer in ReservoirViews" );
                options.push_back( caf::PdmOptionItemInfo( view->name(), view, false, view->uiIconProvider() ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimGridCalculationVariable::currentGridCellResults() const
{
    if ( !m_eclipseCase ) return nullptr;

    return m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimGridCalculationVariable::getResultNamesForResultType( RiaDefines::ResultCatType     resultCatType,
                                                                     const RigCaseCellResultsData* results )
{
    if ( !results ) return QStringList();
    return results->resultNames( resultCatType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridCalculationVariable::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ResultCatType RimGridCalculationVariable::resultCategoryType() const
{
    return m_resultType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCalculationVariable::resultVariable() const
{
    return m_resultVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCalculationVariable::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCalculationVariable::allTimeStepsValue()
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimGridCalculationVariable::cellFilterView() const
{
    return m_cellFilterView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGridCalculationVariable::defaultValue() const
{
    return m_defaultValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::DefaultValueType RimGridCalculationVariable::defaultValueType() const
{
    return m_defaultValueType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::DefaultValueConfig RimGridCalculationVariable::defaultValueConfiguration() const
{
    if ( m_defaultValueType() == RimGridCalculationVariable::DefaultValueType::USER_DEFINED )
        return std::make_pair( m_defaultValueType(), m_defaultValue() );

    return std::make_pair( m_defaultValueType(), HUGE_VAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::handleDroppedMimeData( const QMimeData*     data,
                                                        Qt::DropAction       action,
                                                        caf::PdmFieldHandle* destinationField )
{
    auto objects = RiuDragDrop::convertToObjects( data );
    if ( !objects.empty() )
    {
        auto address = dynamic_cast<RimEclipseResultAddress*>( objects.front() );
        if ( address ) setEclipseResultAddress( *address );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::setEclipseResultAddress( const RimEclipseResultAddress& address )
{
    m_resultVariable = address.resultName();
    m_resultType     = address.resultType();
    m_eclipseCase    = address.eclipseCase();
    updateConnectedEditors();
}
