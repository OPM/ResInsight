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
    addItem( RimElementVectorResult::RESULT_COLORS, "RESULT_COLORS", "Result Colors" );
    addItem( RimElementVectorResult::UNIFORM_COLOR, "UNIFORM_COLOR", "Uniform" );

    setDefault( RimElementVectorResult::RESULT_COLORS );
}

template <>
void AppEnum<RimElementVectorResult::ScaleMethod>::setUp()
{
    addItem( RimElementVectorResult::RESULT, "RESULT", "Result" );
    addItem( RimElementVectorResult::CONSTANT, "CONSTANT", "Constant" );

    setDefault( RimElementVectorResult::RESULT );
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

    CAF_PDM_InitField( &m_resultName, "ResultVariable", QString( "Oil" ), "Value", "", "", "" );
    m_resultName.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_resultName.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_showResult, "ShowResult", false, "", "", "", "" );

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
    min = cvf::UNDEFINED_DOUBLE;
    max = cvf::UNDEFINED_DOUBLE;

    Rim3dView* view = nullptr;
    firstAncestorOrThisOfType( view );

    int currentTimeStep = view->currentTimeStep();

    RigEclipseResultAddress resVarAddr = resultAddressCombined();
    if ( !resVarAddr.isValid() ) return;

    RimEclipseView*         eclipseView = dynamic_cast<RimEclipseView*>( view );
    RigCaseCellResultsData* resultsData =
        eclipseView->eclipseCase()->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->ensureKnownResultLoaded( resVarAddr );

    if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS )
    {
        resultsData->minMaxCellScalarValues( resVarAddr, min, max );
    }
    else if ( m_legendConfig->rangeMode() == RimRegularLegendConfig::RangeModeType::AUTOMATIC_CURRENT_TIMESTEP )
    {
        resultsData->minMaxCellScalarValues( resVarAddr, currentTimeStep, min, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                  bool       isUsingOverrideViewer )
{
    m_legendConfig->setTitle( QString( "Element Vector Result: \n" ) + resultName() );

    double minResultValue;
    double maxResultValue;
    mappingRange( minResultValue, maxResultValue );
    m_legendConfig->setAutomaticRanges( minResultValue, maxResultValue, minResultValue, maxResultValue );

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
QString RimElementVectorResult::resultName() const
{
    return m_resultName();
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

    if ( fieldNeedingOptions == &m_resultName )
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
    uiOrdering.add( &m_resultName );

    caf::PdmUiGroup* visibilityGroup = uiOrdering.addNewGroup( "Visibility" );
    visibilityGroup->add( &m_showVectorI );
    visibilityGroup->add( &m_showVectorJ );
    visibilityGroup->add( &m_showVectorK );
    visibilityGroup->add( &m_showNncData );
    visibilityGroup->add( &m_threshold );

    caf::PdmUiGroup* vectorColorsGroup = uiOrdering.addNewGroup( "Vector Colors" );
    vectorColorsGroup->add( &m_vectorColor );
    if ( m_vectorColor == UNIFORM_COLOR )
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
    if ( field == &m_resultName )
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
RigEclipseResultAddress RimElementVectorResult::resultAddressCombined() const
{
    if ( resultName() == "Oil" )
    {
        return RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                        RiaDefines::combinedOilFluxResultName() );
    }
    else if ( resultName() == "Gas" )
    {
        return RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                        RiaDefines::combinedGasFluxResultName() );
    }
    else if ( resultName() == "Water" )
    {
        return RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                        RiaDefines::combinedWaterFluxResultName() );
    }

    return RigEclipseResultAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElementVectorResult::resultAddressIJK( std::vector<RigEclipseResultAddress>& addresses ) const
{
    addresses.clear();

    // TODO: use enum??
    if ( resultName() == "Oil" )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) );
        return true;
    }
    else if ( resultName() == "Gas" )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) );
        return true;
    }
    else if ( resultName() == "Water" )
    {
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATI+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATJ+" ) );
        addresses.push_back( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATK+" ) );
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElementVectorResult::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( &m_legendConfig );
    uiTreeOrdering.skipRemainingChildren();
}
