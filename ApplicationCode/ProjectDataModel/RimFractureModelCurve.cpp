/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModelCurve.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimFractureModelCurve, "FractureModelCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCurve::RimFractureModelCurve()
{
    CAF_PDM_InitObject( "Fracture Model Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCurve::~RimFractureModelCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase )
    {
        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(),
                                                 m_fractureModel->thicknessDirectionWellPath()->wellPathGeometry(),
                                                 "fracture model" );

        measuredDepthValues = eclExtractor.cellIntersectionMDs();
        tvDepthValues       = eclExtractor.cellIntersectionTVDs();
        rkbDiff             = eclExtractor.wellPathData()->rkbDiff();

        // TODO: hard coded!
        m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
        if ( m_eclipseResultDefinition->resultVariable() == QString( "PRESSURE" ) )
            m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        else if ( m_eclipseResultDefinition->resultVariable() == QString( "OPERNUM_1" ) )
            m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );

        m_eclipseResultDefinition->setEclipseCase( eclipseCase );

        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> resAcc =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                  0,
                                                                  m_timeStep,
                                                                  m_eclipseResultDefinition );

        if ( resAcc.notNull() )
        {
            eclExtractor.curveData( resAcc.p(), &values );
        }
        else
        {
            std::cerr << "RESULT ACCESSOR IS NULL" << std::endl;
        }

        RiaEclipseUnitTools::UnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        if ( eclipseUnitsType == RiaEclipseUnitTools::UnitSystem::UNITS_FIELD )
        {
            // See https://github.com/OPM/ResInsight/issues/538

            depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        }
    }

    bool performDataSmoothing = false;
    if ( !values.empty() && !measuredDepthValues.empty() )
    {
        if ( tvDepthValues.empty() )
        {
            this->setValuesAndDepths( values,
                                      measuredDepthValues,
                                      RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                      0.0,
                                      depthUnit,
                                      !performDataSmoothing,
                                      xUnits );
        }
        else
        {
            this->setValuesWithMdAndTVD( values,
                                         measuredDepthValues,
                                         tvDepthValues,
                                         rkbDiff,
                                         depthUnit,
                                         !performDataSmoothing,
                                         xUnits );
        }
    }
}
