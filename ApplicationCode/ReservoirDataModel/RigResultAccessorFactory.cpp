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

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createFromUiResultName(const RigEclipseCaseData* eclipseCase,
                                                                             size_t gridIndex,
                                                                             RiaDefines::PorosityModelType porosityModel,
                                                                             size_t timeStepIndex,
                                                                             const QString& uiResultName)
{
    cvf::ref<RigResultAccessor> derivedCandidate = createDerivedResultAccessor(eclipseCase,
                                                                               gridIndex,
                                                                               porosityModel,
                                                                               timeStepIndex,
                                                                               uiResultName);

    if (derivedCandidate.notNull()) return derivedCandidate;
    
    return RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, uiResultName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createFromNameAndType(const RigEclipseCaseData* eclipseCase,
                                                                            size_t gridIndex,
                                                                            RiaDefines::PorosityModelType porosityModel,
                                                                            size_t timeStepIndex,
                                                                            const QString& uiResultName,
                                                                            RiaDefines::ResultCatType resultType)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(resultType, uiResultName);
    if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
    {
        return NULL;
    }

    cvf::ref<RigResultAccessor> derivedCandidate = createDerivedResultAccessor(eclipseCase,
                                                                               gridIndex,
                                                                               porosityModel,
                                                                               timeStepIndex,
                                                                               uiResultName);

    if (derivedCandidate.notNull()) return derivedCandidate;

    size_t adjustedTimeStepIndex = timeStepIndex;
    if (resultType == RiaDefines::STATIC_NATIVE)
    {
        adjustedTimeStepIndex = 0;
    }

    return createFromResultIdx(eclipseCase, gridIndex, porosityModel, adjustedTimeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createFromResultDefinition(const RigEclipseCaseData* eclipseCase,
                                                                                 size_t gridIndex,
                                                                                 size_t timeStepIndex,
                                                                                 RimEclipseResultDefinition* resultDefinition)
{
    if (resultDefinition->isFlowDiagOrInjectionFlooding())
    {
        RimFlowDiagSolution* flowSol = resultDefinition->flowDiagSolution();
        if (!flowSol) return new RigHugeValResultAccessor;;

        const std::vector<double>* resultValues = flowSol->flowDiagResults()->resultValues( resultDefinition->flowDiagResAddress(), timeStepIndex);
        if (!resultValues) return new RigHugeValResultAccessor;

        const RigGridBase* grid = eclipseCase->grid(gridIndex);
        if ( !grid ) return new RigHugeValResultAccessor;

        cvf::ref<RigResultAccessor> object = new RigActiveCellsResultAccessor(grid, resultValues, eclipseCase->activeCellInfo(resultDefinition->porosityModel()));

        return object;
    }
    else
    {
        size_t adjustedTimeStepIndex = timeStepIndex;
        if ( resultDefinition->hasStaticResult() )
        {
            adjustedTimeStepIndex = 0;
        }

        return RigResultAccessorFactory::createFromNameAndType(eclipseCase,
                                                                gridIndex,
                                                                resultDefinition->porosityModel(),
                                                                adjustedTimeStepIndex,
                                                                resultDefinition->resultVariable(),
                                                                resultDefinition->resultType());
    }
}

//--------------------------------------------------------------------------------------------------
/// This function must be harmonized with RigResultModifierFactory::createResultModifier()
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createNativeFromUiResultName(const RigEclipseCaseData* eclipseCase,
                                                                                   size_t gridIndex,
                                                                                   RiaDefines::PorosityModelType porosityModel,
                                                                                   size_t timeStepIndex,
                                                                                   const QString& uiResultName)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(uiResultName);
    if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
    {
        return NULL;
    }

    return createFromResultIdx(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createDerivedResultAccessor(const RigEclipseCaseData* eclipseCase,
                                                                                  size_t gridIndex,
                                                                                  RiaDefines::PorosityModelType porosityModel,
                                                                                  size_t timeStepIndex,
                                                                                  const QString& uiResultName)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    const RigGridBase* grid = eclipseCase->grid(gridIndex);

    if (uiResultName == RiaDefines::combinedTransmissibilityResultName())
    {
        CVF_ASSERT(timeStepIndex == 0); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANX");
        cvf::ref<RigResultAccessor> yTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANY");
        cvf::ref<RigResultAccessor> zTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANZ");

        cellFaceAccessObject->setTransResultAccessors(xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedMultResultName())
    {
        CVF_ASSERT(timeStepIndex == 0); // Static result, only data for first time step

        cvf::ref<RigCombMultResultAccessor> cellFaceAccessObject = new RigCombMultResultAccessor(grid);

        cvf::ref<RigResultAccessor> multXPos = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTX");
        cvf::ref<RigResultAccessor> multXNeg = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTX-");
        cvf::ref<RigResultAccessor> multYPos = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTY");
        cvf::ref<RigResultAccessor> multYNeg = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTY-");
        cvf::ref<RigResultAccessor> multZPos = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTZ");
        cvf::ref<RigResultAccessor> multZNeg = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTZ-");

        cellFaceAccessObject->setMultResultAccessors(multXPos.p(), multXNeg.p(), multYPos.p(), multYNeg.p(), multZPos.p(), multZNeg.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedRiTranResultName())
    {
        CVF_ASSERT(timeStepIndex == 0); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riTranXResultName());
        cvf::ref<RigResultAccessor> yTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riTranYResultName());
        cvf::ref<RigResultAccessor> zTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riTranZResultName());

        cellFaceAccessObject->setTransResultAccessors(xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedRiMultResultName())
    {
        CVF_ASSERT(timeStepIndex == 0); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiMultAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riMultXResultName());
        cvf::ref<RigResultAccessor> yRiMultAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riMultYResultName());
        cvf::ref<RigResultAccessor> zRiMultAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riMultZResultName());

        cellFaceAccessObject->setTransResultAccessors(xRiMultAccessor.p(), yRiMultAccessor.p(), zRiMultAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedRiAreaNormTranResultName())
    {
        CVF_ASSERT(timeStepIndex == 0); // Static result, only data for first time step

        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riAreaNormTranXResultName());
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riAreaNormTranYResultName());
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, RiaDefines::riAreaNormTranZResultName());

        cellFaceAccessObject->setTransResultAccessors(xRiAreaNormTransAccessor.p(), yRiAreaNormTransAccessor.p(), zRiAreaNormTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedWaterFluxResultName())
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRWATI+");
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRWATJ+");
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRWATK+");

        cellFaceAccessObject->setTransResultAccessors(xRiAreaNormTransAccessor.p(), yRiAreaNormTransAccessor.p(), zRiAreaNormTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedOilFluxResultName())
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLROILI+");
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLROILJ+");
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLROILK+");

        cellFaceAccessObject->setTransResultAccessors(xRiAreaNormTransAccessor.p(), yRiAreaNormTransAccessor.p(), zRiAreaNormTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RiaDefines::combinedGasFluxResultName())
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRGASI+");
        cvf::ref<RigResultAccessor> yRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRGASJ+");
        cvf::ref<RigResultAccessor> zRiAreaNormTransAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, "FLRGASK+");

        cellFaceAccessObject->setTransResultAccessors(xRiAreaNormTransAccessor.p(), yRiAreaNormTransAccessor.p(), zRiAreaNormTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName.endsWith("IJK"))
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);
        QString baseName = uiResultName.left(uiResultName.size() - 3);

        cvf::ref<RigResultAccessor> iAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, QString("%1I").arg(baseName));
        cvf::ref<RigResultAccessor> jAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, QString("%1J").arg(baseName));
        cvf::ref<RigResultAccessor> kAccessor = RigResultAccessorFactory::createNativeFromUiResultName(eclipseCase, gridIndex, porosityModel, timeStepIndex, QString("%1K").arg(baseName));

        cellFaceAccessObject->setTransResultAccessors(iAccessor.p(), jAccessor.p(), kAccessor.p());

        return cellFaceAccessObject;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createFromResultIdx(const RigEclipseCaseData* eclipseCase,
                                                                          size_t gridIndex,
                                                                          RiaDefines::PorosityModelType porosityModel,
                                                                          size_t timeStepIndex,
                                                                          size_t resultIndex)
{
    if ( resultIndex == cvf::UNDEFINED_SIZE_T )
    {
        return new RigHugeValResultAccessor;
    }

    if (!eclipseCase) return NULL;

    const RigGridBase* grid = eclipseCase->grid(gridIndex);
    if (!grid) return NULL;

    const std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(resultIndex);

    if (timeStepIndex >= scalarSetResults.size())
    {
        return new RigHugeValResultAccessor;
    }

    const std::vector<double>* resultValues = NULL;
    if (timeStepIndex < scalarSetResults.size())
    {
        resultValues = &(scalarSetResults[timeStepIndex]);
    }

    if (!resultValues || resultValues->size() == 0)
    {
        return new RigHugeValResultAccessor;
    }

    bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(resultIndex);
    if (useGlobalActiveIndex)
    {
        cvf::ref<RigResultAccessor> object = new RigActiveCellsResultAccessor(grid, resultValues, eclipseCase->activeCellInfo(porosityModel));
        return object;
    }
    else
    {
        cvf::ref<RigResultAccessor> object = new RigAllGridCellsResultAccessor(grid, resultValues);
        return object;
    }
}

