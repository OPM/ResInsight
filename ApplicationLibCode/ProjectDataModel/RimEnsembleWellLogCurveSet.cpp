/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimEnsembleWellLogCurveSet.h"

#include "RiaColorTools.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaResultNames.h"
#include "RiaStatisticsTools.h"
#include "RiaSummaryCurveAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaTimeTTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleWellLogStatisticsCurve.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleStatistics.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimEnsembleWellLogStatistics.h"
#include "RimEnsembleWellLogs.h"
#include "RimEnsembleWellLogsCollection.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"
#include "RiuTextContentFrame.h"

#include "cafPdmObject.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafTitledOverlayFrame.h"

// TODO: remove?
#include "RifEnsembleStatisticsReader.h"

// #include "cvfScalarMapper.h"

#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <algorithm>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
int statisticsCurveSymbolSize( RiuQwtSymbol::PointSymbolEnum symbol );

CAF_PDM_SOURCE_INIT( RimEnsembleWellLogCurveSet, "RimEnsembleWellLogCurveSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogCurveSet::RimEnsembleWellLogCurveSet()
    : filterChanged( this )

{
    CAF_PDM_InitObject( "Ensemble Curve Set", ":/EnsembleCurveSet16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_curves, "EnsembleCurveSet", "Ensemble Curve Set", "", "", "" );
    m_curves.uiCapability()->setUiHidden( true );
    m_curves.uiCapability()->setUiTreeChildrenHidden( false );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves", "", "", "" );
    m_showCurves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleWellLogs, "EnsembleWellLogs", "Ensemble Well Logs", "", "", "" );
    m_ensembleWellLogs.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelName, "WellLogChannelName", "Well Log Channel Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "FilterEnsembleCurveSet", "Filter by Ensemble Curve Set", "", "", "" );

    CAF_PDM_InitField( &m_colorMode, "ColorMode", caf::AppEnum<ColorMode>( ColorMode::SINGLE_COLOR ), "Coloring Mode", "", "", "" );

    CAF_PDM_InitField( &m_color, "Color", RiaColorTools::textColor3f(), "Color", "", "", "" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter", "", "", "" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE ) );

    CAF_PDM_InitFieldNoDefault( &m_curveFilters, "CurveFilters", "Curve Filters", "", "", "" );
    m_curveFilters = new RimEnsembleCurveFilterCollection();
    m_curveFilters->setUiTreeHidden( true );
    m_curveFilters->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_statistics, "Statistics", "Statistics", "", "", "" );
    m_statistics = new RimEnsembleStatistics( this );
    m_statistics.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_userDefinedName, "UserDefinedName", QString( "Ensemble Curve Set" ), "Curve Set Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_autoGeneratedName, "AutoGeneratedName", "Curve Set Name", "", "", "" );
    m_autoGeneratedName.registerGetMethod( this, &RimEnsembleWellLogCurveSet::createAutoName );
    m_autoGeneratedName.uiCapability()->setUiReadOnly( true );
    m_autoGeneratedName.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_isUsingAutoName, "AutoName", true, "Auto Name", "", "", "" );

    m_qwtPlotCurveForLegendText = new QwtPlotCurve;
    m_qwtPlotCurveForLegendText->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );

    m_ensembleWellLogStatistics.reset( new RimEnsembleWellLogStatistics );

    m_disableStatisticCurves = false;
    m_isCurveSetFiltered     = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogCurveSet::~RimEnsembleWellLogCurveSet()
{
    //    m_curves.deleteAllChildObjects();

    RimWellLogPlot* parentPlot;
    firstAncestorOrThisOfType( parentPlot );
    // if ( parentPlot && parentPlot->viewer() )
    // {
    //     m_qwtPlotCurveForLegendText->detach();
    //     if ( m_legendOverlayFrame )
    //     {
    //         parentPlot->viewer()->removeOverlayFrame( m_legendOverlayFrame );
    //     }
    // }
    if ( m_legendOverlayFrame )
    {
        m_legendOverlayFrame->setParent( nullptr );
        delete m_legendOverlayFrame;
    }
    if ( m_filterOverlayFrame )
    {
        m_filterOverlayFrame->setParent( nullptr );
        delete m_filterOverlayFrame;
    }

    delete m_qwtPlotCurveForLegendText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::isCurvesVisible()
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::setColor( cvf::Color3f color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::loadDataAndUpdate( bool updateParentPlot )
{
    m_curveFilters->loadDataAndUpdate();

    updateAllCurves();
    updateFilterLegend();

    if ( updateParentPlot )
    {
        RimWellLogTrack* parentPlot;
        firstAncestorOrThisOfTypeAsserted( parentPlot );
        parentPlot->viewer()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::setParentQwtPlotNoReplot( QwtPlot* plot )
{
    for ( RimWellLogCurve* curve : m_curves )
    {
        curve->setParentQwtPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::detachQwtCurves()
{
    for ( RimWellLogCurve* curve : m_curves )
    {
        curve->detachQwtCurve();
    }

    m_qwtPlotCurveForLegendText->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::reattachQwtCurves()
{
    for ( RimWellLogCurve* curve : m_curves )
    {
        curve->reattachQwtCurve();
    }

    m_qwtPlotCurveForLegendText->detach();

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    // if ( plot )
    // {
    //     m_qwtPlotCurveForLegendText->attach( plot->viewer() );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::addCurve( RimWellLogCurve* curve )
{
    if ( curve )
    {
        RimWellLogPlot* plot;
        firstAncestorOrThisOfType( plot );
        // if ( plot ) curve->setParentQwtPlotNoReplot( plot->viewer() );

        curve->setColor( m_color );
        m_curves.push_back( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::deleteCurve( RimWellLogCurve* curve )
{
    // if ( curve )
    // {
    //     m_curves.removeChildObject( curve );
    //     delete curve;
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RimEnsembleWellLogCurveSet::curves() const
{
    return m_curves.ptrReferencedObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::deleteEnsembleCurves()
{
    RimWellLogTrack* plotTrack = nullptr;
    firstAncestorOrThisOfType( plotTrack );
    CVF_ASSERT( plotTrack );

    std::vector<size_t> curvesIndexesToDelete;
    for ( size_t c = 0; c < m_curves.size(); c++ )
    {
        RimWellLogCurve* curve = m_curves[c];

        if ( dynamic_cast<RimEnsembleWellLogStatisticsCurve*>( curve ) == nullptr )
        {
            plotTrack->removeCurve( m_curves[c] );
            curvesIndexesToDelete.push_back( c );
        }
    }

    while ( curvesIndexesToDelete.size() > 0 )
    {
        size_t currIndex = curvesIndexesToDelete.back();
        m_curves.erase( currIndex );
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::deleteStatisticsCurves()
{
    RimWellLogTrack* plotTrack = nullptr;
    firstAncestorOrThisOfType( plotTrack );
    CVF_ASSERT( plotTrack );

    std::vector<size_t> curvesIndexesToDelete;
    for ( size_t c = 0; c < m_curves.size(); c++ )
    {
        RimWellLogCurve* curve = m_curves[c];

        if ( dynamic_cast<RimEnsembleWellLogStatisticsCurve*>( curve ) != nullptr )
        {
            plotTrack->removeCurve( m_curves[c] );
            curvesIndexesToDelete.push_back( c );
        }
    }

    while ( curvesIndexesToDelete.size() > 0 )
    {
        size_t currIndex = curvesIndexesToDelete.back();
        m_curves.erase( currIndex );
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEnsembleWellLogCurveSet::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame* RimEnsembleWellLogCurveSet::legendFrame() const
{
    return m_legendOverlayFrame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::onLegendDefinitionChanged()
{
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleWellLogCurveSet::filterCollection() const
{
    return m_curveFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogCurveSet::ColorMode RimEnsembleWellLogCurveSet::colorMode() const
{
    return m_colorMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::setColorMode( ColorMode mode )
{
    m_colorMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::setEnsembleParameter( const QString& parameterName )
{
    m_ensembleParameter = parameterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter::Type RimEnsembleWellLogCurveSet::currentEnsembleParameterType() const
{
    if ( m_colorMode() == ColorMode::BY_ENSEMBLE_PARAM )
    {
        // RimSummaryCaseCollection* group         = m_yValuesSummaryCaseCollection();
        // QString                   parameterName = m_ensembleParameter();

        // if ( group && !parameterName.isEmpty() )
        // {
        //     auto eParam = group->ensembleParameter( parameterName );
        //     return eParam.type;
        // }
    }
    return RigEnsembleParameter::TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateAllCurves()
{
    RimEnsembleWellLogs* group = m_ensembleWellLogs;

    if ( group )
    {
        std::vector<RimWellLogFile*> allWellLogFiles = group->wellLogFiles();
        std::vector<RimWellLogFile*> filteredCases   = filterEnsembleCases( allWellLogFiles );

        m_isCurveSetFiltered = filteredCases.size() < allWellLogFiles.size();

        updateEnsembleCurves( filteredCases );
        updateStatisticsCurves( m_statistics->basedOnFilteredCases() ? filteredCases : allWellLogFiles );
    }

    filterChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateEditors()
{
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    bool updateTextInPlot = false;

    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );

        updateConnectedEditors();

        RimWellLogPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( summaryPlot );
        summaryPlot->updateConnectedEditors();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_ensembleWellLogs )
    {
        updateAllCurves();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_ensembleCurveSet )
    {
        updateAllCurves();

        loadDataAndUpdate( true );
        updateTextInPlot = true;
    }
    else if ( changedField == &m_wellLogChannelName )
    {
        updateAllCurves();
        updateTextInPlot = true;
    }
    else if ( changedField == &m_color )
    {
        updateCurveColors();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_ensembleParameter )
    {
        updateLegendMappingMode();
        updateCurveColors();
    }
    else if ( changedField == &m_colorMode )
    {
        m_ensembleParameter.uiCapability()->setUiHidden( m_colorMode() != ColorMode::BY_ENSEMBLE_PARAM );

        if ( m_colorMode() == ColorMode::BY_ENSEMBLE_PARAM )
        {
            if ( m_ensembleParameter().isEmpty() )
            {
                auto params         = variationSortedEnsembleParameters();
                m_ensembleParameter = !params.empty() ? params.front().name : "";
            }
        }

        updateCurveColors();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_isUsingAutoName )
    {
        if ( !m_isUsingAutoName )
        {
            m_userDefinedName = createAutoName();
        }

        updateTextInPlot = true;
    }
    else if ( changedField == &m_userDefinedName )
    {
        updateTextInPlot = true;
    }

    if ( updateTextInPlot )
    {
        updateAllTextInPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_ensembleWellLogs );
    uiOrdering.add( &m_wellLogChannelName );
    uiOrdering.add( &m_ensembleCurveSet );

    appendColorGroup( uiOrdering );

    {
        caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
        nameGroup->setCollapsedByDefault( true );
        nameGroup->add( &m_isUsingAutoName );
        if ( m_isUsingAutoName )
        {
            nameGroup->add( &m_autoGeneratedName );
        }
        else
        {
            nameGroup->add( &m_userDefinedName );
        }
    }

    caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup( "Statistics" );
    m_statistics->defineUiOrdering( uiConfigName, *statGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::appendColorGroup( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* colorsGroup = uiOrdering.addNewGroup( "Colors" );
    colorsGroup->add( &m_colorMode );

    if ( m_colorMode == ColorMode::SINGLE_COLOR )
    {
        colorsGroup->add( &m_color );
    }
    else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        colorsGroup->add( &m_ensembleParameter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                       QString                 uiConfigName /*= ""*/ )
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        uiTreeOrdering.add( m_legendConfig() );
    }

    for ( auto filter : m_curveFilters->filters() )
    {
        uiTreeOrdering.add( filter );
    }

    uiTreeOrdering.skipRemainingChildren( true );

    caf::IconProvider iconProvider = this->uiIconProvider();
    if ( !iconProvider.valid() ) return;

    // RimEnsembleWellLogCurveSetCollection* coll = nullptr;
    // this->firstAncestorOrThisOfType( coll );
    // if ( coll && coll->curveSetForSourceStepping() == this )
    // {
    //     iconProvider.setOverlayResourceString( ":/StepUpDownCorner16x16.png" );
    // }

    this->setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleWellLogCurveSet::userDescriptionField()
{
    if ( m_isUsingAutoName )
    {
        return &m_autoGeneratedName;
    }
    else
    {
        return &m_userDefinedName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleWellLogCurveSet::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEnsembleWellLogCurveSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleWellLogs )
    {
        RimProject*                       proj = RimProject::current();
        std::vector<RimEnsembleWellLogs*> groups =
            proj->activeOilField()->ensembleWellLogsCollection()->ensembleWellLogs();

        for ( RimEnsembleWellLogs* ensemble : groups )
        {
            options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_wellLogChannelName )
    {
        if ( m_ensembleWellLogs )
        {
            std::set<QString> wellLogChannelNames;
            for ( auto wellLogFile : m_ensembleWellLogs->wellLogFiles() )
            {
                std::vector<RimWellLogFileChannel*> fileLogs = wellLogFile->wellLogChannels();
                for ( size_t i = 0; i < fileLogs.size(); i++ )
                {
                    QString wellLogChannelName = fileLogs[i]->name();
                    wellLogChannelNames.insert( wellLogChannelName );
                }
            }

            for ( auto wellLogChannelName : wellLogChannelNames )
            {
                options.push_back( caf::PdmOptionItemInfo( wellLogChannelName, wellLogChannelName ) );
            }
        }

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensembleCurveSet )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        RimMainPlotCollection*            mainPlotColl = RimProject::current()->mainPlotCollection();
        std::vector<RimEnsembleCurveSet*> ensembleCurveSets;
        mainPlotColl->descendantsOfType( ensembleCurveSets );
        for ( auto ensembleCurveSet : ensembleCurveSets )
        {
            options.push_back( caf::PdmOptionItemInfo( ensembleCurveSet->name(), ensembleCurveSet ) );
        }
    }
    else if ( fieldNeedingOptions == &m_colorMode )
    {
        auto singleColorOption = ColorModeEnum( ColorMode::SINGLE_COLOR );
        auto byEnsParamOption  = ColorModeEnum( ColorMode::BY_ENSEMBLE_PARAM );

        options.push_back( caf::PdmOptionItemInfo( singleColorOption.uiText(), ColorMode::SINGLE_COLOR ) );

        RimEnsembleWellLogs* ensembleWellLogs = m_ensembleWellLogs;
        if ( ensembleWellLogs && ensembleWellLogs->hasEnsembleParameters() )
        {
            options.push_back( caf::PdmOptionItemInfo( byEnsParamOption.uiText(), ColorMode::BY_ENSEMBLE_PARAM ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        auto params = correlationSortedEnsembleParameters();
        for ( const auto& paramCorrPair : params )
        {
            QString name = paramCorrPair.first.name;
            double  corr = paramCorrPair.second;
            options.push_back(
                caf::PdmOptionItemInfo( QString( "%1 (Avg. correlation: %2)" ).arg( name ).arg( corr, 5, 'f', 2 ), name ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateFilterLegend()
{
    RimWellLogTrack* plotTrack;
    firstAncestorOrThisOfType( plotTrack );

    if ( plotTrack && plotTrack->viewer() )
    {
        if ( m_ensembleCurveSet != nullptr && m_ensembleCurveSet->isFiltered() )
        {
            if ( !m_filterOverlayFrame )
            {
                m_filterOverlayFrame =
                    new RiuDraggableOverlayFrame( plotTrack->viewer()->canvas(), plotTrack->viewer()->overlayMargins() );
            }
            m_filterOverlayFrame->setContentFrame( m_ensembleCurveSet->curveFilters()->makeFilterDescriptionFrame() );
            plotTrack->viewer()->addOverlayFrame( m_filterOverlayFrame );
        }
        else
        {
            if ( m_filterOverlayFrame )
            {
                plotTrack->viewer()->removeOverlayFrame( m_filterOverlayFrame );
            }
        }
        plotTrack->viewer()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateCurveColors()
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        QString parameterName = m_ensembleParameter();

        {
            QString legendTitle;
            if ( m_isUsingAutoName )
            {
                legendTitle = m_autoGeneratedName();
            }
            else
            {
                legendTitle += m_userDefinedName();
            }

            legendTitle += "\n";
            legendTitle += parameterName;

            m_legendConfig->setTitle( legendTitle );
        }

        // if ( group && !parameterName.isEmpty() && !group->allSummaryCases().empty() )
        // {
        //     auto ensembleParam = group->ensembleParameter( parameterName );
        //     if ( ensembleParam.isText() || ensembleParam.isNumeric() )
        //     {
        //         RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, ensembleParam );
        //         // for ( auto& curve : m_curves )
        //         // {
        //         //     // if ( curve->summaryAddressY().category() ==
        //         //     RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
        //         //     //     continue;
        //         //     // RimSummaryCase* rimCase = curve->summaryCaseY();
        //         //     // cvf::Color3f    curveColor =
        //         //     //     RimEnsembleWellLogCurveSetColorManager::caseColor( m_legendConfig, rimCase,
        //         ensembleParam
        //         //     );
        //         //     // curve->setColor( curveColor );
        //         //     // curve->updateCurveAppearance();
        //         // }
        //     }
        // }
    }
    else if ( m_colorMode == ColorMode::SINGLE_COLOR )
    {
        for ( auto& curve : m_curves )
        {
            // Statistics curves have separate color settings
            if ( dynamic_cast<RimEnsembleWellLogStatisticsCurve*>( curve.p() ) == nullptr )
            {
                curve->setColor( m_color );
                curve->updateCurveAppearance();
            }
        }
    }

    RimWellLogTrack* plotTrack;
    firstAncestorOrThisOfType( plotTrack );
    if ( plotTrack && plotTrack->viewer() )
    {
        //     if ( m_yValuesSummaryCaseCollection() && isCurvesVisible() && m_colorMode != ColorMode::SINGLE_COLOR &&
        //          m_legendConfig->showLegend() )
        //     {
        //         if ( !m_legendOverlayFrame )
        //         {
        //             m_legendOverlayFrame =
        //                 new RiuDraggableOverlayFrame( plot->viewer()->canvas(), plot->viewer()->overlayMargins() );
        //         }
        //         m_legendOverlayFrame->setContentFrame( m_legendConfig->makeLegendFrame() );
        //         plot->viewer()->addOverlayFrame( m_legendOverlayFrame );
        //     }
        //     else
        //     {
        //         if ( m_legendOverlayFrame )
        //         {
        //             plot->viewer()->removeOverlayFrame( m_legendOverlayFrame );
        //         }
        //     }
        plotTrack->viewer()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateEnsembleCurves( const std::vector<RimWellLogFile*>& sumCases )
{
    RimWellLogTrack* plotTrack = nullptr;
    firstAncestorOrThisOfType( plotTrack );
    CVF_ASSERT( plotTrack );

    RimWellLogPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfType( wellLogPlot );
    CVF_ASSERT( wellLogPlot );

    deleteEnsembleCurves();
    // m_qwtPlotCurveForLegendText->detach();
    deleteStatisticsCurves();

    if ( m_statistics->hideEnsembleCurves() ) return;

    QString wellLogChannelName = m_wellLogChannelName();
    if ( plotTrack && wellLogChannelName != "None" )
    {
        if ( isCurvesVisible() )
        {
            bool isFirst = true;

            for ( auto& wellLogFile : sumCases )
            {
                RimWellLogFileCurve* curve = new RimWellLogFileCurve;
                plotTrack->addCurve( curve );

                QString errorMessage;
                if ( wellLogFile->readFile( &errorMessage ) )
                {
                    RigWellLogFile* wellLogDataFile = wellLogFile->wellLogFileData();
                    CVF_ASSERT( wellLogDataFile );

                    if ( isFirst )
                    {
                        // Initialize plot with depth unit from the first log file
                        wellLogPlot->setDepthUnit( wellLogDataFile->depthUnit() );
                        isFirst = false;
                    }
                }
                else
                {
                    RiaLogging::warning( errorMessage );
                }

                RimWellPath* wellPath = RimProject::current()->wellPathByName( wellLogFile->wellName() );
                curve->setWellPath( wellPath );
                curve->setWellLogChannelName( wellLogChannelName );
                curve->setWellLogFile( wellLogFile );

                curve->loadDataAndUpdate( true );

                curve->updateCurveVisibility();

                m_curves.push_back( curve );
            }

            updateCurveColors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateStatisticsCurves( const std::vector<RimWellLogFile*>& sumCases )
{
    RimEnsembleWellLogs* ensembleWellLogs   = m_ensembleWellLogs;
    QString              wellLogChannelName = m_wellLogChannelName();

    if ( !isCurvesVisible() || m_disableStatisticCurves || !ensembleWellLogs || wellLogChannelName == "None" ) return;

    // Calculate
    {
        std::vector<RimWellLogFile*> wellLogFiles = sumCases;
        if ( wellLogFiles.empty() )
        {
            if ( m_statistics->basedOnFilteredCases() )
                wellLogFiles = filterEnsembleCases( ensembleWellLogs->wellLogFiles() );
            else
                wellLogFiles = ensembleWellLogs->wellLogFiles();
        }

        m_ensembleWellLogStatistics->calculate( wellLogFiles, wellLogChannelName );
    }

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    std::vector<RimEnsembleWellLogStatistics::StatisticsType> statisticsTypes;
    if ( m_statistics->isActive() )
    {
        if ( m_statistics->showP10Curve() && m_ensembleWellLogStatistics->hasP10Data() )
            statisticsTypes.push_back( RimEnsembleWellLogStatistics::StatisticsType::P10 );
        if ( m_statistics->showP50Curve() && m_ensembleWellLogStatistics->hasP50Data() )
            statisticsTypes.push_back( RimEnsembleWellLogStatistics::StatisticsType::P50 );
        if ( m_statistics->showP90Curve() && m_ensembleWellLogStatistics->hasP90Data() )
            statisticsTypes.push_back( RimEnsembleWellLogStatistics::StatisticsType::P90 );
        if ( m_statistics->showMeanCurve() && m_ensembleWellLogStatistics->hasMeanData() )
            statisticsTypes.push_back( RimEnsembleWellLogStatistics::StatisticsType::MEAN );
    }

    auto statisticsCurveSymbolFromStatistics = []( RimEnsembleWellLogStatistics::StatisticsType statisticsType ) {
        if ( statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P10 ) return RiuQwtSymbol::SYMBOL_TRIANGLE;
        if ( statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P90 )
            return RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE;
        if ( statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P50 ) return RiuQwtSymbol::SYMBOL_DIAMOND;
        return RiuQwtSymbol::SYMBOL_ELLIPSE;
    };

    RimWellLogTrack* plotTrack = nullptr;
    firstAncestorOrThisOfType( plotTrack );
    CVF_ASSERT( plotTrack );

    deleteStatisticsCurves();
    for ( auto statisticsType : statisticsTypes )
    {
        auto curve = new RimEnsembleWellLogStatisticsCurve();
        curve->setEnsembleWellLogCurveSet( this );
        curve->setStatisticsType( statisticsType );

        m_curves.push_back( curve );
        curve->setColor( m_statistics->color() );

        auto symbol = statisticsCurveSymbolFromStatistics( statisticsType );
        curve->setSymbol( symbol );
        curve->setSymbolSize( statisticsCurveSymbolSize( symbol ) );
        curve->setSymbolSkipDistance( 150 );
        if ( m_statistics->showCurveLabels() )
        {
            curve->setSymbolLabel( caf::AppEnum<RimEnsembleWellLogStatistics::StatisticsType>::uiText( statisticsType ) );
        }
        curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );

        plotTrack->addCurve( curve );

        curve->updateCurveVisibility();
        curve->loadDataAndUpdate( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateStatisticsCurves()
{
    updateStatisticsCurves( std::vector<RimWellLogFile*>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogCurveSet* RimEnsembleWellLogCurveSet::clone() const
{
    RimEnsembleWellLogCurveSet* copy = dynamic_cast<RimEnsembleWellLogCurveSet*>(
        this->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    return copy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::showCurves( bool show )
{
    m_showCurves = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateAllTextInPlot()
{
    updateEnsembleLegendItem();

    // RimWellLogTrack* summaryPlot = nullptr;
    // this->firstAncestorOrThisOfTypeAsserted( summaryPlot );
    // if ( summaryPlot->viewer() )
    // {
    //     summaryPlot->u
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEnsembleParameter> RimEnsembleWellLogCurveSet::variationSortedEnsembleParameters() const
{
    // RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    // if ( ensemble )
    // {
    //     return ensemble->variationSortedEnsembleParameters();
    // }
    // else
    // {
    return std::vector<RigEnsembleParameter>();
    //}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>> RimEnsembleWellLogCurveSet::correlationSortedEnsembleParameters() const
{
    // RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    // if ( ensemble )
    // {
    //     return ensemble->correlationSortedEnsembleParameters( summaryAddress() );
    // }
    // else
    // {
    return std::vector<std::pair<RigEnsembleParameter, double>>();
    //}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*>
    RimEnsembleWellLogCurveSet::filterEnsembleCases( const std::vector<RimWellLogFile*>& wellLogFiles )
{
    std::vector<RimWellLogFile*> filteredCases;

    if ( m_ensembleCurveSet != nullptr && m_statistics->basedOnFilteredCases() )
    { // && m_ensembleCurveSet->isFiltered() )
        RiaLogging::debug( QString( "FILTERING ENSEMBLE CASES" ) );

        // Get the summary cases from the related ensemble summary curve set.
        RimSummaryCaseCollection* summaryCaseCollection = m_ensembleCurveSet->summaryCaseCollection();

        //
        std::vector<RimSummaryCase*> sumCases =
            m_ensembleCurveSet->filterEnsembleCases( summaryCaseCollection->allSummaryCases() );
        for ( auto sumCase : sumCases )
        {
            for ( auto wellLogFile : wellLogFiles )
            {
                if ( isSameRealization( sumCase, wellLogFile ) )
                {
                    filteredCases.push_back( wellLogFile );
                }
            }
        }
    }
    else
    {
        RiaLogging::debug( QString( "NOT FILTERING ENSEMBLE CASES" ) );

        filteredCases = wellLogFiles;
    }

    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::isSameRealization( RimSummaryCase* summaryCase, RimWellLogFile* wellLogFile ) const
{
    QString wellLogFileName = wellLogFile->fileName();

    //
    if ( summaryCase->hasCaseRealizationParameters() )
    {
        // TODO: make less naive..
        int     realizationNumber   = summaryCase->caseRealizationParameters()->realizationNumber();
        QString summaryCaseFileName = summaryCase->summaryHeaderFilename();

        if ( wellLogFileName.contains( QString( "realization-%1" ).arg( realizationNumber ) ) )
        {
            RiaLogging::debug(
                QString( "Matching summary case %1 with well log file %2" ).arg( summaryCaseFileName ).arg( wellLogFileName ) );

            return true;
        }
    }

    RiaLogging::debug( QString( "No matching summary case found for well log file: %1." ).arg( wellLogFileName ) );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::disableStatisticCurves()
{
    m_disableStatisticCurves = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::isFiltered() const
{
    return m_isCurveSetFiltered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::hasP10Data() const
{
    return m_ensembleWellLogStatistics->hasP10Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::hasP50Data() const
{
    return m_ensembleWellLogStatistics->hasP50Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::hasP90Data() const
{
    return m_ensembleWellLogStatistics->hasP90Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogCurveSet::hasMeanData() const
{
    return m_ensembleWellLogStatistics->hasMeanData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEnsembleWellLogStatistics* RimEnsembleWellLogCurveSet::ensembleWellLogStatistics() const
{
    return m_ensembleWellLogStatistics.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateEnsembleLegendItem()
{
    m_qwtPlotCurveForLegendText->setTitle( name() );

    {
        QwtSymbol* symbol = nullptr;

        if ( m_colorMode == ColorMode::SINGLE_COLOR )
        {
            symbol = new QwtSymbol( QwtSymbol::HLine );

            QColor curveColor( m_color.value().rByte(), m_color.value().gByte(), m_color.value().bByte() );
            QPen   curvePen( curveColor );
            curvePen.setWidth( 2 );

            symbol->setPen( curvePen );
            symbol->setSize( 6, 6 );
        }
        else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
        {
            QPixmap p = QPixmap( ":/Legend.png" );

            symbol = new QwtSymbol;
            symbol->setPixmap( p );
            symbol->setSize( 8, 8 );
        }

        m_qwtPlotCurveForLegendText->setSymbol( symbol );
    }

    bool showLegendItem = isCurvesVisible();
    m_qwtPlotCurveForLegendText->setItemAttribute( QwtPlotItem::Legend, showLegendItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleWellLogCurveSet::name() const
{
    QString curveSetName;
    if ( m_isUsingAutoName )
    {
        curveSetName = m_autoGeneratedName();
    }
    else
    {
        curveSetName += m_userDefinedName();
    }

    return curveSetName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleWellLogCurveSet::createAutoName() const
{
    QStringList nameParts;

    if ( m_ensembleWellLogs() )
    {
        nameParts.append( m_ensembleWellLogs->name() );
        nameParts.append( m_wellLogChannelName() );
    }

    if ( !nameParts.isEmpty() )
        return nameParts.join( " - " );
    else
        return "Ensemble Well Log Curve Set";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogCurveSet::updateLegendMappingMode()
{
    switch ( currentEnsembleParameterType() )
    {
        case RigEnsembleParameter::TYPE_TEXT:
            if ( m_legendConfig->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
            break;

        case RigEnsembleParameter::TYPE_NUMERIC:
            if ( m_legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
            break;
    }
}
