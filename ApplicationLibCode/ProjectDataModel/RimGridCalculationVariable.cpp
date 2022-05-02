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
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimGridCalculationVariable, "RimGridCalculationVariable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::RimGridCalculationVariable()
{
    CAF_PDM_InitObject( "RimGridCalculationVariable", ":/octave.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultType, "ResultType", "Type" );
    CAF_PDM_InitField( &m_resultVariable, "ResultVariable", RiaResultNames::undefinedResultName(), "Variable" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseGridCase", "Grid Case" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    // if ( changedField == &m_button )
    // {
    //     bool updateContainingEditor = false;

    //     {
    //         RiuGridVectorSelectionDialog dlg( nullptr );
    //         dlg.hideEnsembles();

    //         readDataFromApplicationStore( &dlg );

    //         if ( dlg.exec() == QDialog::Accepted )
    //         {
    //             std::vector<RiaGridCurveDefinition> curveSelection = dlg.curveSelection();
    //             if ( curveSelection.size() > 0 )
    //             {
    //                 m_case = curveSelection[0].summaryCase();
    //                 m_summaryAddress->setAddress( curveSelection[0].summaryAddress() );

    //                 writeDataToApplicationStore();

    //                 updateContainingEditor = true;
    //             }
    //         }
    //     }

    //     if ( updateContainingEditor )
    //     {
    //         RimGridCalculation* rimCalculation = nullptr;
    //         this->firstAncestorOrThisOfTypeAsserted( rimCalculation );

    //         // RimCalculation is pointed to by RicGridCurveCalculator in a PtrField
    //         // Update editors connected to RicGridCurveCalculator
    //         std::vector<caf::PdmObjectHandle*> referringObjects;
    //         rimCalculation->objectsWithReferringPtrFields( referringObjects );
    //         for ( auto o : referringObjects )
    //         {
    //             o->uiCapability()->updateConnectedEditors();
    //         }
    //     }
    // }
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

    uiOrdering.skipRemainingFields();

    m_resultType.uiCapability()->setUiReadOnly( m_eclipseCase == nullptr );
    m_timeStep.uiCapability()->setUiReadOnly( m_resultType == RiaDefines::ResultCatType::STATIC_NATIVE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGridCalculationVariable::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
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
        RimTools::timeStepsForCase( m_eclipseCase(), &options );
    }

    if ( useOptionsOnly ) *useOptionsOnly = true;

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
