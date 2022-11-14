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

#include "RimGridCalculation.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGridCalculationVariable.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "expressionparser/ExpressionParser.h"

CAF_PDM_SOURCE_INIT( RimGridCalculation, "RimGridCalculation" );

namespace caf
{
template <>
void caf::AppEnum<RimGridCalculation::DefaultValueType>::setUp()
{
    addItem( RimGridCalculation::DefaultValueType::POSITIVE_INFINITY, "POSITIVE_INFINITY", "Infinity" );
    addItem( RimGridCalculation::DefaultValueType::FROM_PROPERTY, "FROM_PROPERTY", "Property Value" );
    addItem( RimGridCalculation::DefaultValueType::USER_DEFINED, "USER_DEFINED", "User Defined Custom Value" );
    setDefault( RimGridCalculation::DefaultValueType::POSITIVE_INFINITY );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation::RimGridCalculation()
{
    CAF_PDM_InitObject( "RimGridCalculation", ":/octave.png", "Calculation", "" );
    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility" );
    CAF_PDM_InitFieldNoDefault( &m_defaultValueType, "DefaultValueType", "Non-visible Cell Value" );
    CAF_PDM_InitField( &m_defaultValue, "DefaultValue", 0.0, "Custom Value" );
    CAF_PDM_InitFieldNoDefault( &m_destinationCase, "DestinationCase", "Destination Case" );
    CAF_PDM_InitField( &m_defaultPropertyVariableIndex, "DefaultPropertyVariableName", 0, "Property Variable Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable* RimGridCalculation::createVariable()
{
    auto variable = new RimGridCalculationVariable;

    variable->eclipseResultChanged.connect( this, &RimGridCalculation::onVariableUpdated );

    return variable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::calculate()
{
    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    RimEclipseCase* eclipseCase = destinationEclipseCase();
    if ( !eclipseCase )
    {
        RiaLogging::errorInMessageBox( nullptr,
                                       "Grid Property Calculator",
                                       QString( "No case found for calculation : %1" ).arg( leftHandSideVariableName ) );
        return false;
    }

    auto [isOk, errorMessage] = validateVariables();
    if ( !isOk )
    {
        RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        return false;
    }

    for ( auto variableCase : inputCases() )
    {
        if ( !eclipseCase->isGridSizeEqualTo( variableCase ) )
        {
            QString msg = "Detected IJK mismatch between input cases and destination case. All grid "
                          "cases must have identical IJK sizes.";
            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", msg );
            return false;
        }
    }

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    if ( !eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
    {
        eclipseCase->results( porosityModel )->createResultEntry( resAddr, true );
    }

    eclipseCase->results( porosityModel )->clearScalarResult( resAddr );

    const size_t timeStepCount = eclipseCase->results( porosityModel )->maxTimeStepCount();

    std::vector<std::vector<double>>* scalarResultFrames =
        eclipseCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
    scalarResultFrames->resize( timeStepCount );

    for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
    {
        std::vector<std::vector<double>> values;
        for ( size_t i = 0; i < m_variables.size(); i++ )
        {
            RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
            CAF_ASSERT( v != nullptr );
            values.push_back( getInputVectorForVariable( v, tsId, porosityModel ) );
        }

        ExpressionParser parser;
        for ( size_t i = 0; i < m_variables.size(); i++ )
        {
            RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
            CAF_ASSERT( v != nullptr );
            parser.assignVector( v->name(), values[i] );
        }

        std::vector<double> resultValues;
        resultValues.resize( values[0].size() );
        parser.assignVector( leftHandSideVariableName, resultValues );

        QString errorText;
        bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

        if ( evaluatedOk )
        {
            if ( m_cellFilterView() )
            {
                filterResults( m_cellFilterView(), values, m_defaultValueType(), m_defaultValue(), resultValues );
            }

            scalarResultFrames->at( tsId ) = resultValues;

            m_isDirty = false;
        }
        else
        {
            QString s = "The following error message was received from the parser library : \n\n";
            s += errorText;

            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", s );
            return false;
        }
    }

    eclipseCase->updateResultAddressCollection();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridCalculation::destinationEclipseCase() const
{
    return m_destinationCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimGridCalculation::inputCases() const
{
    std::vector<RimEclipseCase*> cases;

    for ( const auto& variable : m_variables )
    {
        auto* v = dynamic_cast<RimGridCalculationVariable*>( variable.p() );

        if ( v->eclipseCase() ) cases.push_back( v->eclipseCase() );
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation::DefaultValueConfig RimGridCalculation::defaultValueConfiguration() const
{
    if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::USER_DEFINED )
        return std::make_pair( m_defaultValueType(), m_defaultValue() );

    return std::make_pair( m_defaultValueType(), HUGE_VAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimUserDefinedCalculation::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_destinationCase );

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Cell Filter" );
    filterGroup->setCollapsedByDefault();
    filterGroup->add( &m_cellFilterView );
    filterGroup->add( &m_defaultValueType );
    if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
        filterGroup->add( &m_defaultPropertyVariableIndex );
    else if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::USER_DEFINED )
        filterGroup->add( &m_defaultValue );

    uiOrdering.skipRemainingFields();

    // Update state
    if ( m_cellFilterView() )
    {
        m_defaultValueType.uiCapability()->setUiReadOnly( false );
        m_defaultValue.uiCapability()->setUiReadOnly( m_defaultValueType() !=
                                                      RimGridCalculation::DefaultValueType::USER_DEFINED );
    }
    else
    {
        m_defaultValueType.uiCapability()->setUiReadOnly( true );
        m_defaultValue.uiCapability()->setUiReadOnly( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCalculation::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_cellFilterView )
    {
        options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );

        std::vector<Rim3dView*> views;
        RimProject::current()->allViews( views );

        RimEclipseCase* firstEclipseCase = nullptr;
        if ( !inputCases().empty() ) firstEclipseCase = inputCases().front();

        if ( firstEclipseCase )
        {
            for ( auto* view : views )
            {
                auto eclipseView = dynamic_cast<RimEclipseView*>( view );
                if ( !eclipseView ) continue;
                if ( !firstEclipseCase->isGridSizeEqualTo( eclipseView->eclipseCase() ) ) continue;

                options.push_back( caf::PdmOptionItemInfo( view->autoName(), view, false, view->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_destinationCase )
    {
        if ( inputCases().empty() )
        {
            RimTools::eclipseCaseOptionItems( &options );
        }
        else
        {
            RimEclipseCase* firstInputCase = inputCases()[0];

            RimProject* proj = RimProject::current();
            if ( proj )
            {
                std::vector<RimCase*> cases;
                proj->allCases( cases );

                for ( RimCase* c : cases )
                {
                    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( c );
                    if ( !eclipseCase ) continue;
                    if ( !firstInputCase->isGridSizeEqualTo( eclipseCase ) ) continue;

                    options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
                }
            }
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_defaultPropertyVariableIndex )
    {
        for ( int i = 0; i < m_variables.size(); i++ )
        {
            auto v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

            QString optionText = v->name();
            options.push_back( caf::PdmOptionItemInfo( optionText, i ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::initAfterRead()
{
    for ( auto& variable : m_variables )
    {
        auto gridVar = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        if ( gridVar )
        {
            gridVar->eclipseResultChanged.connect( this, &RimGridCalculation::onVariableUpdated );

            if ( m_destinationCase == nullptr ) m_destinationCase = gridVar->eclipseCase();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::onVariableUpdated( const SignalEmitter* emitter )
{
    if ( m_destinationCase == nullptr )
    {
        auto variable = dynamic_cast<const RimGridCalculationVariable*>( emitter );
        if ( variable && variable->eclipseCase() )
        {
            m_destinationCase = variable->eclipseCase();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridCalculation::getInputVectorForVariable( RimGridCalculationVariable*   v,
                                                                   size_t                        tsId,
                                                                   RiaDefines::PorosityModelType porosityModel ) const
{
    int timeStep = v->timeStep();

    auto resultCategoryType = v->resultCategoryType();

    // General case is to use the data from the given time step
    size_t timeStepToUse = tsId;

    if ( resultCategoryType == RiaDefines::ResultCatType::STATIC_NATIVE )
    {
        // Use the first time step for static data for all time steps
        timeStepToUse = 0;
    }
    else if ( timeStep != RimGridCalculationVariable::allTimeStepsValue() )
    {
        // Use data from a specific time step for this variable for all result time steps
        timeStepToUse = timeStep;
    }

    RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );

    auto   mainGrid     = v->eclipseCase()->mainGrid();
    size_t maxGridCount = mainGrid->gridCount();

    size_t              cellCount = mainGrid->globalCellArray().size();
    std::vector<double> inputValues( cellCount );
    for ( size_t gridIdx = 0; gridIdx < maxGridCount; ++gridIdx )
    {
        auto grid = mainGrid->gridByIndex( gridIdx );

        cvf::ref<RigResultAccessor> sourceResultAccessor =
            RigResultAccessorFactory::createFromResultAddress( v->eclipseCase()->eclipseCaseData(),
                                                               gridIdx,
                                                               porosityModel,
                                                               timeStepToUse,
                                                               resAddr );

#pragma omp parallel for
        for ( int localGridCellIdx = 0; localGridCellIdx < static_cast<int>( grid->cellCount() ); localGridCellIdx++ )
        {
            const size_t reservoirCellIndex = grid->reservoirCellIndex( localGridCellIdx );
            inputValues[reservoirCellIndex] = sourceResultAccessor->cellScalar( localGridCellIdx );
        }
    }

    return inputValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithVector( const std::vector<double>& inputValues,
                                                          cvf::ref<cvf::UByteArray>  visibility,
                                                          std::vector<double>&       resultValues )
{
#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( resultValues.size() ); i++ )
    {
        if ( !visibility->val( i ) )
        {
            resultValues[i] = inputValues[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithDefaultValue( double                    defaultValue,
                                                                cvf::ref<cvf::UByteArray> visibility,
                                                                std::vector<double>&      resultValues )
{
#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( resultValues.size() ); i++ )
    {
        if ( !visibility->val( i ) )
        {
            resultValues[i] = defaultValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::filterResults( RimGridView*                            cellFilterView,
                                        const std::vector<std::vector<double>>& values,
                                        RimGridCalculation::DefaultValueType    defaultValueType,
                                        double                                  defaultValue,
                                        std::vector<double>&                    resultValues ) const
{
    auto visibility = cellFilterView->currentTotalCellVisibility();

    if ( defaultValueType == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
    {
        if ( m_defaultPropertyVariableIndex < values.size() )
            replaceFilteredValuesWithVector( values[m_defaultPropertyVariableIndex], visibility, resultValues );
        else
        {
            QString errorMessage =
                "Invalid input data for default result property, no data assigned to non-visible cells.";
            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        }
    }
    else
    {
        double valueToUse = defaultValue;
        if ( defaultValueType == RimGridCalculation::DefaultValueType::POSITIVE_INFINITY ) valueToUse = HUGE_VAL;

        replaceFilteredValuesWithDefaultValue( valueToUse, visibility, resultValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::updateDependentObjects()
{
    RimEclipseCase* eclipseCase = destinationEclipseCase();
    if ( eclipseCase )
    {
        RimReloadCaseTools::updateAll3dViews( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::removeDependentObjects()
{
    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    RimEclipseCase* eclipseCase = destinationEclipseCase();
    if ( eclipseCase )
    {
        // Select "None" result if the result that is being removed were displayed in a view.
        for ( auto v : eclipseCase->reservoirViews() )
        {
            if ( v->cellResult()->resultType() == resAddr.resultCatType() &&
                 v->cellResult()->resultVariable() == resAddr.resultName() )
            {
                v->cellResult()->setResultType( RiaDefines::ResultCatType::GENERATED );
                v->cellResult()->setResultVariable( "None" );
            }
        }

        eclipseCase->results( porosityModel )->clearScalarResult( resAddr );
        eclipseCase->results( porosityModel )->setRemovedTagOnGeneratedResult( resAddr );

        RimReloadCaseTools::updateAll3dViews( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RimGridCalculation::validateVariables()
{
    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->eclipseCase() )
        {
            QString errorMessage = QString( "No case defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        if ( v->resultVariable().isEmpty() )
        {
            QString errorMessage = QString( "No result variable defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        auto                    resultCategoryType = v->resultCategoryType();
        RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );
        if ( !v->eclipseCase()->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            QString errorMessage = QString( "Unable to load result for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }
    }

    return std::make_pair( true, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                                            std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_variables )
    {
        // Update the editors of all the variables if a variable changes.
        // This makes the read-only state of the filter parameters consistent:
        // only one filter is allowed at a time.
        for ( auto v : m_variables )
        {
            v->updateConnectedEditors();
        }
    }
}
