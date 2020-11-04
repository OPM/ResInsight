/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimFractureModelCurve.h"

#include "RiaApplication.h"
#include "RiaFractureModelDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeOrdering.h"

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

    CAF_PDM_InitFieldNoDefault( &m_curveProperty, "CurveProperty", "Curve Property", "", "", "" );
    m_curveProperty.uiCapability()->setUiHidden( true );

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
    m_case          = fractureModel->eclipseCase();
    m_timeStep      = fractureModel->timeStep();
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setEclipseResultCategory( RiaDefines::ResultCatType catType )
{
    m_eclipseResultDefinition->setResultType( catType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setCurveProperty( RiaDefines::CurveProperty curveProperty )
{
    m_curveProperty = curveProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimFractureModelCurve::curveProperty() const
{
    return m_curveProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimWellLogExtractionCurve::fieldChangedByUi( changedField, oldValue, newValue );
    RimFractureModelPlot* fractureModelPlot;
    firstAncestorOrThisOfTypeAsserted( fractureModelPlot );

    if ( fractureModelPlot )
    {
        fractureModelPlot->loadDataAndUpdate();
    }
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

    if ( m_fractureModel && m_fractureModel->eclipseCase() )
    {
        RimEclipseCase* eclipseCase = m_fractureModel->eclipseCase();

        bool isOk = m_fractureModel->calculator()->extractCurveData( curveProperty(),
                                                                     m_fractureModel->timeStep(),
                                                                     values,
                                                                     measuredDepthValues,
                                                                     tvDepthValues,
                                                                     rkbDiff );
        if ( !isOk )
        {
            return;
        }

        RiaEclipseUnitTools::UnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        if ( eclipseUnitsType == RiaEclipseUnitTools::UnitSystem::UNITS_FIELD )
        {
            // See https://github.com/OPM/ResInsight/issues/538

            depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        }
    }

    bool performDataSmoothing = false;
    if ( !values.empty() && !measuredDepthValues.empty() && measuredDepthValues.size() == values.size() )
    {
        this->setValuesWithMdAndTVD( values, measuredDepthValues, tvDepthValues, rkbDiff, depthUnit, !performDataSmoothing, xUnits );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModelCurve::createCurveAutoName()
{
    return caf::AppEnum<RiaDefines::CurveProperty>::uiText( m_curveProperty() );
}
