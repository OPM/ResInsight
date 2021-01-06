/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimSaturationPressurePlot.h"

#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEquilibriumAxisAnnotation.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimPlotAxisProperties.h"

#include "CellFilters/RimPlotCellPropertyFilter.h"

CAF_PDM_SOURCE_INIT( RimSaturationPressurePlot, "RimSaturationPressurePlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlot::RimSaturationPressurePlot()
{
    CAF_PDM_InitObject( "Saturation Pressure Plot", ":/SummaryXPlotLight16x16.png", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlot::assignCaseAndEquilibriumRegion( RiaDefines::PorosityModelType porosityModel,
                                                                RimEclipseResultCase*         eclipseResultCase,
                                                                int                           zeroBasedEquilRegionIndex,
                                                                int                           timeStep )
{
    CVF_ASSERT( eclipseResultCase && eclipseResultCase->eclipseCaseData() );

    auto equilData = eclipseResultCase->eclipseCaseData()->equilData();
    if ( zeroBasedEquilRegionIndex >= (int)equilData.size() )
    {
        RiaLogging::error( "Invalid equilibrium region index" );
        return;
    }

    setShowInfoBox( false );
    nameConfig()->addDataSetNames = false;

    QString caseName  = eclipseResultCase->caseUserDescription();
    QString plotTitle = QString( "%1 - EQLNUM %2" ).arg( caseName ).arg( zeroBasedEquilRegionIndex + 1 );

    nameConfig()->setCustomName( plotTitle );

    auto eq = equilData[zeroBasedEquilRegionIndex];

    double gasOilContactDepth   = eq.gasOilContactDepth();
    double waterOilContactDepth = eq.waterOilContactDepth();

    {
        // Blue PRESSURE curve with data for specified EQLNUM value

        RimGridCrossPlotDataSet* curveSet = createDataSet();
        curveSet->configureForPressureSaturationCurves( eclipseResultCase, "PRESSURE", timeStep );

        cvf::Color3f curveColor = RiaColorTables::summaryCurveBluePaletteColors().cycledColor3f( 0 );
        curveSet->setCustomColor( curveColor );

        RimPlotCellPropertyFilter* cellFilter =
            createEquilibriumRegionPropertyFilter( eclipseResultCase, zeroBasedEquilRegionIndex );

        curveSet->addCellFilter( cellFilter );
    }

    {
        // Red dew pressure (PDEW) curve with data for specified EQLNUM value, filtered on depth by gasOilContact

        RimGridCrossPlotDataSet* curveSet = createDataSet();
        curveSet->configureForPressureSaturationCurves( eclipseResultCase, "PDEW", timeStep );

        cvf::Color3f curveColor = RiaColorTables::summaryCurveRedPaletteColors().cycledColor3f( 0 );
        curveSet->setCustomColor( curveColor );

        RimPlotCellPropertyFilter* cellFilter =
            createEquilibriumRegionPropertyFilter( eclipseResultCase, zeroBasedEquilRegionIndex );
        curveSet->addCellFilter( cellFilter );

        {
            RigCaseCellResultsData* caseCellResultsData = eclipseResultCase->eclipseCaseData()->results( porosityModel );
            if ( caseCellResultsData )
            {
                RigEclipseResultAddress depthResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DEPTH" );

                double minDepth = 0.0;
                double maxDepth = 0.0;
                caseCellResultsData->minMaxCellScalarValues( depthResultAddress, minDepth, maxDepth );

                maxDepth = gasOilContactDepth;

                RimPlotCellPropertyFilter* depthCellFilter =
                    createDepthPropertyFilter( eclipseResultCase, minDepth, maxDepth );

                curveSet->addCellFilter( depthCellFilter );
            }
        }
    }

    {
        // Green bubble point pressure (PBUB) curve with data for specified EQLNUM value, filtered on depth between
        // gasOilContact and waterOilContactDepth

        RimGridCrossPlotDataSet* curveSet = createDataSet();
        curveSet->configureForPressureSaturationCurves( eclipseResultCase, "PBUB", timeStep );

        cvf::Color3f curveColor = RiaColorTables::summaryCurveGreenPaletteColors().cycledColor3f( 0 );
        curveSet->setCustomColor( curveColor );

        {
            RimPlotCellPropertyFilter* cellFilter = new RimPlotCellPropertyFilter();
            {
                RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
                resultDefinition->setEclipseCase( eclipseResultCase );
                resultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
                resultDefinition->setResultVariable( RiaDefines::eqlnumResultName() );

                cellFilter->setResultDefinition( resultDefinition );
            }

            cellFilter->setValueRange( zeroBasedEquilRegionIndex + 1, zeroBasedEquilRegionIndex + 1 );

            curveSet->addCellFilter( cellFilter );
        }

        {
            RigCaseCellResultsData* caseCellResultsData = eclipseResultCase->eclipseCaseData()->results( porosityModel );
            if ( caseCellResultsData )
            {
                RigEclipseResultAddress depthResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DEPTH" );

                double minDepth = 0.0;
                double maxDepth = 0.0;
                caseCellResultsData->minMaxCellScalarValues( depthResultAddress, minDepth, maxDepth );

                minDepth = gasOilContactDepth;
                maxDepth = waterOilContactDepth;

                RimPlotCellPropertyFilter* depthCellFilter =
                    createDepthPropertyFilter( eclipseResultCase, minDepth, maxDepth );

                curveSet->addCellFilter( depthCellFilter );
            }
        }
    }

    RimPlotAxisProperties* yAxisProps = yAxisProperties();
    yAxisProps->setInvertedAxis( true );

    {
        RimEquilibriumAxisAnnotation* annotation = new RimEquilibriumAxisAnnotation;
        annotation->setEquilibriumData( eclipseResultCase,
                                        zeroBasedEquilRegionIndex,
                                        RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_EQUIL_GAS_OIL_CONTACT );

        yAxisProps->appendAnnotation( annotation );
    }
    {
        RimEquilibriumAxisAnnotation* annotation = new RimEquilibriumAxisAnnotation;
        annotation->setEquilibriumData( eclipseResultCase,
                                        zeroBasedEquilRegionIndex,
                                        RimEquilibriumAxisAnnotation::PlotAxisAnnotationType::PL_EQUIL_WATER_OIL_CONTACT );

        yAxisProps->appendAnnotation( annotation );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlot::fixPointersAfterCopy( RimSaturationPressurePlot* source, RimSaturationPressurePlot* copy )
{
    CAF_ASSERT( source && copy );

    std::vector<RimPlotCellPropertyFilter*> sourceFilters;
    source->descendantsIncludingThisOfType( sourceFilters );

    std::vector<RimPlotCellPropertyFilter*> copyFilters;
    copy->descendantsIncludingThisOfType( copyFilters );

    if ( !sourceFilters.empty() && ( sourceFilters.size() == copyFilters.size() ) )
    {
        for ( size_t i = 0; i < sourceFilters.size(); i++ )
        {
            auto sourceFilter = sourceFilters[i];
            auto copyFilter   = copyFilters[i];

            sourceFilter->updatePointerAfterCopy( copyFilter );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlot::initAfterRead()
{
    yAxisProperties()->showAnnotationObjectsInProjectTree();

    RimGridCrossPlot::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSaturationPressurePlot::xAxisParameterString() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCellPropertyFilter*
    RimSaturationPressurePlot::createEquilibriumRegionPropertyFilter( RimEclipseResultCase* eclipseResultCase,
                                                                      int                   zeroBasedEquilRegionIndex )
{
    RimPlotCellPropertyFilter* cellFilter = new RimPlotCellPropertyFilter();

    RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
    resultDefinition->setEclipseCase( eclipseResultCase );
    resultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    resultDefinition->setResultVariable( RiaDefines::eqlnumResultName() );

    cellFilter->setResultDefinition( resultDefinition );

    cellFilter->setValueRange( zeroBasedEquilRegionIndex + 1, zeroBasedEquilRegionIndex + 1 );

    return cellFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCellPropertyFilter* RimSaturationPressurePlot::createDepthPropertyFilter( RimEclipseResultCase* eclipseResultCase,
                                                                                 double                minDepth,
                                                                                 double                maxDepth )
{
    RimPlotCellPropertyFilter* depthCellFilter = new RimPlotCellPropertyFilter();

    RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
    resultDefinition->setEclipseCase( eclipseResultCase );
    resultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    resultDefinition->setResultVariable( "DEPTH" );

    depthCellFilter->setResultDefinition( resultDefinition );

    depthCellFilter->setValueRange( minDepth, maxDepth );

    return depthCellFilter;
}
