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
    addItem( RimElementVectorResult::VectorView::AGGREGATED, "AGGREGATED", "Aggregated" );
    addItem( RimElementVectorResult::VectorView::INDIVIDUAL, "INDIVIDUAL", "Individual" );

    setDefault( RimElementVectorResult::VectorView::AGGREGATED );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElementVectorResult::RimElementVectorResult()
{
    CAF_PDM_InitObject( "Element Vector Result", ":/CellResult.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_resultNames, "ResultVariable", std::vector<QString>{QString( "Oil" )}, "Fluid", "", "", "" );
    m_resultNames.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_resultNames.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_showResult, "ShowResult", false, "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_vectorView, "VectorView", "View vectors", "", "", "" );

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
RimElementVectorResult::VectorView RimElementVectorResult::vectorView()
{
    return m_vectorView();
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
    if ( !resultAddressesCombined( resVarAddresses ) ) return;

    for ( size_t index = 0; index < resVarAddresses.size(); index++ )
    {
        double localMin = cvf::UNDEFINED_DOUBLE;
        double localMax = cvf::UNDEFINED_DOUBLE;

        RigEclipseResultAddress resVarAddr = resVarAddresses.at( index );
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
        min += localMin;
        max += localMax;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                  bool       isUsingOverrideViewer )
{
    QStringList resultNames;
    for ( size_t i = 0; i < m_resultNames().size(); i++ )
    {
        resultNames << m_resultNames().at( i );
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
std::vector<QString> RimElementVectorResult::resultNames() const
{
    return m_resultNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RimElementVectorResult::getResultMetaDataForUIFieldSetting()
{
    std::vector<std::string> fieldNames;
    fieldNames.push_back( "Oil" );
    fieldNames.push_back( "Water" );
    fieldNames.push_back( "Gas" );

    return fieldNames;
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
QList<caf::PdmOptionItemInfo>
    RimElementVectorResult::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &m_resultNames )
    {
        std::vector<std::string> fieldCompNames = getResultMetaDataForUIFieldSetting();

        for ( size_t oIdx = 0; oIdx < fieldCompNames.size(); ++oIdx )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::fromStdString( fieldCompNames[oIdx] ),
                                                       QString::fromStdString( fieldCompNames[oIdx] ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultNames );

    caf::PdmUiGroup* visibilityGroup = uiOrdering.addNewGroup( "Visibility" );
    visibilityGroup->add( &m_vectorView );
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
void RimElementVectorResult::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                    QString                    uiConfigName,
                                                    caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_resultNames )
    {
        caf::PdmUiListEditorAttribute* listEditAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( listEditAttr )
        {
            listEditAttr->m_heightHint = 50;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::resultAddressesCombined( std::vector<RigEclipseResultAddress>& addresses ) const
{
    addresses.clear();

    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Oil" ) )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                      RiaDefines::combinedOilFluxResultName() ) );
    }
    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Gas" ) )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                      RiaDefines::combinedGasFluxResultName() ) );
    }
    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Water" ) )
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

    // TODO: use enum??
    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Oil" ) )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) );
    }
    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Gas" ) )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) );
    }
    if ( std::count( m_resultNames().begin(), m_resultNames().end(), "Water" ) )
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
