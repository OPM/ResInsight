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

#include "RimFractureModelStressCurve.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "RiaLogging.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimFractureModelStressCurve, "RimFractureModelStressCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelStressCurve::RimFractureModelStressCurve()
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
RimFractureModelStressCurve::~RimFractureModelStressCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelStressCurve::setCurveProperty( RiaDefines::CurveProperty curveProperty )
{
    m_curveProperty = curveProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimFractureModelStressCurve::curveProperty() const
{
    return m_curveProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelStressCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelStressCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    bool isOk = m_fractureModel->calculator()
                    ->extractCurveData( curveProperty(), m_fractureModel->timeStep(), values, measuredDepthValues, tvDepthValues, rkbDiff );
    if ( !isOk )
    {
        return;
    }

    RimEclipseCase* eclipseCase = m_fractureModel->eclipseCase();
    if ( eclipseCase && eclipseCase->eclipseCaseData()->unitsType() == RiaEclipseUnitTools::UnitSystem::UNITS_FIELD )
    {
        // See https://github.com/OPM/ResInsight/issues/538

        depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModelStressCurve::createCurveAutoName()
{
    return caf::AppEnum<RiaDefines::CurveProperty>::uiText( m_curveProperty() );
}
