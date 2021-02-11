/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigResultAccessorFactory.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigCombMultResultAccessor.h"
#include "RigCombTransResultAccessor.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"

#include "RimEclipseResultDefinition.h"
#include "RimFlowDiagSolution.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RigResultAccessorFactory::createFromResultDefinition( const RigEclipseCaseData*         eclipseCase,
                                                          size_t                            gridIndex,
                                                          size_t                            timeStepIndex,
                                                          const RimEclipseResultDefinition* resultDefinition )
{
    if ( resultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        RimFlowDiagSolution* flowSol = resultDefinition->flowDiagSolution();
        if ( !flowSol ) return new RigHugeValResultAccessor;
        ;

        const std::vector<double>* resultValues =
            flowSol->flowDiagResults()->resultValues( resultDefinition->flowDiagResAddress(), timeStepIndex );
        if ( !resultValues ) return new RigHugeValResultAccessor;

        const RigGridBase* grid = eclipseCase->grid( gridIndex );
        if ( !grid ) return new RigHugeValResultAccessor;

        cvf::ref<RigResultAccessor> object =
            new RigActiveCellsResultAccessor( grid,
                                              resultValues,
                                              eclipseCase->activeCellInfo( resultDefinition->porosityModel() ) );

        return object;
    }
    else
    {
        return RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                  gridIndex,
                                                                  resultDefinition->porosityModel(),
                                                                  timeStepIndex,
                                                                  resultDefinition->eclipseResultAddress() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createFromResultAddress( const RigEclipseCaseData* eclipseCase,
                                                                               size_t                    gridIndex,
                                                                               RiaDefines::PorosityModelType porosityModel,
                                                                               size_t timeStepIndex,
                                                                               const RigEclipseResultAddress& resVarAddr )
{
    if ( !eclipseCase || !eclipseCase->results( porosityModel ) || !eclipseCase->activeCellInfo( porosityModel ) )
    {
        return nullptr;
    }

    if ( !eclipseCase->results( porosityModel )->hasResultEntry( resVarAddr ) )
    {
        return nullptr;
    }

    size_t adjustedTimeStepIndex = timeStepIndex;
    if ( resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE ||
         resVarAddr.resultCatType() == RiaDefines::ResultCatType::FORMATION_NAMES )
    {
        adjustedTimeStepIndex = 0;
    }

    cvf::ref<RigResultAccessor> derivedCandidate =
        createCombinedResultAccessor( eclipseCase, gridIndex, porosityModel, adjustedTimeStepIndex, resVarAddr );

    if ( derivedCandidate.notNull() ) return derivedCandidate;

    return createNativeFromResultAddress( eclipseCase, gridIndex, porosityModel, adjustedTimeStepIndex, resVarAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RigResultAccessorFactory::createCombinedResultAccessor( const RigEclipseCaseData*      eclipseCase,
                                                            size_t                         gridIndex,
                                                            RiaDefines::PorosityModelType  porosityModel,
                                                            size_t                         timeStepIndex,
                                                            const RigEclipseResultAddress& resVarAddr )
{
    CVF_ASSERT( gridIndex < eclipseCase->gridCount() );
    CVF_ASSERT( eclipseCase );
    CVF_ASSERT( eclipseCase->results( porosityModel ) );
    CVF_ASSERT( eclipseCase->activeCellInfo( porosityModel ) );

    RigEclipseResultAddress nativeAddr( resVarAddr );
    const RigGridBase*      grid = eclipseCase->grid( gridIndex );

    if ( resVarAddr.resultName() == RiaResultNames::combinedTransmissibilityResultName() )
    {
        CVF_ASSERT( timeStepIndex == 0 ); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );
        nativeAddr.setResultName( "TRANX" );
        cvf::ref<RigResultAccessor> xTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "TRANY" );
        cvf::ref<RigResultAccessor> yTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "TRANZ" );
        cvf::ref<RigResultAccessor> zTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName() == RiaResultNames::combinedMultResultName() )
    {
        CVF_ASSERT( timeStepIndex == 0 ); // Static result, only data for first time step

        cvf::ref<RigCombMultResultAccessor> cellFaceAccessObject = new RigCombMultResultAccessor( grid );

        nativeAddr.setResultName( "MULTX" );
        cvf::ref<RigResultAccessor> multXPos = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );
        nativeAddr.setResultName( "MULTX-" );
        cvf::ref<RigResultAccessor> multXNeg = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );
        nativeAddr.setResultName( "MULTY" );
        cvf::ref<RigResultAccessor> multYPos = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );
        nativeAddr.setResultName( "MULTY-" );
        cvf::ref<RigResultAccessor> multYNeg = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );
        nativeAddr.setResultName( "MULTZ" );
        cvf::ref<RigResultAccessor> multZPos = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );
        nativeAddr.setResultName( "MULTZ-" );
        cvf::ref<RigResultAccessor> multZNeg = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                        gridIndex,
                                                                                                        porosityModel,
                                                                                                        timeStepIndex,
                                                                                                        nativeAddr );

        cellFaceAccessObject
            ->setMultResultAccessors( multXPos.p(), multXNeg.p(), multYPos.p(), multYNeg.p(), multZPos.p(), multZNeg.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName() == RiaResultNames::combinedRiTranResultName() )
    {
        CVF_ASSERT( timeStepIndex == 0 ); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );

        nativeAddr.setResultName( RiaResultNames::riTranXResultName() );
        cvf::ref<RigResultAccessor> xTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riTranYResultName() );
        cvf::ref<RigResultAccessor> yTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riTranZResultName() );
        cvf::ref<RigResultAccessor> zTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName() == RiaResultNames::combinedRiMultResultName() )
    {
        CVF_ASSERT( timeStepIndex == 0 ); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );
        nativeAddr.setResultName( RiaResultNames::riMultXResultName() );
        cvf::ref<RigResultAccessor> xRiMultAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riMultYResultName() );
        cvf::ref<RigResultAccessor> yRiMultAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riMultZResultName() );
        cvf::ref<RigResultAccessor> zRiMultAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xRiMultAccessor.p(), yRiMultAccessor.p(), zRiMultAccessor.p() );

        return cellFaceAccessObject;
    }

    else if ( resVarAddr.resultName() == RiaResultNames::combinedRiAreaNormTranResultName() )
    {
        CVF_ASSERT( timeStepIndex == 0 ); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );

        nativeAddr.setResultName( RiaResultNames::riAreaNormTranXResultName() );
        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riAreaNormTranYResultName() );
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( RiaResultNames::riAreaNormTranZResultName() );
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xRiAreaNormTransAccessor.p(),
                                                       yRiAreaNormTransAccessor.p(),
                                                       zRiAreaNormTransAccessor.p() );

        return cellFaceAccessObject;
    }

    else if ( resVarAddr.resultName() == RiaResultNames::combinedWaterFluxResultName() )
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );

        nativeAddr.setResultName( "FLRWATI+" );
        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLRWATJ+" );
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLRWATK+" );
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xRiAreaNormTransAccessor.p(),
                                                       yRiAreaNormTransAccessor.p(),
                                                       zRiAreaNormTransAccessor.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName() == RiaResultNames::combinedOilFluxResultName() )
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );

        nativeAddr.setResultName( "FLROILI+" );
        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLROILJ+" );
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLROILK+" );
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xRiAreaNormTransAccessor.p(),
                                                       yRiAreaNormTransAccessor.p(),
                                                       zRiAreaNormTransAccessor.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName() == RiaResultNames::combinedGasFluxResultName() )
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );

        nativeAddr.setResultName( "FLRGASI+" );
        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLRGASJ+" );
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );
        nativeAddr.setResultName( "FLRGASK+" );
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor =
            RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                     gridIndex,
                                                                     porosityModel,
                                                                     timeStepIndex,
                                                                     nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( xRiAreaNormTransAccessor.p(),
                                                       yRiAreaNormTransAccessor.p(),
                                                       zRiAreaNormTransAccessor.p() );

        return cellFaceAccessObject;
    }
    else if ( resVarAddr.resultName().endsWith( "IJK" ) )
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor( grid );
        QString baseName = resVarAddr.resultName().left( resVarAddr.resultName().size() - 3 );

        nativeAddr.setResultName( QString( "%1I" ).arg( baseName ) );
        cvf::ref<RigResultAccessor> iAccessor = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                         gridIndex,
                                                                                                         porosityModel,
                                                                                                         timeStepIndex,
                                                                                                         nativeAddr );
        nativeAddr.setResultName( QString( "%1J" ).arg( baseName ) );
        cvf::ref<RigResultAccessor> jAccessor = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                         gridIndex,
                                                                                                         porosityModel,
                                                                                                         timeStepIndex,
                                                                                                         nativeAddr );
        nativeAddr.setResultName( QString( "%1K" ).arg( baseName ) );
        cvf::ref<RigResultAccessor> kAccessor = RigResultAccessorFactory::createNativeFromResultAddress( eclipseCase,
                                                                                                         gridIndex,
                                                                                                         porosityModel,
                                                                                                         timeStepIndex,
                                                                                                         nativeAddr );

        cellFaceAccessObject->setTransResultAccessors( iAccessor.p(), jAccessor.p(), kAccessor.p() );

        return cellFaceAccessObject;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RigResultAccessorFactory::createNativeFromResultAddress( const RigEclipseCaseData*      eclipseCase,
                                                             size_t                         gridIndex,
                                                             RiaDefines::PorosityModelType  porosityModel,
                                                             size_t                         timeStepIndex,
                                                             const RigEclipseResultAddress& resultAddress )
{
    if ( !eclipseCase || !eclipseCase->results( porosityModel ) || !eclipseCase->activeCellInfo( porosityModel ) )
    {
        return nullptr;
    }

    if ( !eclipseCase->results( porosityModel )->hasResultEntry( resultAddress ) )
    {
        return nullptr;
    }

    if ( !resultAddress.isValid() )
    {
        return nullptr;
    }

    const RigGridBase* grid = eclipseCase->grid( gridIndex );
    if ( !grid )
    {
        return nullptr;
    }

    const std::vector<std::vector<double>>& scalarSetResults =
        eclipseCase->results( porosityModel )->cellScalarResults( resultAddress );

    if ( timeStepIndex >= scalarSetResults.size() )
    {
        return new RigHugeValResultAccessor;
    }

    const std::vector<double>* resultValues = nullptr;
    if ( timeStepIndex < scalarSetResults.size() )
    {
        resultValues = &( scalarSetResults[timeStepIndex] );
    }

    if ( !resultValues || resultValues->size() == 0 )
    {
        return new RigHugeValResultAccessor;
    }

    bool useGlobalActiveIndex = eclipseCase->results( porosityModel )->isUsingGlobalActiveIndex( resultAddress );
    if ( useGlobalActiveIndex )
    {
        cvf::ref<RigResultAccessor> object =
            new RigActiveCellsResultAccessor( grid, resultValues, eclipseCase->activeCellInfo( porosityModel ) );
        return object;
    }
    else
    {
        cvf::ref<RigResultAccessor> object = new RigAllGridCellsResultAccessor( grid, resultValues );
        return object;
    }
}
