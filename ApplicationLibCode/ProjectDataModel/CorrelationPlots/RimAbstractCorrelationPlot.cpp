/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimAbstractCorrelationPlot.h"

#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimAnalysisPlotDataEntry.h"
#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"

#include "qwt_plot.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimAbstractCorrelationPlot, "AbstractCorrelationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAbstractCorrelationPlot::RimAbstractCorrelationPlot()
    : m_selectMultipleVectors( false )
{
    CAF_PDM_InitObject( "Abstract Correlation Plot", ":/CorrelationPlot16x16.png" );
    setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedVarsUiField, "SelectedVariableDisplayVar", "Vector" );
    m_selectedVarsUiField.xmlCapability()->disableIO();
    m_selectedVarsUiField.uiCapability()->setUiReadOnly( true );
    m_selectedVarsUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_dataSources, "AnalysisPlotData", "" );
    m_dataSources.uiCapability()->setUiTreeChildrenHidden( true );
    m_dataSources.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonSelectSummaryAddress, "SelectAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pushButtonSelectSummaryAddress );
    m_pushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_pushButtonSelectSummaryAddress = false;

    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Available Time Steps" );

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "AutoTitle", true, "Automatic Plot Title" );
    CAF_PDM_InitField( &m_description, "PlotTitle", QString( "Correlation Plot" ), "Custom Plot Title" );

    CAF_PDM_InitFieldNoDefault( &m_labelFontSize, "LabelFontSize", "Label Font Size" );
    m_labelFontSize = caf::FontTools::RelativeSize::XSmall;

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );
    m_axisValueFontSize = caf::FontTools::RelativeSize::XSmall;

    m_legendFontSize = caf::FontTools::RelativeSize::XSmall;

    CAF_PDM_InitField( &m_useCaseFilter, "UseCaseFilter", false, "Use Ensemble Filter" );
    CAF_PDM_InitFieldNoDefault( &m_curveSetForFiltering, "CurveSetForFiltering", "  Ensemble Filter" );
    CAF_PDM_InitField( &m_editCaseFilter, "EditCaseFilter", false, "Edit" );
    m_editCaseFilter.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_editCaseFilter.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_analyserOfSelectedCurveDefs = std::make_unique<RiaSummaryCurveDefinitionAnalyser>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAbstractCorrelationPlot::~RimAbstractCorrelationPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions )
{
    m_dataSources.deleteChildren();
    for ( auto curveDef : curveDefinitions )
    {
        auto dataEntry = new RimAnalysisPlotDataEntry();
        dataEntry->setFromCurveDefinition( curveDef );
        m_dataSources.push_back( dataEntry );
    }
    connectAllCaseSignals();

    auto timeSteps = allAvailableTimeSteps();
    if ( m_timeStep().isNull() && !timeSteps.empty() )
    {
        m_timeStep = RiaQDateTimeTools::fromTime_t( *timeSteps.rbegin() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setTimeStep( std::time_t timeStep )
{
    m_timeStep = RiaQDateTimeTools::fromTime_t( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_pushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );

        if ( m_selectMultipleVectors )
        {
            dlg.enableMultiSelect( true );
        }

        dlg.hideSummaryCases();
        dlg.setCurveSelection( curveDefinitions() );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                std::vector<RiaSummaryCurveDefinition> summaryVectorDefinitions = dlg.curveSelection();
                m_dataSources.deleteChildren();
                for ( const RiaSummaryCurveDefinition& vectorDef : summaryVectorDefinitions )
                {
                    auto plotEntry = new RimAnalysisPlotDataEntry();
                    plotEntry->setFromCurveDefinition( vectorDef );
                    m_dataSources.push_back( plotEntry );
                }
                connectAllCaseSignals();
                loadDataAndUpdate();
                updateConnectedEditors();
            }
        }

        m_pushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_timeStep )
    {
        loadDataAndUpdate();
        updateConnectedEditors();
    }
    else if ( changedField == &m_showPlotTitle || changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        updatePlotTitle();
    }
    else if ( changedField == &m_labelFontSize || changedField == &m_axisTitleFontSize || changedField == &m_axisValueFontSize ||
              changedField == &m_legendFontSize || changedField == &m_titleFontSize )
    {
        loadDataAndUpdate();
    }
    else if ( changedField == &m_timeStepFilter )
    {
        std::vector<QDateTime> allDateTimes;
        for ( time_t timeStep : allAvailableTimeSteps() )
        {
            QDateTime dateTime = RiaQDateTimeTools::fromTime_t( timeStep );
            allDateTimes.push_back( dateTime );
        }

        std::vector<int> filteredTimeStepIndices =
            RimTimeStepFilter::filteredTimeStepIndices( allDateTimes, 0, (int)allDateTimes.size() - 1, m_timeStepFilter(), 1 );

        m_timeStep = allDateTimes[filteredTimeStepIndices.back()];

        updateConnectedEditors();
    }
    else if ( changedField == &m_curveSetForFiltering )
    {
        connectCurveFilterSignals();

        loadDataAndUpdate();
    }
    else if ( changedField == &m_useCaseFilter )
    {
        loadDataAndUpdate();
    }
    else if ( changedField == &m_editCaseFilter )
    {
        m_editCaseFilter = false;
        if ( m_curveSetForFiltering != nullptr )
        {
            auto filterColl    = m_curveSetForFiltering->filterCollection();
            auto activeFilters = filterColl->filters();
            auto firstFilter   = activeFilters.empty() ? nullptr : activeFilters.front();

            if ( firstFilter )
            {
                RiuPlotMainWindowTools::selectAsCurrentItem( firstFilter );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->m_buttonText = "...";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAbstractCorrelationPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAbstractCorrelationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_timeStep )
    {
        std::set<time_t> allTimeSteps = allAvailableTimeSteps();
        if ( allTimeSteps.empty() )
        {
            return options;
        }

        std::vector<QDateTime> allDateTimes;
        for ( time_t timeStep : allTimeSteps )
        {
            QDateTime dateTime = RiaQDateTimeTools::fromTime_t( timeStep );
            allDateTimes.push_back( dateTime );
        }

        std::vector<int> filteredTimeStepIndices =
            RimTimeStepFilter::filteredTimeStepIndices( allDateTimes, 0, (int)allDateTimes.size() - 1, m_timeStepFilter(), 1 );

        QString dateFormatString     = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                        RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
        QString timeFormatString     = RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                                        RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
        QString dateTimeFormatString = QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );

        bool showTime = m_timeStepFilter == RimTimeStepFilter::TS_ALL || RimTimeStepFilter::TS_INTERVAL_DAYS;

        for ( auto timeStepIndex : filteredTimeStepIndices )
        {
            QDateTime dateTime = allDateTimes[timeStepIndex];

            if ( showTime && dateTime.time() != QTime( 0, 0, 0 ) )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateTimeFormatString ), dateTime ) );
            }
            else
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateFormatString ), dateTime ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_labelFontSize || fieldNeedingOptions == &m_axisTitleFontSize ||
              fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }
    else if ( fieldNeedingOptions == &m_curveSetForFiltering )
    {
        RimSummaryCaseCollection* ensemble = nullptr;

        for ( auto e : m_dataSources )
        {
            auto ens = e->ensemble();
            if ( ens )
            {
                ensemble = ens;
            }
        }

        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        if ( ensemble )
        {
            std::vector<RimEnsembleCurveSet*> referringObjects = ensemble->objectsWithReferringPtrFieldsOfType<RimEnsembleCurveSet>();

            for ( auto object : referringObjects )
            {
                auto nameFiled = object->name();

                options.push_back( caf::PdmOptionItemInfo( nameFiled, object ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimAbstractCorrelationPlot::allAvailableTimeSteps()
{
    std::set<time_t> timeStepUnion;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();
    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        const std::set<time_t>& timeSteps = ensemble->ensembleTimeSteps();
        timeStepUnion.insert( timeSteps.begin(), timeSteps.end() );
    }

    return timeStepUnion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAbstractCorrelationPlot::curveDefinitions() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;
    for ( auto dataEntry : m_dataSources )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinitionAnalyser* RimAbstractCorrelationPlot::getOrCreateSelectedCurveDefAnalyser() const
{
    m_analyserOfSelectedCurveDefs->setCurveDefinitions( curveDefinitions() );
    return m_analyserOfSelectedCurveDefs.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimAbstractCorrelationPlot::addresses() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    for ( auto dataEntry : m_dataSources )
    {
        addresses.insert( dataEntry->summaryAddress() );
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimAbstractCorrelationPlot::ensembles() const
{
    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();
    return analyserOfSelectedCurveDefs->m_ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimAbstractCorrelationPlot::filterEnsembleCases( RimSummaryCaseCollection* ensemble ) const
{
    std::set<RimSummaryCase*> setOfCases;

    if ( ensemble )
    {
        std::vector<RimSummaryCase*> summaryCasesVector;

        if ( m_useCaseFilter() && m_curveSetForFiltering() && m_curveSetForFiltering->summaryCaseCollection() == ensemble )
        {
            summaryCasesVector = m_curveSetForFiltering->filterEnsembleCases( ensemble->allSummaryCases() );
        }
        else
        {
            summaryCasesVector = ensemble->allSummaryCases();
        }

        setOfCases.insert( summaryCasesVector.begin(), summaryCasesVector.end() );
    }

    return setOfCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAbstractCorrelationPlot::isCaseFilterEnabled() const
{
    return m_useCaseFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::enableCaseFilter( bool enable )
{
    m_useCaseFilter = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimAbstractCorrelationPlot::caseFilterDataSource() const
{
    return m_curveSetForFiltering();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setCaseFilterDataSource( RimEnsembleCurveSet* ensemble )
{
    m_curveSetForFiltering = ensemble;
    connectCurveFilterSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigEnsembleParameter> RimAbstractCorrelationPlot::ensembleParameters()
{
    std::set<RigEnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        std::vector<RigEnsembleParameter> parameters = ensemble->alphabeticEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }
    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigEnsembleParameter> RimAbstractCorrelationPlot::variationSortedEnsembleParameters()
{
    std::set<RigEnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        std::vector<RigEnsembleParameter> parameters = ensemble->variationSortedEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }
    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter RimAbstractCorrelationPlot::ensembleParameter( const QString& ensembleParameterName )
{
    std::set<RigEnsembleParameter> ensembleParms = ensembleParameters();
    for ( const RigEnsembleParameter& eParam : ensembleParms )
    {
        if ( eParam.name == ensembleParameterName ) return eParam;
    }

    return RigEnsembleParameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimAbstractCorrelationPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimAbstractCorrelationPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAbstractCorrelationPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimAbstractCorrelationPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
        updatePlotTitle();

        new RiuContextMenuLauncher( m_plotWidget, { "RicShowPlotDataFeature" } );
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAbstractCorrelationPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimAbstractCorrelationPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->qwtPlot()->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimAbstractCorrelationPlot::timeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAbstractCorrelationPlot::timeStepString() const
{
    QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                    RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
    QString timeFormatString = RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                                    RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

    return timeStep().toString( dateFormatString ) + " " + timeStep().toString( timeFormatString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAbstractCorrelationPlot::labelFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_labelFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAbstractCorrelationPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAbstractCorrelationPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setLabelFontSize( caf::FontTools::RelativeSize fontSize )
{
    m_labelFontSize = fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setAxisTitleFontSize( caf::FontTools::RelativeSize fontSize )
{
    m_axisTitleFontSize = fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setAxisValueFontSize( caf::FontTools::RelativeSize fontSize )
{
    m_axisValueFontSize = fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::updateLegend()
{
    if ( m_plotWidget )
    {
        m_plotWidget->qwtPlot()->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RimAbstractCorrelationPlot::timeDiff( time_t lhs, time_t rhs )
{
    if ( lhs >= rhs )
    {
        return lhs - rhs;
    }
    return rhs - lhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAbstractCorrelationPlot::selectedVectorNamesText()
{
    QString vectorNames;
    for ( const std::string& quantityName : getOrCreateSelectedCurveDefAnalyser()->m_vectorNames )
    {
        vectorNames += QString::fromStdString( quantityName ) + ", ";
    }

    if ( !vectorNames.isEmpty() )
    {
        vectorNames.chop( 2 );
    }

    return vectorNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAbstractCorrelationPlot::completeAddressText()
{
    QString vectorName;
    if ( getOrCreateSelectedCurveDefAnalyser()->m_summaryAdresses.size() == 1 )
    {
        auto firstItem = getOrCreateSelectedCurveDefAnalyser()->m_summaryAdresses.begin();
        vectorName     = QString::fromStdString( firstItem->uiText() );
    }
    else
    {
        vectorName = m_selectedVarsUiField;
    }

    return vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::initAfterRead()
{
    connectAllCaseSignals();
    connectCurveFilterSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::appendDataSourceFields( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );

    m_selectedVarsUiField = selectedVectorNamesText();

    curveDataGroup->add( &m_selectedVarsUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, { false, 1, 0 } );
    curveDataGroup->add( &m_timeStepFilter );
    curveDataGroup->add( &m_timeStep );
    curveDataGroup->add( &m_useCaseFilter );
    curveDataGroup->add( &m_curveSetForFiltering );
    m_curveSetForFiltering.uiCapability()->setUiHidden( !m_useCaseFilter() );
    curveDataGroup->add( &m_editCaseFilter, { false, 1, 0 } );
    m_editCaseFilter.uiCapability()->setUiHidden( !m_useCaseFilter() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::onCaseRemoved( const SignalEmitter* emitter, RimSummaryCase* summaryCase )
{
    loadDataAndUpdate();
    if ( m_plotWidget ) m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::connectAllCaseSignals()
{
    for ( auto dataEntry : m_dataSources )
    {
        if ( dataEntry->ensemble() )
        {
            dataEntry->ensemble()->caseRemoved.connect( this, &RimAbstractCorrelationPlot::onCaseRemoved );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::connectCurveFilterSignals()
{
    if ( m_curveSetForFiltering() )
    {
        m_curveSetForFiltering()->filterChanged.connect( this, &RimAbstractCorrelationPlot::onFilterSourceChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::onFilterSourceChanged( const caf::SignalEmitter* emitter )
{
    if ( m_useCaseFilter() ) loadDataAndUpdate();
}
