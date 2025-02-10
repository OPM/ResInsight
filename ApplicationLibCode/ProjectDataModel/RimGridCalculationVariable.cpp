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

#include "RigCaseCellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseResultAddress.h"
#include "RimGridCalculation.h"
#include "RimResultSelectionUi.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuDragDrop.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

CAF_PDM_SOURCE_INIT( RimGridCalculationVariable, "RimGridCalculationVariable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::RimGridCalculationVariable()
    : eclipseResultChanged( this )

{
    CAF_PDM_InitObject( "RimGridCalculationVariable", ":/octave.png" );

    CAF_PDM_InitField( &m_resultType, "ResultType", RiaDefines::ResultCatType::STATIC_NATIVE, "Type" );
    caf::AppEnum<RiaDefines::ResultCatType>::setEnumSubset( &m_resultType,
                                                            { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                              RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                              RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                              RiaDefines::ResultCatType::GENERATED } );

    CAF_PDM_InitField( &m_resultVariable, "ResultVariable", RiaResultNames::undefinedResultName(), "Variable" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseGridCase", "Grid Case" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", allTimeStepsValue(), "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_button, "PushButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_button );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCalculationVariable::displayString() const
{
    QStringList nameComponents;

    if ( m_eclipseCase() ) nameComponents.append( m_eclipseCase()->uiName() );

    nameComponents.append( m_resultVariable() );
    return nameComponents.join( " : " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
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
    uiOrdering.add( &m_button );

    uiOrdering.skipRemainingFields();

    m_resultType.uiCapability()->setUiReadOnly( m_eclipseCase == nullptr );
    m_timeStep.uiCapability()->setUiReadOnly( m_resultType == RiaDefines::ResultCatType::STATIC_NATIVE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto attr = dynamic_cast<caf::PdmUiTableViewPushButtonEditorAttribute*>( attribute );
    if ( attr )
    {
        attr->registerPushButtonTextForFieldKeyword( m_button.keyword(), "Edit" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_button )
    {
        auto eclipseCase = m_eclipseCase();
        if ( !eclipseCase )
        {
            auto cases = RimEclipseCaseTools::eclipseCases();
            if ( !cases.empty() )
            {
                eclipseCase = cases.front();
            }
        }

        RimResultSelectionUi selectionUi;
        selectionUi.setEclipseResultAddress( eclipseCase, m_resultType(), m_resultVariable );

        caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(), &selectionUi, "Select Result", "" );
        if ( propertyDialog.exec() == QDialog::Accepted )
        {
            setEclipseResultAddress( selectionUi.eclipseCase(), selectionUi.resultType(), selectionUi.resultVariable() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCalculationVariable::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultVariable )
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
    if ( !results ) return {};
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
void RimGridCalculationVariable::handleDroppedMimeData( const QMimeData* data, Qt::DropAction action, caf::PdmFieldHandle* destinationField )
{
    auto objects = RiuDragDrop::convertToObjects( data );
    if ( !objects.empty() )
    {
        if ( auto address = dynamic_cast<RimEclipseResultAddress*>( objects.front() ) )
        {
            setEclipseResultAddress( *address );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::setEclipseResultAddress( RimEclipseCase*                         eclipseCase,
                                                          caf::AppEnum<RiaDefines::ResultCatType> resultType,
                                                          const QString&                          resultName )
{
    m_eclipseCase    = eclipseCase;
    m_resultVariable = resultName;
    m_resultType     = resultType;

    eclipseResultChanged.send();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::setEclipseResultAddress( const RimEclipseResultAddress& resultAddress )
{
    setEclipseResultAddress( resultAddress.eclipseCase(), resultAddress.resultType(), resultAddress.resultName() );
}
