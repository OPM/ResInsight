/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellRftEnsembleCurveSet.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimWellRftPlot.h"

#include "RiuCvfOverlayItemWidget.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <QFrame>

CAF_PDM_SOURCE_INIT( RimWellRftEnsembleCurveSet, "WellRftEnsembleCurveSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet::RimWellRftEnsembleCurveSet()
{
    CAF_PDM_InitObject( "Ensemble Curve Set", ":/EnsembleCurveSet16x16.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );
    m_ensemble.uiCapability()->setUiTreeChildrenHidden( true );
    m_ensemble.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "NameAndUnit", "NameAndUnit", "", "", "" );
    m_ensembleName.registerGetMethod( this, &RimWellRftEnsembleCurveSet::ensembleName );
    m_ensembleName.uiCapability()->setUiHidden( true );
    m_ensembleName.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_ensembleColorMode, "ColorMode", ColorModeEnum( ColorMode::SINGLE_COLOR ), "Coloring Mode", "", "", "" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter", "", "", "" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_ensembleLegendConfig, "LegendConfig", "", "", "", "" );
    m_ensembleLegendConfig = new RimRegularLegendConfig();
    m_ensembleLegendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet::~RimWellRftEnsembleCurveSet()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimWellRftEnsembleCurveSet::ensemble() const
{
    return m_ensemble;
}

void RimWellRftEnsembleCurveSet::setEnsemble( RimSummaryCaseCollection* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetColorManager::ColorMode RimWellRftEnsembleCurveSet::colorMode() const
{
    return m_ensembleColorMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::setColorMode( ColorMode mode )
{
    m_ensembleColorMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::initializeLegend()
{
    auto ensembleParam = m_ensemble->ensembleParameter( m_ensembleParameter );
    m_ensembleLegendConfig->setTitle( m_ensemble->name() + "\n" + m_ensembleParameter );
    RimEnsembleCurveSetColorManager::initializeLegendConfig( m_ensembleLegendConfig, ensembleParam );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellRftEnsembleCurveSet::caseColor( const RimSummaryCase* summaryCase ) const
{
    auto ensembleParam = m_ensemble->ensembleParameter( m_ensembleParameter );
    return RimEnsembleCurveSetColorManager::caseColor( m_ensembleLegendConfig, summaryCase, ensembleParam );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellRftEnsembleCurveSet::currentEnsembleParameter() const
{
    return m_ensembleParameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::setEnsembleParameter( const QString& parameterName )
{
    m_ensembleParameter = parameterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellRftEnsembleCurveSet::allEnsembleParameters() const
{
    std::set<QString>         paramSet;
    RimSummaryCaseCollection* group = m_ensemble;
    if ( group )
    {
        for ( RimSummaryCase* rimCase : group->allSummaryCases() )
        {
            if ( rimCase->caseRealizationParameters() != nullptr )
            {
                auto ps = rimCase->caseRealizationParameters()->parameters();
                for ( auto p : ps )
                    paramSet.insert( p.first );
            }
        }
    }
    return std::vector<QString>( paramSet.begin(), paramSet.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimWellRftEnsembleCurveSet::legendConfig()
{
    return m_ensembleLegendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter::Type RimWellRftEnsembleCurveSet::currentEnsembleParameterType() const
{
    if ( m_ensembleColorMode() == ColorMode::BY_ENSEMBLE_PARAM )
    {
        RimSummaryCaseCollection* group         = m_ensemble();
        QString                   parameterName = m_ensembleParameter();

        if ( group && !parameterName.isEmpty() )
        {
            auto eParam = group->ensembleParameter( parameterName );
            return eParam.type;
        }
    }
    return EnsembleParameter::TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    if ( changedField == &m_ensembleColorMode || changedField == &m_ensembleParameter )
    {
        RimWellRftPlot* rftPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( rftPlot );
        rftPlot->syncCurvesFromUiSelection();
        rftPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellRftEnsembleCurveSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        for ( const QString& param : allEnsembleParameters() )
        {
            options.push_back( caf::PdmOptionItemInfo( param, param ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* colorsGroup = uiOrdering.addNewGroup( "Ensemble Curve Colors" );
    colorsGroup->add( &m_ensembleColorMode );

    if ( m_ensembleColorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        colorsGroup->add( &m_ensembleParameter );
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                       QString                 uiConfigName /*= "" */ )
{
    if ( m_ensembleColorMode == ColorMode::BY_ENSEMBLE_PARAM && !m_ensembleParameter().isEmpty() )
    {
        uiTreeOrdering.add( m_ensembleLegendConfig() );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellRftEnsembleCurveSet::userDescriptionField()
{
    return &m_ensembleName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellRftEnsembleCurveSet::ensembleName() const
{
    if ( m_ensemble ) return m_ensemble->name();

    return "";
}
