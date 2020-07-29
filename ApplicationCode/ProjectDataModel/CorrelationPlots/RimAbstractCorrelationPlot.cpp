#include "RimAbstractCorrelationPlot.h"

#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimAnalysisPlotDataEntry.h"
#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuQwtPlotWidget.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimAbstractCorrelationPlot, "AbstractCorrelationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAbstractCorrelationPlot::RimAbstractCorrelationPlot()
    : m_selectMultipleVectors( false )
{
    CAF_PDM_InitObject( "Abstract Correlation Plot", ":/CorrelationPlot16x16.png", "", "" );
    this->setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedVarsUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_selectedVarsUiField.xmlCapability()->disableIO();
    m_selectedVarsUiField.uiCapability()->setUiReadOnly( true );
    m_selectedVarsUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlotDataSelection, "AnalysisPlotData", "", "", "", "" );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeChildrenHidden( true );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pushButtonSelectSummaryAddress );
    m_pushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_pushButtonSelectSummaryAddress = false;

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_useAutoPlotTitle, "AutoTitle", true, "Automatic Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_description, "PlotTitle", QString( "Correlation Plot" ), "Custom Plot Title", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_labelFontSize, "LabelFontSize", "Label Font Size", "", "", "" );
    m_labelFontSize = caf::FontTools::RelativeSize::XSmall;

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size", "", "", "" );
    m_axisValueFontSize = caf::FontTools::RelativeSize::XSmall;

    m_legendFontSize = caf::FontTools::RelativeSize::XSmall;
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
    m_analysisPlotDataSelection.deleteAllChildObjects();
    for ( auto curveDef : curveDefinitions )
    {
        auto dataEntry = new RimAnalysisPlotDataEntry();
        dataEntry->setFromCurveDefinition( curveDef );
        m_analysisPlotDataSelection.push_back( dataEntry );
    }
    auto timeSteps = allAvailableTimeSteps();
    if ( m_timeStep().isNull() && !timeSteps.empty() )
    {
        m_timeStep = QDateTime::fromTime_t( *timeSteps.rbegin() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::setTimeStep( std::time_t timeStep )
{
    m_timeStep = QDateTime::fromTime_t( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
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
                m_analysisPlotDataSelection.deleteAllChildObjects();
                for ( const RiaSummaryCurveDefinition& vectorDef : summaryVectorDefinitions )
                {
                    auto plotEntry = new RimAnalysisPlotDataEntry();
                    plotEntry->setFromCurveDefinition( vectorDef );
                    m_analysisPlotDataSelection.push_back( plotEntry );
                }
                this->loadDataAndUpdate();
                this->updateConnectedEditors();
            }
        }

        m_pushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_timeStep )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_showPlotTitle || changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        this->updatePlotTitle();
    }
    else if ( changedField == &m_labelFontSize || changedField == &m_axisTitleFontSize ||
              changedField == &m_axisValueFontSize || changedField == &m_legendFontSize || changedField == &m_titleFontSize )
    {
        this->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAbstractCorrelationPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
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
QList<caf::PdmOptionItemInfo>
    RimAbstractCorrelationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_timeStep )
    {
        std::set<time_t> allTimeSteps = allAvailableTimeSteps();

        std::map<QDate, std::vector<QTime>> timeStepsDateMap;
        for ( time_t timeStep : allTimeSteps )
        {
            QDateTime dateTime = QDateTime::fromTime_t( timeStep );
            timeStepsDateMap[dateTime.date()].push_back( dateTime.time() );
        }

        QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                        RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
        QString timeFormatString =
            RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                 RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

        for ( auto dateTimePair : timeStepsDateMap )
        {
            QDate date          = dateTimePair.first;
            bool  multipleTimes = dateTimePair.second.size() > 1u;
            for ( auto time : dateTimePair.second )
            {
                QString timestepString = date.toString( dateFormatString );
                if ( multipleTimes )
                {
                    timestepString += QString( " %1" ).arg( time.toString( timeFormatString ) );
                }
                options.push_back( caf::PdmOptionItemInfo( timestepString, QDateTime( date, time ) ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_labelFontSize || fieldNeedingOptions == &m_axisTitleFontSize ||
              fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
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
    for ( auto dataEntry : m_analysisPlotDataSelection )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinitionAnalyser* RimAbstractCorrelationPlot::getOrCreateSelectedCurveDefAnalyser()
{
    if ( !m_analyserOfSelectedCurveDefs )
    {
        m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
            new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitions() ) );
    }

    return m_analyserOfSelectedCurveDefs.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimAbstractCorrelationPlot::addresses()
{
    std::set<RifEclipseSummaryAddress> addresses;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        const std::set<RifEclipseSummaryAddress>& caseAddrs = ensemble->ensembleSummaryAddresses();
        for ( auto addr : caseAddrs )
        {
            if ( analyserOfSelectedCurveDefs->m_quantityNames.count( addr.quantityName() ) )
            {
                addresses.insert( addr );
            }
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimAbstractCorrelationPlot::ensembles()
{
    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();
    return analyserOfSelectedCurveDefs->m_ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<EnsembleParameter> RimAbstractCorrelationPlot::ensembleParameters()
{
    std::set<EnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        std::vector<EnsembleParameter> parameters = ensemble->alphabeticEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }
    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<EnsembleParameter> RimAbstractCorrelationPlot::variationSortedEnsembleParameters()
{
    std::set<EnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

    for ( RimSummaryCaseCollection* ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        std::vector<EnsembleParameter> parameters = ensemble->variationSortedEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }
    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimAbstractCorrelationPlot::ensembleParameter( const QString& ensembleParameterName )
{
    std::set<EnsembleParameter> ensembleParms = ensembleParameters();
    for ( const EnsembleParameter& eParam : ensembleParms )
    {
        if ( eParam.name == ensembleParameterName ) return eParam;
    }

    return EnsembleParameter();
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
RiuQwtPlotWidget* RimAbstractCorrelationPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
        updatePlotTitle();
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
void RimAbstractCorrelationPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->detachItems();
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
                                                                    RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
    QString timeFormatString =
        RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                             RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

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
        m_plotWidget->insertLegend( nullptr );
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
QString RimAbstractCorrelationPlot::selectedVarsText() const
{
    QString vectorNames;
    if ( m_analyserOfSelectedCurveDefs )
    {
        for ( const std::string& quantityName : m_analyserOfSelectedCurveDefs->m_quantityNames )
        {
            vectorNames += QString::fromStdString( quantityName ) + ", ";
        }

        if ( !vectorNames.isEmpty() )
        {
            vectorNames.chop( 2 );
        }
    }

    return vectorNames;
}
