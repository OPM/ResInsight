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
#include "RimElementVectorResult.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "RiuViewer.h"

#include "cafAppEnum.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimElementVectorResult, "RimElementVectorResult" );

namespace caf
{
template <>
void AppEnum<RimElementVectorResult::TensorColors>::setUp()
{
    addItem( RimElementVectorResult::TensorColors::RESULT_COLORS, "RESULT_COLORS", "Result Colors" );
    addItem( RimElementVectorResult::TensorColors::UNIFORM_COLOR, "UNIFORM_COLOR", "Uniform" );

    setDefault( RimElementVectorResult::TensorColors::RESULT_COLORS );
}

template <>
void AppEnum<RimElementVectorResult::ScaleMethod>::setUp()
{
    addItem( RimElementVectorResult::ScaleMethod::RESULT, "RESULT", "Result" );
    addItem( RimElementVectorResult::ScaleMethod::RESULT_LOG, "RESULT_LOG", "Result (logarithmic scaling)" );
    addItem( RimElementVectorResult::ScaleMethod::CONSTANT, "CONSTANT", "Constant" );

    setDefault( RimElementVectorResult::ScaleMethod::RESULT );
}

template <>
void AppEnum<RimElementVectorResult::VectorView>::setUp()
{
    addItem( RimElementVectorResult::VectorView::CELL_CENTER_TOTAL, "AGGREGATED", "Cell Center Total" );
    addItem( RimElementVectorResult::VectorView::PER_FACE, "INDIVIDUAL", "Per Face" );

    setDefault( RimElementVectorResult::VectorView::CELL_CENTER_TOTAL );
}

template <>
void AppEnum<RimElementVectorResult::VectorSurfaceCrossingLocation>::setUp()
{
    addItem( RimElementVectorResult::VectorSurfaceCrossingLocation::VECTOR_ANCHOR, "VECTOR_ANCHOR", "At vector anchor" );
    addItem( RimElementVectorResult::VectorSurfaceCrossingLocation::VECTOR_CENTER, "VECTOR_CENTER", "At vector center" );

    setDefault( RimElementVectorResult::VectorSurfaceCrossingLocation::VECTOR_ANCHOR );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::RimElementVectorResult()
{
    CAF_PDM_InitObject( "Flow Vector Result", ":/CellResult.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showOil, "ShowOil", true, "Oil", "", "", "" );
    CAF_PDM_InitField( &m_showGas, "ShowGas", false, "Gas", "", "", "" );
    CAF_PDM_InitField( &m_showWater, "ShowWater", true, "Water", "", "", "" );

    CAF_PDM_InitField( &m_showResult, "ShowResult", false, "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_vectorView, "VectorView", "View vectors", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_vectorSurfaceCrossingLocation,
                                "VectorSurfaceCrossingLocation",
                                "Vectors touching surface",
                                "",
                                "",
                                "" );
    m_vectorSurfaceCrossingLocation.uiCapability()->setUiReadOnly(
        m_vectorView() == RimElementVectorResult::VectorView::CELL_CENTER_TOTAL );

    CAF_PDM_InitField( &m_showVectorI, "ShowVectorI", true, "I", "", "", "" );
    CAF_PDM_InitField( &m_showVectorJ, "ShowVectorJ", true, "J", "", "", "" );
    CAF_PDM_InitField( &m_showVectorK, "ShowVectorK", true, "K", "", "", "" );
    CAF_PDM_InitField( &m_showNncData, "ShowNncData", true, "Show NNC data", "", "", "" );
    CAF_PDM_InitField( &m_threshold, "Threshold", 0.0f, "Threshold", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_vectorColor, "VectorColor", "Color", "", "", "" );
    cvf::Color3f defaultUniformColor = cvf::Color3f::BLACK;
    CAF_PDM_InitField( &m_uniformVectorColor, "UniformVectorColor", defaultUniformColor, "Uniform Vector Color", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_scaleMethod, "ScaleMethod", "Scale Method", "", "", "" );
    CAF_PDM_InitField( &m_sizeScale, "SizeScale", 1.0f, "Size Scale", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::~RimElementVectorResult()
{
    delete m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::setShowResult( bool enableResult )
{
    m_showResult = enableResult;

    updateConnectedEditors();
    updateUiIconFromState( enableResult );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showResult() const
{
    return m_showResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::VectorView RimElementVectorResult::vectorView() const
{
    return m_vectorView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showOil() const
{
    return m_showOil();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showGas() const
{
    return m_showGas();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showWater() const
{
    return m_showWater();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showVectorI() const
{
    return m_showVectorI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showVectorJ() const
{
    return m_showVectorJ();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

bool RimElementVectorResult::showVectorK() const
{
    return m_showVectorK();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::showNncData() const
{
    return m_showNncData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::VectorSurfaceCrossingLocation RimElementVectorResult::vectorSuraceCrossingLocation() const
{
    return m_vectorSurfaceCrossingLocation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimElementVectorResult::threshold() const
{
    return m_threshold();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimElementVectorResult::sizeScale() const
{
    return m_sizeScale();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::TensorColors RimElementVectorResult::vectorColors() const
{
    return m_vectorColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::ScaleMethod RimElementVectorResult::scaleMethod() const
{
    return m_scaleMethod();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Color3f& RimElementVectorResult::getUniformVectorColor() const
{
    return m_uniformVectorColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::mappingRange( double& min, double& max ) const
{
    min = 0.0;
    max = 0.0;

    Rim3dView* view = nullptr;
    firstAncestorOrThisOfType( view );

    int currentTimeStep = view->currentTimeStep();

    std::vector<RigEclipseResultAddress> resVarAddresses;
    size_t                               directions = 1;
    if ( !resultAddressesIJK( resVarAddresses ) ) return;
    std::vector<RigEclipseResultAddress> cleanedResVarAddresses;
    directions = 0;
    std::vector<cvf::Vec3d> unitVectors;

    // resVarAddresses contains three directions per fluid, check which of them shall be used.
    for ( size_t fluidDirIndex = 0; fluidDirIndex < resVarAddresses.size(); fluidDirIndex += 3 )
    {
        if ( showVectorI() )
        {
            // Only increment directions and add to unit vectors once per direction (not per direction for each fluid).
            if ( fluidDirIndex == 0 )
            {
                directions++;
                unitVectors.push_back( cvf::Vec3d::X_AXIS );
            }
            cleanedResVarAddresses.push_back( resVarAddresses.at( 0 + fluidDirIndex ) );
        }
        if ( showVectorJ() )
        {
            if ( fluidDirIndex == 0 )
            {
                directions++;
                unitVectors.push_back( cvf::Vec3d::Y_AXIS );
            }
            cleanedResVarAddresses.push_back( resVarAddresses.at( 1 + fluidDirIndex ) );
        }
        if ( showVectorK() )
        {
            if ( fluidDirIndex == 0 )
            {
                directions++;
                unitVectors.push_back( cvf::Vec3d::Z_AXIS );
            }
            cleanedResVarAddresses.push_back( resVarAddresses.at( 2 + fluidDirIndex ) );
        }
    }
    resVarAddresses = cleanedResVarAddresses;

    if ( directions > 0 )
    {
        std::vector<double> directionsMax( directions, 0.0 );
        std::vector<double> directionsMin( directions, 0.0 );

        for ( size_t index = 0; index < resVarAddresses.size(); index += directions )
        {
            cvf::Vec3d aggregatedVectorMax;
            cvf::Vec3d aggregatedVectorMin;
            for ( size_t dir = 0; dir < directions; dir += 1 )
            {
                double localMin = cvf::UNDEFINED_DOUBLE;
                double localMax = cvf::UNDEFINED_DOUBLE;

                RigEclipseResultAddress resVarAddr = resVarAddresses.at( index + dir );
                if ( !resVarAddr.isValid() ) return;

                RimEclipseView*         eclipseView = dynamic_cast<RimEclipseView*>( view );
                RigCaseCellResultsData* resultsData =
                    eclipseView->eclipseCase()->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

                resultsData->ensureKnownResultLoaded( resVarAddr );

                if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS )
                {
                    resultsData->minMaxCellScalarValues( resVarAddr, localMin, localMax );
                }
                else if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_CURRENT_TIMESTEP )
                {
                    resultsData->minMaxCellScalarValues( resVarAddr, currentTimeStep, localMin, localMax );
                }
                if ( vectorView() == RimElementVectorResult::VectorView::CELL_CENTER_TOTAL )
                {
                    aggregatedVectorMax += unitVectors.at( dir ) * localMax;
                    aggregatedVectorMin += unitVectors.at( dir ) * localMin;
                }
                else
                {
                    directionsMax[dir] += localMax;
                    directionsMin[dir] += localMin;
                }
            }
            if ( vectorView() == RimElementVectorResult::VectorView::CELL_CENTER_TOTAL )
            {
                directionsMax[0] += aggregatedVectorMax.length();
                directionsMin[0] += aggregatedVectorMin.length();
            }
        }
        min = directionsMin.front();
        max = directionsMax.front();
        if ( vectorView() != RimElementVectorResult::VectorView::CELL_CENTER_TOTAL )
        {
            for ( size_t i = 0; i < directionsMax.size(); i++ )
            {
                max = std::max<double>( max, directionsMax.at( i ) );
                min = std::min<double>( min, directionsMin.at( i ) );
            }
        }
        else
        {
            max = std::max<double>( max, min );
            min = 0.0;
        }
    }

    if ( showNncData() )
    {
        RigNNCData* nncData =
            dynamic_cast<RimEclipseView*>( view )->eclipseCase()->eclipseCaseData()->mainGrid()->nncData();
        std::vector<RigEclipseResultAddress> combinedAddresses;
        if ( !resultAddressesCombined( combinedAddresses ) ) return;

        for ( size_t flIdx = 0; flIdx < combinedAddresses.size(); flIdx++ )
        {
            if ( combinedAddresses[flIdx].m_resultCatType == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
            {
                if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS )
                {
                    const std::vector<std::vector<double>>* nncResultVals =
                        nncData->dynamicConnectionScalarResult( combinedAddresses[flIdx] );
                    for ( size_t i = 0; i < nncResultVals->size(); i++ )
                    {
                        for ( size_t j = 0; j < nncResultVals->at( i ).size(); j++ )
                        {
                            max = std::max<double>( max, nncResultVals->at( i ).at( j ) );
                            min = std::min<double>( min, nncResultVals->at( i ).at( j ) );
                        }
                    }
                }
                else if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_CURRENT_TIMESTEP )
                {
                    const std::vector<double>* nncResultVals =
                        nncData->dynamicConnectionScalarResult( combinedAddresses[flIdx],
                                                                static_cast<size_t>( currentTimeStep ) );
                    for ( size_t i = 0; i < nncResultVals->size(); i++ )
                    {
                        max = std::max<double>( max, nncResultVals->at( i ) );
                        min = std::min<double>( min, nncResultVals->at( i ) );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                  bool       isUsingOverrideViewer )
{
    QStringList resultNames;
    if ( showOil() )
    {
        resultNames << QString( "Oil" );
    }
    if ( showGas() )
    {
        resultNames << QString( "Gas" );
    }
    if ( showWater() )
    {
        resultNames << QString( "Water" );
    }

    m_legendConfig->setTitle( QString( "Element Vector Result: \n" ) + resultNames.join( ", " ) );

    double minResultValue;
    double maxResultValue;
    mappingRange( minResultValue, maxResultValue );
    m_legendConfig->setAutomaticRanges( minResultValue, maxResultValue, minResultValue, maxResultValue );

    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );

    double posClosestToZero = HUGE_VAL;
    double negClosestToZero = -HUGE_VAL;
    m_legendConfig->setClosestToZeroValues( posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero );
    nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( m_legendConfig->titledOverlayFrame(), isUsingOverrideViewer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig* RimElementVectorResult::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( changedField == &m_showResult )
    {
        setShowResult( m_showResult );
    }
    if ( changedField == &m_vectorView )
    {
        m_vectorSurfaceCrossingLocation.uiCapability()->setUiReadOnly(
            vectorView() == RimElementVectorResult::VectorView::CELL_CENTER_TOTAL );
    }

    RimEclipseView* view;
    firstAncestorOrThisOfType( view );
    view->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimElementVectorResult::objectToggleField()
{
    return &m_showResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* fluidsGroup = uiOrdering.addNewGroup( "Fluids" );
    fluidsGroup->add( &m_showOil );
    fluidsGroup->add( &m_showGas );
    fluidsGroup->add( &m_showWater );

    caf::PdmUiGroup* visibilityGroup = uiOrdering.addNewGroup( "Visibility" );
    visibilityGroup->add( &m_vectorView );
    visibilityGroup->add( &m_vectorSurfaceCrossingLocation );
    visibilityGroup->add( &m_showVectorI );
    visibilityGroup->add( &m_showVectorJ );
    visibilityGroup->add( &m_showVectorK );
    visibilityGroup->add( &m_showNncData );
    visibilityGroup->add( &m_threshold );

    caf::PdmUiGroup* vectorColorsGroup = uiOrdering.addNewGroup( "Vector Colors" );
    vectorColorsGroup->add( &m_vectorColor );
    if ( m_vectorColor == TensorColors::UNIFORM_COLOR )
    {
        vectorColorsGroup->add( &m_uniformVectorColor );
    }

    caf::PdmUiGroup* vectorSizeGroup = uiOrdering.addNewGroup( "Vector Size" );
    vectorSizeGroup->add( &m_sizeScale );
    vectorSizeGroup->add( &m_scaleMethod );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::resultAddressesCombined( std::vector<RigEclipseResultAddress>& addresses ) const
{
    addresses.clear();

    if ( showOil() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                      RiaDefines::combinedOilFluxResultName() ) );
    }
    if ( showGas() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                      RiaDefines::combinedGasFluxResultName() ) );
    }
    if ( showWater() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                      RiaDefines::combinedWaterFluxResultName() ) );
    }
    return addresses.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::resultAddressesIJK( std::vector<RigEclipseResultAddress>& addresses ) const
{
    addresses.clear();

    if ( showOil() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) );
    }
    if ( showGas() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) );
    }
    if ( showWater() )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATK+" ) );
    }

    return addresses.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( &m_legendConfig );
    uiTreeOrdering.skipRemainingChildren();
}
