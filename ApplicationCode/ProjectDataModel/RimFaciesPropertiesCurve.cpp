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

#include "RimFaciesPropertiesCurve.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFaciesProperties.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFaciesProperties.h"
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
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

CAF_PDM_SOURCE_INIT( RimFaciesPropertiesCurve, "FaciesPropertiesCurve" );

namespace caf
{
template <>
void AppEnum<RimFaciesPropertiesCurve::PropertyType>::setUp()
{
    addItem( RimFaciesPropertiesCurve::PropertyType::YOUNGS_MODULUS, "YOUNGS_MODULUS", "Young's Modulus" );
    addItem( RimFaciesPropertiesCurve::PropertyType::POISSONS_RATIO, "POISSONS_RATIO", "Poisson's Ratio" );
    addItem( RimFaciesPropertiesCurve::PropertyType::K_IC, "K_IC", "K-Ic" );
    addItem( RimFaciesPropertiesCurve::PropertyType::PROPPANT_EMBEDMENT, "PROPPANT_EMBEDMENT", "Proppant Embedment" );
    setDefault( RimFaciesPropertiesCurve::PropertyType::YOUNGS_MODULUS );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesPropertiesCurve::RimFaciesPropertiesCurve()
{
    CAF_PDM_InitObject( "Fracture Model Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_propertyType, "PropertyType", "Property Type", "", "", "" );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesPropertiesCurve::~RimFaciesPropertiesCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesPropertiesCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesPropertiesCurve::setEclipseResultCategory( RiaDefines::ResultCatType catType )
{
    m_eclipseResultDefinition->setResultType( catType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesPropertiesCurve::setPropertyType( PropertyType propertyType )
{
    m_propertyType = propertyType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesPropertiesCurve::performDataExtraction( bool* isUsingPseudoLength )
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

        // Extract formation data
        cvf::ref<RigResultAccessor> formationResultAccessor = RigResultAccessorFactory::
            createFromResultAddress( eclipseCase->eclipseCaseData(),
                                     0,
                                     RiaDefines::PorosityModelType::MATRIX_MODEL,
                                     0,
                                     RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                              RiaDefines::activeFormationNamesResultName() ) );
        if ( !formationResultAccessor.notNull() )
        {
            std::cerr << "NO FORMATION RES ACCEESOR" << std::endl;
            return;
        }

        // std::vector<double> formationValues;
        // eclExtractor.curveData( formationResultAccessor.p(), &formationValues );

        CurveSamplingPointData curveData =
            RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

        std::vector<std::pair<double, double>> yValues;
        std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

        std::vector<QString> formationNamesToPlot;
        RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                formationNamesVector,
                                                RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH,
                                                &formationNamesToPlot,
                                                &yValues );

        // Extract facies data
        m_eclipseResultDefinition->setResultVariable( "OPERNUM_1" );
        m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );
        m_eclipseResultDefinition->setEclipseCase( eclipseCase );
        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> faciesResultAccessor =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                  0,
                                                                  m_timeStep,
                                                                  m_eclipseResultDefinition );

        if ( !faciesResultAccessor.notNull() )
        {
            std::cerr << "NO FACIES RES ACCESSOR" << std::endl;
            return;
        }

        std::vector<double> faciesValues;
        eclExtractor.curveData( faciesResultAccessor.p(), &faciesValues );

        // Extract porosity data
        //        m_eclipseResultDefinition->setResultType();
        cvf::ref<RigResultAccessor> poroResAcc =
            RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(),
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                        "PORO" ) );
        if ( !poroResAcc.notNull() )
        {
            std::cerr << "NO PORO RES ACCESSOR" << std::endl;
            return;
        }

        std::vector<double> poroValues;
        eclExtractor.curveData( poroResAcc.p(), &poroValues );

        std::cout << "Dims: FOrmation names to plot: " << formationNamesToPlot.size() << " Y: " << yValues.size()
                  << " FACIES: " << faciesValues.size() << " PORO: " << poroValues.size() << std::endl;

        for ( size_t i = 0; i < yValues.size(); i++ )
        {
            std::cout << i << ": " << yValues[i].first << "-" << yValues[i].second << " ==> "
                      << formationNamesToPlot[i].toStdString() << std::endl;
        }

        // TODO: make this settable??
        RimColorLegend* colorLegend = RimProject::current()->colorLegendCollection()->findByName( "Facies colors" );
        if ( !colorLegend )
        {
            std::cerr << "NO COLOR LEGEND" << std::endl;
            return;
        }

        RimFaciesProperties* faciesProperties = m_fractureModel->faciesProperties();
        if ( !faciesProperties )
        {
            std::cerr << "No facies properties" << std::endl;
            return;
        }

        for ( size_t i = 0; i < tvDepthValues.size(); i++ )
        {
            // TODO: get from somewhere??
            QString fieldName     = "Norne";
            QString faciesName    = findFaciesName( *colorLegend, faciesValues[i] );
            QString formationName = findFormationNameForDepth( formationNamesToPlot, yValues, tvDepthValues[i] );
            double  porosity      = poroValues[i];

            // std::cout << i << ": Depth: " << tvDepthValues[i] << " Poro: " << poroValues[i]
            //           << " Facies: " << faciesValues[i] << " name: " << faciesName.toStdString()
            //           << " formation: " << formationName.toStdString();

            FaciesKey faciesKey = std::make_tuple( fieldName, formationName, faciesName );
            if ( faciesProperties->hasPropertiesForFacies( faciesKey ) )
            {
                const RigFaciesProperties& rigFaciesProperties = faciesProperties->propertiesForFacies( faciesKey );

                if ( m_propertyType() == PropertyType::YOUNGS_MODULUS )
                {
                    double val = rigFaciesProperties.getYoungsModulus( porosity );
                    values.push_back( val );
                }
                else if ( m_propertyType() == PropertyType::POISSONS_RATIO )
                {
                    double val = rigFaciesProperties.getPoissonsRatio( porosity );
                    values.push_back( val );
                }
                else if ( m_propertyType() == PropertyType::K_IC )
                {
                    double val = rigFaciesProperties.getK_Ic( porosity );
                    values.push_back( val );
                }
                else if ( m_propertyType() == PropertyType::PROPPANT_EMBEDMENT )
                {
                    double val = rigFaciesProperties.getProppantEmbedment( porosity );
                    values.push_back( val );
                }
            }
            else
            {
                std::cerr << "  Missing key" << std::endl;
                return;
            }
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

QString RimFaciesPropertiesCurve::findFaciesName( const RimColorLegend& colorLegend, double value )
{
    for ( auto item : colorLegend.colorLegendItems() )
    {
        if ( item->categoryValue() == static_cast<int>( value ) ) return item->categoryName();
    }

    return "not found";
}

QString RimFaciesPropertiesCurve::findFormationNameForDepth( const std::vector<QString>& formationNames,
                                                             const std::vector<std::pair<double, double>>& depthRanges,
                                                             double                                        depth )
{
    //    assert(formationNames.size() == depthRanges.size());
    for ( size_t i = 0; i < formationNames.size(); i++ )
    {
        double high = depthRanges[i].second;
        double low  = depthRanges[i].first;
        if ( depth >= low && depth <= high )
        {
            return formationNames[i];
        }
    }

    return "not found";
}

QString RimFaciesPropertiesCurve::createCurveAutoName()
{
    return caf::AppEnum<RimFaciesPropertiesCurve::PropertyType>::uiText( m_propertyType() );
}
