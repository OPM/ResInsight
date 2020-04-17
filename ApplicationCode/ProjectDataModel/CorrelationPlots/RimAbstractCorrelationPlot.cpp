#include "RimAbstractCorrelationPlot.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifSummaryReaderInterface.h"

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
{
    CAF_PDM_InitObject( "Abstract Correlation Plot", ":/CorrelationPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddressUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_summaryAddressUiField.xmlCapability()->disableIO();
    m_summaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address", "", "", "" );
    m_summaryAddress.uiCapability()->setUiHidden( true );
    m_summaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pushButtonSelectSummaryAddress );
    m_pushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_pushButtonSelectSummaryAddress = false;

    m_summaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_useAutoPlotTitle, "AutoTitle", true, "Automatic Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_description, "PlotTitle", QString( "Correlation Plot" ), "Custom Plot Title", "", "", "" );
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
void RimAbstractCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_pushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimSummaryCaseCollection*       candidateEnsemble = m_ensemble;
        RifEclipseSummaryAddress        candicateAddress  = m_summaryAddress->address();

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddress( candidateEnsemble, candicateAddress );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_summaryAddress->setAddress( curveSelection[0].summaryAddress() );

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
    else if ( changedField == &m_ensemble )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_showPlotTitle || changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        this->updatePlotTitle();
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

    if ( fieldNeedingOptions == &m_ensemble )
    {
        RimProject*                            project = RiaApplication::instance()->project();
        std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
        project->descendantsIncludingThisOfType( summaryCaseCollections );
        for ( auto summaryCaseCollection : summaryCaseCollections )
        {
            if ( summaryCaseCollection->isEnsemble() )
            {
                options.push_back( caf::PdmOptionItemInfo( summaryCaseCollection->name(), summaryCaseCollection ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_summaryAddressUiField )
    {
        if ( m_ensemble )
        {
            RimEnsembleCurveSet::appendOptionItemsForSummaryAddresses( &options, m_ensemble );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        QString dateTimeFormat = RiaApplication::instance()->preferences()->dateTimeFormat();
        if ( m_ensemble )
        {
            std::set<time_t> allTimeSteps = allAvailableTimeSteps();
            for ( time_t timeStep : allTimeSteps )
            {
                QDateTime dateTime       = QDateTime::fromTime_t( timeStep );
                QString   timestepString = dateTime.toString( Qt::ISODate );
                options.push_back( caf::PdmOptionItemInfo( timestepString, dateTime ) );
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

    for ( RimSummaryCase* sumCase : m_ensemble->allSummaryCases() )
    {
        const std::vector<time_t>& timeSteps = sumCase->summaryReader()->timeSteps( m_summaryAddress->address() );

        for ( time_t t : timeSteps )
        {
            timeStepUnion.insert( t );
        }
    }

    return timeStepUnion;
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
bool RimAbstractCorrelationPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                                int                         oldFontSize,
                                                int                         fontSize,
                                                bool                        forceChange /*= false */ )
{
    bool anyChange = false;

    return anyChange;
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
        // updatePlotTitle();
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
