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
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigStatisticsMath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimGridCalculationCollection.h"
#include "RimGridCalculationVariable.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimTools.h"

#include "expressionparser/ExpressionParser.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <QCheckBox>
#include <QMessageBox>

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
    CAF_PDM_InitField( &m_applyToAllCases, "AllDestinationCase", false, "Apply to All Cases" );
    CAF_PDM_InitField( &m_defaultPropertyVariableIndex, "DefaultPropertyVariableName", 0, "Property Variable Name" );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "SelectedTimeSteps", "Time Step Selection" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::preCalculate() const
{
    if ( RiaGuiApplication::isRunning() && m_applyToAllCases() )
    {
        const QString cacheKey = "GridCalculatorMessage";

        auto cacheValue = RiaApplication::instance()->cacheDataObject( cacheKey );
        if ( cacheValue.isValid() && cacheValue.canConvert<bool>() )
        {
            return cacheValue.toBool();
        }

        QCheckBox* cb = new QCheckBox( "Don't show this question again" );

        QMessageBox msgbox;
        msgbox.setWindowTitle( "Grid Property Calculator" );
        msgbox.setText( "Calculation will be executed on all grid model cases.\nDo you want to continue?\n" );
        msgbox.setIcon( QMessageBox::Icon::Question );
        msgbox.addButton( QMessageBox::Yes );
        msgbox.addButton( QMessageBox::No );
        msgbox.setDefaultButton( QMessageBox::Yes );
        msgbox.setCheckBox( cb );

        auto reply = msgbox.exec();

        bool returnValue = ( reply == QMessageBox::Yes );

        if ( cb->isChecked() && returnValue )
        {
            RiaApplication::instance()->setCacheDataObject( cacheKey, returnValue );
        }

        return returnValue;
    }

    return true;
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
    for ( auto calculationCase : outputEclipseCases() )
    {
        if ( !calculationCase ) continue;

        for ( auto inputCase : inputCases() )
        {
            if ( !calculationCase->isGridSizeEqualTo( inputCase ) )
            {
                QString msg = "Detected IJK mismatch between input cases and destination case. All grid "
                              "cases must have identical IJK sizes.";
                RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", msg );
                return false;
            }
        }
    }

    cvf::UByteArray* inputValueVisibilityFilter = nullptr;
    if ( m_cellFilterView() )
    {
        inputValueVisibilityFilter = m_cellFilterView()->currentTotalCellVisibility().p();
    }

    std::optional<std::vector<size_t>> timeSteps = std::nullopt;

    if ( !m_selectedTimeSteps().empty() )
    {
        std::vector<size_t> tmp;
        for ( auto t : m_selectedTimeSteps() )
        {
            tmp.push_back( static_cast<size_t>( t ) );
        }

        timeSteps = tmp;
    }

    bool evaluateDependentCalculations = true;
    return calculateForCases( outputEclipseCases(), inputValueVisibilityFilter, timeSteps, evaluateDependentCalculations );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimGridCalculation::outputEclipseCases() const
{
    if ( m_applyToAllCases )
    {
        // Find all Eclipse cases suitable for grid calculations. This includes all single grid cases and source cases in a grid case group.
        // Exclude the statistics cases, as it is not possible to use them in a grid calculations.
        //
        // Note that data read from file can be released from memory when statistics for a time step is calculated. See
        // RimEclipseStatisticsCaseEvaluator::evaluateForResults()

        return RimEclipseCaseTools::allEclipseGridCases();
    }

    return { m_destinationCase };
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

    uiOrdering.add( &m_applyToAllCases );
    if ( !allSourceCasesAreEqualToDestinationCase() )
    {
        m_applyToAllCases = false;
    }
    m_applyToAllCases.uiCapability()->setUiReadOnly( !allSourceCasesAreEqualToDestinationCase() );

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Cell Filter" );
    filterGroup->setCollapsedByDefault();
    filterGroup->add( &m_cellFilterView );

    if ( m_cellFilterView() != nullptr )
    {
        filterGroup->add( &m_defaultValueType );

        if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
            filterGroup->add( &m_defaultPropertyVariableIndex );
        else if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::USER_DEFINED )
            filterGroup->add( &m_defaultValue );
    }

    caf::PdmUiGroup* timeStepGroup = uiOrdering.addNewGroup( "Time Step Filter" );
    timeStepGroup->setCollapsedByDefault();
    timeStepGroup->add( &m_selectedTimeSteps );

    uiOrdering.skipRemainingFields();
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
        RimEclipseCase* firstInputCase = nullptr;

        if ( !inputCases().empty() )
        {
            firstInputCase = inputCases()[0];
        }

        for ( auto eclipseCase : RimEclipseCaseTools::allEclipseGridCases() )
        {
            if ( !eclipseCase ) continue;
            if ( firstInputCase && !firstInputCase->isGridSizeEqualTo( eclipseCase ) ) continue;

            options.push_back( caf::PdmOptionItemInfo( eclipseCase->caseUserDescription(), eclipseCase, false, eclipseCase->uiIconProvider() ) );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_defaultPropertyVariableIndex )
    {
        for ( int i = 0; i < static_cast<int>( m_variables.size() ); i++ )
        {
            auto v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

            QString optionText = v->name();
            options.push_back( caf::PdmOptionItemInfo( optionText, i ) );
        }
    }
    else if ( &m_selectedTimeSteps == fieldNeedingOptions )
    {
        RimEclipseCase* firstEclipseCase = nullptr;
        if ( !inputCases().empty() ) firstEclipseCase = inputCases().front();

        if ( firstEclipseCase )
        {
            const auto timeStepStrings = firstEclipseCase->timeStepStrings();

            int index = 0;
            for ( const auto& text : timeStepStrings )
            {
                options.push_back( caf::PdmOptionItemInfo( text, index++ ) );
            }
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
    if ( auto variable = dynamic_cast<const RimGridCalculationVariable*>( emitter ) )
    {
        if ( auto variableCase = variable->eclipseCase() )
        {
            if ( !m_destinationCase || !m_destinationCase->isGridSizeEqualTo( variableCase ) )
            {
                m_destinationCase = variableCase;
            }
        }
    }

    updateConnectedEditors();

    variableUpdated.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::allSourceCasesAreEqualToDestinationCase() const
{
    if ( m_destinationCase() == nullptr ) return false;

    for ( const auto& variable : m_variables )
    {
        auto gridVar = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        if ( gridVar )
        {
            if ( gridVar->eclipseCase() != m_destinationCase() ) return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RimGridCalculation::outputAddress() const
{
    QString                 leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );
    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );
    return resAddr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridCalculation::getDataForVariable( RimGridCalculationVariable*   variable,
                                                            size_t                        tsId,
                                                            RiaDefines::PorosityModelType porosityModel,
                                                            RimEclipseCase*               destinationCase,
                                                            bool                          useDataFromDestinationCase ) const
{
    // The data can be taken from the destination case or from the calculation variable.
    auto eclipseCase = useDataFromDestinationCase ? destinationCase : variable->eclipseCase();

    if ( !eclipseCase ) return {};

    int timeStep = variable->timeStep();

    auto resultCategoryType = variable->resultCategoryType();

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

    RigEclipseResultAddress resAddr( resultCategoryType, variable->resultVariable() );

    auto eclipseCaseData        = eclipseCase->eclipseCaseData();
    auto rigCaseCellResultsData = eclipseCaseData->results( porosityModel );
    if ( !rigCaseCellResultsData->ensureKnownResultLoaded( resAddr ) ) return {};

    // Active cell info must always be retrieved from the destination case, as the returned vector must be of the same size as number of
    // active cells in the destination case.
    auto activeCellInfoDestination = destinationCase->eclipseCaseData()->activeCellInfo( porosityModel );
    auto activeReservoirCells      = activeCellInfoDestination->activeReservoirCellIndices();

    std::vector<double> values( activeCellInfoDestination->activeReservoirCellIndices().size() );

    auto resultAccessor = RigResultAccessorFactory::createFromResultAddress( eclipseCaseData, 0, porosityModel, timeStepToUse, resAddr );

#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( activeReservoirCells.size() ); i++ )
    {
        values[i] = resultAccessor->cellScalarGlobIdx( activeReservoirCells[i] );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithVector( const std::vector<double>&    inputValues,
                                                          cvf::ref<cvf::UByteArray>     visibility,
                                                          std::vector<double>&          resultValues,
                                                          RiaDefines::PorosityModelType porosityModel,
                                                          RimEclipseCase*               outputEclipseCase )

{
    auto activeCellInfo = outputEclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
    int  numCells       = static_cast<int>( visibility->size() );

#pragma omp parallel for
    for ( int i = 0; i < numCells; i++ )
    {
        if ( !visibility->val( i ) && activeCellInfo->isActive( i ) )
        {
            size_t cellResultIndex        = activeCellInfo->cellResultIndex( i );
            resultValues[cellResultIndex] = inputValues[cellResultIndex];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithDefaultValue( double                        defaultValue,
                                                                cvf::ref<cvf::UByteArray>     visibility,
                                                                std::vector<double>&          resultValues,
                                                                RiaDefines::PorosityModelType porosityModel,
                                                                RimEclipseCase*               outputEclipseCase )

{
    auto activeCellInfo = outputEclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
    int  numCells       = static_cast<int>( visibility->size() );

#pragma omp parallel for
    for ( int i = 0; i < numCells; i++ )
    {
        if ( !visibility->val( i ) && activeCellInfo->isActive( i ) )
        {
            size_t cellResultIndex        = activeCellInfo->cellResultIndex( i );
            resultValues[cellResultIndex] = defaultValue;
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
                                        std::vector<double>&                    resultValues,
                                        RiaDefines::PorosityModelType           porosityModel,
                                        RimEclipseCase*                         outputEclipseCase ) const

{
    auto visibility = cellFilterView->currentTotalCellVisibility();

    if ( defaultValueType == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
    {
        if ( m_defaultPropertyVariableIndex < static_cast<int>( values.size() ) )
            replaceFilteredValuesWithVector( values[m_defaultPropertyVariableIndex], visibility, resultValues, porosityModel, outputEclipseCase );
        else
        {
            QString errorMessage = "Invalid input data for default result property, no data assigned to non-visible cells.";
            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        }
    }
    else
    {
        double valueToUse = defaultValue;
        if ( defaultValueType == RimGridCalculation::DefaultValueType::POSITIVE_INFINITY ) valueToUse = HUGE_VAL;

        replaceFilteredValuesWithDefaultValue( valueToUse, visibility, resultValues, porosityModel, outputEclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::updateDependentObjects()
{
    for ( RimEclipseCase* eclipseCase : outputEclipseCases() )
    {
        if ( eclipseCase )
        {
            RimReloadCaseTools::updateAll3dViews( eclipseCase );
        }
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

    for ( RimEclipseCase* eclipseCase : outputEclipseCases() )
    {
        if ( eclipseCase )
        {
            // Select "None" result if the result that is being removed were displayed in a view.
            for ( auto v : eclipseCase->reservoirViews() )
            {
                if ( v->cellResult()->resultType() == resAddr.resultCatType() && v->cellResult()->resultVariable() == resAddr.resultName() )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::calculateForCases( const std::vector<RimEclipseCase*>& calculationCases,
                                            cvf::UByteArray*                    inputValueVisibilityFilter,
                                            std::optional<std::vector<size_t>>  timeSteps,
                                            bool                                evaluateDependentCalculations )
{
    if ( calculationCases.empty() ) return true;

    if ( evaluateDependentCalculations ) findAndEvaluateDependentCalculations( calculationCases, inputValueVisibilityFilter, timeSteps );

    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    auto [isOk, errorMessage] = validateVariables();
    if ( !isOk )
    {
        RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        return false;
    }

    const bool isMultipleCasesPresent   = calculationCases.size() > 1;
    const bool hasAggregationExpression = m_expression().contains( "sum" ) || m_expression().contains( "avg" ) ||
                                          m_expression().contains( "min" ) || m_expression().contains( "max" ) ||
                                          m_expression().contains( "count" );

    if ( isMultipleCasesPresent )
    {
        QString txt = "Starting calculation '" + description() + "' for " + QString::number( calculationCases.size() ) + " cases.";
        RiaLogging::info( txt );

        if ( hasAggregationExpression )
        {
            RiaLogging::info( QString( "  Detected aggregated value in expression '%1'." ).arg( m_expression() ) );
            RiaLogging::info( QString( "  Aggregated value per realization is displayed in one column per time step." ) );
        }
    }

    std::vector<std::vector<double>> allAggregatedValues;

    bool anyErrorsDetected = false;
    for ( RimEclipseCase* calculationCase : calculationCases )
    {
        if ( !calculationCase )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Grid Property Calculator",
                                           QString( "No case found for calculation : %1" ).arg( leftHandSideVariableName ) );
            anyErrorsDetected = true;
            continue;
        }

        auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

        if ( !calculationCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            bool needsToBeStored = false;
            calculationCase->results( porosityModel )->createResultEntry( resAddr, needsToBeStored );
        }

        calculationCase->results( porosityModel )->clearScalarResult( resAddr );

        // If an input grid is present, max time step count is zero. Make sure the time step count for the calculation is
        // always 1 or more.
        const size_t timeStepCount = std::max( size_t( 1 ), calculationCase->results( porosityModel )->maxTimeStepCount() );

        if ( calculationCase == calculationCases.front() && hasAggregationExpression )
        {
            // Print time header

            auto timeStepStrings = calculationCase->timeStepStrings();

            QString timeHeader;
            for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
            {
                // Skip time steps that are not in the list of time steps to calculate
                if ( timeSteps && std::find( timeSteps->begin(), timeSteps->end(), tsId ) == timeSteps->end() ) continue;

                timeHeader += "\t";
                auto index = static_cast<int>( tsId );

                if ( index < timeStepStrings.size() )
                {
                    timeHeader += timeStepStrings.at( static_cast<int>( tsId ) );
                }
                else
                    timeHeader += "Undefined";
            }
            RiaLogging::info( timeHeader );
        }

        std::vector<std::vector<double>>* scalarResultFrames =
            calculationCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
        scalarResultFrames->resize( timeStepCount );

        std::vector<double> aggregatedValuesOneTimeStep;

        for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
        {
            // Skip time steps that are not in the list of time steps to calculate
            if ( timeSteps && std::find( timeSteps->begin(), timeSteps->end(), tsId ) == timeSteps->end() ) continue;

            std::vector<std::vector<double>> dataForAllVariables;
            for ( size_t i = 0; i < m_variables.size(); i++ )
            {
                RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
                CAF_ASSERT( v != nullptr );

                bool useDataFromDestinationCase = ( v->eclipseCase() == m_destinationCase );

                auto dataForVariable = getDataForVariable( v, tsId, porosityModel, calculationCase, useDataFromDestinationCase );
                if ( dataForVariable.empty() )
                {
                    RiaLogging::error( QString( "  No data found for variable '%1'." ).arg( v->name() ) );
                }
                else if ( inputValueVisibilityFilter && hasAggregationExpression )
                {
                    const double defaultValue = 0.0;
                    replaceFilteredValuesWithDefaultValue( defaultValue, inputValueVisibilityFilter, dataForVariable, porosityModel, calculationCase );
                }

                dataForAllVariables.push_back( dataForVariable );
            }

            ExpressionParser parser;
            for ( size_t i = 0; i < m_variables.size(); i++ )
            {
                RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
                CAF_ASSERT( v != nullptr );
                parser.assignVector( v->name(), dataForAllVariables[i] );
            }

            std::vector<double> resultValues;
            resultValues.resize( dataForAllVariables[0].size() );
            parser.assignVector( leftHandSideVariableName, resultValues );

            QString errorText;
            bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

            if ( evaluatedOk )
            {
                if ( hasAggregationExpression )
                {
                    auto it =
                        std::find_if( resultValues.begin(), resultValues.end(), []( double v ) { return ( !std::isnan( v ) && v != 0.0 ); } );
                    if ( it != resultValues.end() )
                    {
                        aggregatedValuesOneTimeStep.push_back( *it );
                    }
                    else
                    {
                        aggregatedValuesOneTimeStep.push_back( 0.0 );
                    }
                }

                if ( m_cellFilterView() && !resultValues.empty() )
                {
                    filterResults( m_cellFilterView(),
                                   dataForAllVariables,
                                   m_defaultValueType(),
                                   m_defaultValue(),
                                   resultValues,
                                   porosityModel,
                                   calculationCase );
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

            calculationCase->updateResultAddressCollection();
        }

        if ( hasAggregationExpression )
        {
            QString txt = "    " + calculationCase->caseUserDescription();

            for ( auto v : aggregatedValuesOneTimeStep )
            {
                txt += "\t" + QString::number( v );
            }

            allAggregatedValues.push_back( aggregatedValuesOneTimeStep );

            RiaLogging::info( txt );
        }
    }

    if ( isMultipleCasesPresent )
    {
        auto [anyError, statisticsText] = createStatisticsText( allAggregatedValues );
        if ( anyError )
        {
            anyErrorsDetected = true;
        }
        else if ( !statisticsText.empty() )
        {
            RiaLogging::info( "  Statistics" );
            for ( const auto& txt : statisticsText )
            {
                RiaLogging::info( txt );
            }
        }

        QString txt = "Completed calculation '" + description() + "' for " + QString::number( calculationCases.size() ) + " cases.";
        RiaLogging::info( txt );
    }

    return !anyErrorsDetected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::findAndEvaluateDependentCalculations( const std::vector<RimEclipseCase*>& calculationCases,
                                                               cvf::UByteArray*                    inputValueVisibilityFilter,
                                                               std::optional<std::vector<size_t>>  timeSteps )
{
    auto proj     = RimProject::current();
    auto calcColl = proj->gridCalculationCollection();

    auto dependentCalculations = calcColl->dependentCalculations( this );
    for ( auto dependentCalc : dependentCalculations )
    {
        if ( dependentCalc == this ) continue;

        // Propagate the settings for this calculation to the dependent calculation. This will allow changes on top level calculation to be
        // propagated to dependent calculations automatically. Do not trigger findAndEvaluateDependentCalculations() recursively, as all
        // dependent calculations are traversed in this function.

        bool evaluateDependentCalculations = false;
        dependentCalc->calculateForCases( calculationCases, inputValueVisibilityFilter, timeSteps, evaluateDependentCalculations );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::assignEclipseCaseForNullPointers( RimEclipseCase* eclipseCase )
{
    if ( m_destinationCase() == nullptr )
    {
        m_destinationCase = eclipseCase;
    }

    for ( auto v : m_variables )
    {
        if ( auto gridVar = dynamic_cast<RimGridCalculationVariable*>( v.p() ) )
        {
            if ( gridVar->eclipseCase() == nullptr )
            {
                gridVar->setEclipseCase( eclipseCase );
            }
        }
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
void RimGridCalculation::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QStringList> RimGridCalculation::createStatisticsText( const std::vector<std::vector<double>>& values )
{
    bool        anyErrorsDetected = false;
    QStringList statisticsText;

    if ( values.empty() ) return { anyErrorsDetected, statisticsText };

    size_t size = values.front().size();
    for ( const auto& aggregatedVector : values )
    {
        if ( aggregatedVector.size() != size )
        {
            QString msg = "Detected mismatch in number of time steps for aggregated values. All grid "
                          "cases must have identical number of time steps.";
            RiaLogging::error( msg );
            anyErrorsDetected = true;
            return { anyErrorsDetected, statisticsText };
        }
    }

    std::vector<double> minValues;
    std::vector<double> p90Values;
    std::vector<double> p50Values;
    std::vector<double> avgValues;
    std::vector<double> p10Values;
    std::vector<double> maxValues;

    for ( size_t timeIdx = 0; timeIdx < values.front().size(); timeIdx++ )
    {
        std::vector<double> valuesForTimeStep;

        for ( auto v : values )
        {
            valuesForTimeStep.push_back( v[timeIdx] );
        }

        // Required to have minimum 8 values to calculate statistics
        if ( valuesForTimeStep.size() < 8 ) continue;

        double p90 = std::numeric_limits<double>::infinity();
        double p50 = std::numeric_limits<double>::infinity();
        double avg = std::numeric_limits<double>::infinity();
        double p10 = std::numeric_limits<double>::infinity();

        auto [min, max] = std::minmax_element( valuesForTimeStep.begin(), valuesForTimeStep.end() );

        RigStatisticsMath::calculateStatisticsCurves( valuesForTimeStep, &p10, &p50, &p90, &avg, RigStatisticsMath::PercentileStyle::SWITCHED );

        minValues.push_back( *min );
        p90Values.push_back( p90 );
        p50Values.push_back( p50 );
        avgValues.push_back( avg );
        p10Values.push_back( p10 );
        maxValues.push_back( *max );
    }

    if ( !minValues.empty() )
    {
        QString minTxt = "  Minimum";
        QString p90Txt = "  P90    ";
        QString p50Txt = "  P50    ";
        QString avgTxt = "  Mean   ";
        QString p10Txt = "  P10    ";
        QString maxTxt = "  Maximum";

        for ( size_t timeIdx = 0; timeIdx < p10Values.size(); timeIdx++ )
        {
            minTxt += "\t" + QString::number( minValues[timeIdx] );
            p90Txt += "\t" + QString::number( p90Values[timeIdx] );
            p50Txt += "\t" + QString::number( p50Values[timeIdx] );
            avgTxt += "\t" + QString::number( avgValues[timeIdx] );
            p10Txt += "\t" + QString::number( p10Values[timeIdx] );
            maxTxt += "\t" + QString::number( maxValues[timeIdx] );
        }

        statisticsText.push_back( minTxt );
        statisticsText.push_back( p90Txt );
        statisticsText.push_back( p50Txt );
        statisticsText.push_back( avgTxt );
        statisticsText.push_back( p10Txt );
        statisticsText.push_back( maxTxt );
    }

    return { anyErrorsDetected, statisticsText };
}
