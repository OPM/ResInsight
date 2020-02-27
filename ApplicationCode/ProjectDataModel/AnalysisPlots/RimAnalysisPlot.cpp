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

#include "RimAnalysisPlot.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiuGroupedBarChartBuilder.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "RifSummaryReaderInterface.h"

#include "RimAnalysisPlotDataEntry.h"
#include "RimDerivedSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot_barchart.h"
#include "qwt_scale_draw.h"

#include "cafPdmUiActionPushButtonEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiListEditor.h"

#include <limits>
#include <map>

class RimCurveDefinitionSplitter
{
public:
    RimCurveDefinitionSplitter( std::vector<RiaSummaryCurveDefinition> curveDefs )
    {
        for ( const auto& curveDef : curveDefs )
        {
            if ( curveDef.isEnsembleCurve() )
            {
                m_ensembles.insert( curveDef.ensemble() );
            }
            else
            {
                m_singleSummaryCases.insert( curveDef.summaryCase() );
            }

            RifEclipseSummaryAddress address = curveDef.summaryAddress();

            m_quantityNames.insert( address.quantityName() );

            address.setQuantityName( "" );
            m_summaryItems.insert( address );
        }
    }

    std::set<RimSummaryCase*>           m_singleSummaryCases;
    std::set<RimSummaryCaseCollection*> m_ensembles;

    std::set<RifEclipseSummaryAddress> m_summaryItems; // Quantity name set to "", stores only the identifiers
    std::set<std::string>              m_quantityNames; // Quantity names from the addresses
};

namespace caf
{
template <>
void caf::AppEnum<RimAnalysisPlot::SortGroupType>::setUp()
{
    addItem( RimAnalysisPlot::NONE, "NONE", "None" );
    addItem( RimAnalysisPlot::SUMMARY_ITEM, "SUMMARY_ITEM", "Summary Item" );
    addItem( RimAnalysisPlot::QUANTITY, "QUANTITY", "Quantity" );
    addItem( RimAnalysisPlot::CASE, "CASE", "Case" );
    addItem( RimAnalysisPlot::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimAnalysisPlot::VALUE, "VALUE", "Value" );
    addItem( RimAnalysisPlot::ABS_VALUE, "ABS_VALUE", "abs(Value)" );
    addItem( RimAnalysisPlot::OTHER_VALUE, "OTHER_VALUE", "Other Value" );
    addItem( RimAnalysisPlot::ABS_OTHER_VALUE, "ABS_OTHER_VALUE", "abs(Other Value)" );
    addItem( RimAnalysisPlot::TIME_STEP, "TIME_STEP", "Time Step" );
    setDefault( RimAnalysisPlot::NONE );
}

template <>
void caf::AppEnum<RimAnalysisPlot::BarOrientation>::setUp()
{
    addItem( RimAnalysisPlot::BARS_HORIZONTAL, "BARS_HORIZONTAL", "Horizontal" );
    addItem( RimAnalysisPlot::BARS_VERTICAL, "BARS_VERTICAL", "Vertical" );
    setDefault( RimAnalysisPlot::BARS_VERTICAL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimAnalysisPlot, "AnalysisPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::RimAnalysisPlot()
    : RimPlot()
{
    CAF_PDM_InitObject( "Analysis Plot", ":/Histogram16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Title", "", "", "" );
    m_showPlotTitle.xmlCapability()->setIOWritable( false );
    m_showPlotTitle.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto", "", "", "" );
    m_useAutoPlotTitle.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "Analysis Plot" ), "Title", "", "", "" );
    m_description.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_barOrientation, "BarOrientation", "Bar Orientation", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_selectedVarsUiField, "selectedVarsUiField", "Selected Variables", "", "", "" );
    m_selectedVarsUiField.xmlCapability()->disableIO();
    m_selectedVarsUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedVarsUiField.uiCapability()->setUiReadOnly( true );

    // CAF_PDM_InitField( &m_selectVariablesButtonField, "BrowseButton", false, "...", ":/Histogram16x16.png", "", "" );
    CAF_PDM_InitField( &m_selectVariablesButtonField, "BrowseButton", false, "...", "", "", "" );
    caf::PdmUiActionPushButtonEditor::configureEditorForField( &m_selectVariablesButtonField );

    CAF_PDM_InitFieldNoDefault( &m_data, "AnalysisPlotData", "", "", "", "" );
    m_data.uiCapability()->setUiTreeChildrenHidden( true );
    m_data.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_addTimestepUiField, "AddTimeStepsUiField", "Add Timestep:", "", "", "" );
    m_addTimestepUiField.xmlCapability()->disableIO();
    m_addTimestepUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_referenceCase, "ReferenceCase", "Reference Case", "", "", "" );

    CAF_PDM_InitField( &m_useTopBarsFilter, "UseTopBarsFilter", false, "Show Only Top", "", "", "" );
    m_useTopBarsFilter.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_maxBarCount, "MaxBarCount", 20, "Bar Count", "", "", "" );
    m_maxBarCount.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_majorGroupType, "MajorGroupType", "Major Grouping", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_mediumGroupType, "MediumGroupType", "Medium Grouping", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_minorGroupType, "MinorGroupType", "Minor Grouping", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_valueSortOperation, "ValueSortOperation", "Sort by Value", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_sortGroupForLegend, "groupForLegend", "Legend Using", "", "", "" );

    CAF_PDM_InitField( &m_useBarText, "UseBarText", true, "Activate Bar Labels", "", "", "" );
    m_useBarText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useCaseInBarText, "UseCaseInBarText", false, "Case Name", "", "", "" );
    CAF_PDM_InitField( &m_useEnsembleInBarText, "UseEnsembleInBarText", false, "Ensemble", "", "", "" );
    CAF_PDM_InitField( &m_useSummaryItemInBarText, "UseSummaryItemInBarText", false, "Summary Item", "", "", "" );
    CAF_PDM_InitField( &m_useTimeStepInBarText, "UseTimeStepInBarText", false, "Time Step", "", "", "" );
    CAF_PDM_InitField( &m_useQuantityInBarText, "UseQuantityInBarText", false, "Quantity", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::~RimAnalysisPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::cleanupBeforeClose()
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
bool RimAnalysisPlot::showPlotTitle() const
{
    return m_showPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setShowPlotTitle( bool showTitle )
{
    m_showPlotTitle = showTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::reattachAllCurves()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateAxes()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimAnalysisPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAnalysisPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::asciiDataForPlotExport() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateLegend()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnalysisPlot::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnalysisPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                     int                         oldFontSize,
                                     int                         fontSize,
                                     bool                        forceChange /*= false */ )
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setAutoScaleXEnabled( bool enabled )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setAutoScaleYEnabled( bool enabled )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateZoomInQwt()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateZoomFromQwt()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimAnalysisPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onAxisSelected( int axis, bool toggle )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateCaseNameHasChanged()
{
    // Todo
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAnalysisPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

        this->connect( m_plotWidget, SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        // updatePlotTitle();
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::doUpdateLayout()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::doRemoveFromCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimAnalysisPlot::snapshotWindowContent()
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
caf::PdmFieldHandle* RimAnalysisPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_selectVariablesButtonField )
    {
        // Do select variables
        RiuSummaryVectorSelectionDialog dlg( nullptr );

        dlg.setCurveSelection( this->curveDefinitions() );
        dlg.enableMultiSelect( true );
        dlg.setCaseAndAddress( nullptr, RifEclipseSummaryAddress() );

        if ( dlg.exec() == QDialog::Accepted )
        {
            std::vector<RiaSummaryCurveDefinition> summaryVectorDefinitions = dlg.curveSelection();
            m_data.deleteAllChildObjects();
            for ( const RiaSummaryCurveDefinition& vectorDef : summaryVectorDefinitions )
            {
                auto plotEntry = new RimAnalysisPlotDataEntry();
                plotEntry->setFromCurveDefinition( vectorDef );
                m_data.push_back( plotEntry );
            }
        }

        m_selectVariablesButtonField = false;
    }
    else if ( changedField == &m_addTimestepUiField )
    {
        m_selectedTimeSteps.v().push_back( m_addTimestepUiField() );
        m_addTimestepUiField = QDateTime();
    }

    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* selVectorsGrp = uiOrdering.addNewGroup( "Selected Vectors" );
    selVectorsGrp->add( &m_selectedVarsUiField );
    selVectorsGrp->add( &m_selectVariablesButtonField, {false} );

    QString vectorNames;
    if ( m_curveDefSplitter )
    {
        for ( const std::string& quantityName : m_curveDefSplitter->m_quantityNames )
        {
            vectorNames += QString::fromStdString( quantityName ) + ", ";
        }

        if ( !vectorNames.isEmpty() )
        {
            vectorNames.chop( 2 );
        }
    }

    m_selectedVarsUiField = vectorNames;

    caf::PdmUiGroup* timeStepGrp = uiOrdering.addNewGroup( "Time Steps" );
    timeStepGrp->add( &m_addTimestepUiField );
    timeStepGrp->add( &m_selectedTimeSteps );

    uiOrdering.add( &m_referenceCase, {true, 3, 2} );

    uiOrdering.add( &m_showPlotTitle );
    uiOrdering.add( &m_useAutoPlotTitle, {false} );
    uiOrdering.add( &m_description, {false} );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );

    uiOrdering.add( &m_barOrientation, {true, 3, 2} );

    caf::PdmUiGroup* sortGrp = uiOrdering.addNewGroup( "Sorting and Grouping" );
    sortGrp->add( &m_majorGroupType );
    sortGrp->add( &m_mediumGroupType );
    sortGrp->add( &m_minorGroupType );
    sortGrp->add( &m_valueSortOperation );
    sortGrp->add( &m_useTopBarsFilter );
    sortGrp->add( &m_maxBarCount, {false} );
    m_maxBarCount.uiCapability()->setUiReadOnly( !m_useTopBarsFilter() );

    caf::PdmUiGroup* legendGrp = uiOrdering.addNewGroup( "Legend" );
    legendGrp->add( &m_showPlotLegends );
    legendGrp->add( &m_sortGroupForLegend );
    legendGrp->add( &m_legendFontSize );
    m_sortGroupForLegend.uiCapability()->setUiReadOnly( !m_showPlotLegends() );
    m_legendFontSize.uiCapability()->setUiReadOnly( !m_showPlotLegends() );

    caf::PdmUiGroup* barLabelGrp = uiOrdering.addNewGroup( "Bar Labels" );
    barLabelGrp->add( &m_useBarText );
    barLabelGrp->add( &m_useQuantityInBarText );
    barLabelGrp->add( &m_useSummaryItemInBarText, {false} );
    barLabelGrp->add( &m_useCaseInBarText );
    barLabelGrp->add( &m_useEnsembleInBarText, {false} );
    barLabelGrp->add( &m_useTimeStepInBarText, {true, 4, 1} );

    m_useQuantityInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useSummaryItemInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useCaseInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useEnsembleInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useTimeStepInBarText.uiCapability()->setUiReadOnly( !m_useBarText );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAnalysisPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( !options.isEmpty() ) return options;

    if ( !m_curveDefSplitter )
    {
        m_curveDefSplitter =
            std::unique_ptr<RimCurveDefinitionSplitter>( new RimCurveDefinitionSplitter( this->curveDefinitions() ) );
    }

    if ( fieldNeedingOptions == &m_addTimestepUiField )
    {
        options.push_back( {"None", QDateTime()} );

        std::set<QDateTime> timeStepUnion;

        std::set<RimSummaryCase*> timeStepDefiningSumCases = m_curveDefSplitter->m_singleSummaryCases;
        for ( RimSummaryCaseCollection* sumCaseColl : m_curveDefSplitter->m_ensembles )
        {
            std::vector<RimSummaryCase*> sumCases = sumCaseColl->allSummaryCases();
            if ( sumCases.size() )
            {
                timeStepDefiningSumCases.insert( sumCases[0] );
            }
        }

        for ( RimSummaryCase* sumCase : timeStepDefiningSumCases )
        {
            const std::vector<time_t>& timeSteps = sumCase->summaryReader()->timeSteps( RifEclipseSummaryAddress() );

            for ( time_t t : timeSteps )
            {
                timeStepUnion.insert( RiaQDateTimeTools::fromTime_t( t ) );
            }
        }

        for ( const QDateTime& dateTime : timeStepUnion )
        {
            QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {dateTime} );

            options.push_back( {dateTime.toString( formatString ), dateTime} );
        }
    }
    else if ( fieldNeedingOptions == &m_valueSortOperation )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( NONE ), NONE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( VALUE ), VALUE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( ABS_VALUE ), ABS_VALUE ) );
    }
    else if ( fieldNeedingOptions == &m_majorGroupType || fieldNeedingOptions == &m_mediumGroupType ||
              fieldNeedingOptions == &m_minorGroupType || fieldNeedingOptions == &m_sortGroupForLegend )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( NONE ), NONE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SUMMARY_ITEM ), SUMMARY_ITEM ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( QUANTITY ), QUANTITY ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( CASE ), CASE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( ENSEMBLE ), ENSEMBLE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( TIME_STEP ), TIME_STEP ) );
    }
    else if ( fieldNeedingOptions == &m_referenceCase )
    {
        std::vector<RimSummaryCase*> allSummaryCases = RiaApplication::instance()->project()->allSummaryCases();

        options.push_back( {"None", nullptr} );

        for ( auto sumCase : allSummaryCases )
        {
            QString displayName = sumCase->displayCaseName();
            auto    caseColl    = dynamic_cast<RimSummaryCaseCollection*>( sumCase->parentField()->ownerObject() );
            if ( caseColl )
            {
                displayName = caseColl->name() + "/" + displayName;
            }

            options.push_back( {displayName, sumCase} );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_curveDefSplitter =
        std::unique_ptr<RimCurveDefinitionSplitter>( new RimCurveDefinitionSplitter( this->curveDefinitions() ) );

    if ( m_plotWidget )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;

        // buildTestPlot( chartBuilder );
        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget,
                                        m_barOrientation == BARS_HORIZONTAL ? Qt::Horizontal : Qt::Vertical,
                                        m_useTopBarsFilter() ? m_maxBarCount : -1 );

        if ( m_showPlotLegends && m_plotWidget->legend() == nullptr )
        {
            QwtLegend* legend = new QwtLegend( m_plotWidget );
            m_plotWidget->insertLegend( legend, QwtPlot::RightLegend );
        }
        else if ( !m_showPlotLegends )
        {
            m_plotWidget->insertLegend( nullptr );
        }

        m_plotWidget->setLegendFontSize( m_legendFontSize() );
        m_plotWidget->updateLegend();
    }

    this->updateAxes();
    this->updatePlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString assignGroupingText( RimAnalysisPlot::SortGroupType  sortGroup,
                            const RiaSummaryCurveDefinition dataEntry,
                            const QString&                  timestepString )
{
    QString groupingText;

    switch ( sortGroup )
    {
        case RimAnalysisPlot::SUMMARY_ITEM:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddress();
            groupingText                  = QString::fromStdString( addr.itemUiText() );
        }
        break;
        case RimAnalysisPlot::CASE:
        {
            if ( dataEntry.summaryCase() )
            {
                groupingText = dataEntry.summaryCase()->displayCaseName();
            }
        }
        break;
        case RimAnalysisPlot::ENSEMBLE:
        {
            if ( dataEntry.ensemble() )
            {
                groupingText = dataEntry.ensemble()->name();
            }
        }
        break;
        case RimAnalysisPlot::QUANTITY:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddress();

            groupingText = QString::fromStdString( addr.quantityName() );
        }
        break;
        case RimAnalysisPlot::TIME_STEP:
        {
            groupingText = timestepString;
        }
        break;
        default:
        {
            // Return empty string
        }
        break;
    }

    return groupingText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder )
{
    std::vector<time_t> selectedTimesteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimesteps.push_back( dateTime.toTime_t() );
    }

    RifSummaryReaderInterface* referenceCaseReader = nullptr;

    if ( m_referenceCase ) referenceCaseReader = m_referenceCase->summaryReader();

    for ( const RimAnalysisPlotDataEntry* dataEntry : m_data )
    {
        RiaSummaryCurveDefinition orgBarDataEntry = dataEntry->curveDefinition();

        std::vector<RiaSummaryCurveDefinition> barDataDefinitions;

        if ( orgBarDataEntry.isEnsembleCurve() )
        {
            std::vector<RimSummaryCase*> sumCases = orgBarDataEntry.ensemble()->allSummaryCases();
            for ( auto sumCase : sumCases )
            {
                barDataDefinitions.push_back(
                    RiaSummaryCurveDefinition( sumCase, orgBarDataEntry.summaryAddress(), orgBarDataEntry.ensemble() ) );
            }
        }
        else
        {
            barDataDefinitions.push_back( orgBarDataEntry );
        }

        for ( const RiaSummaryCurveDefinition& curveDef : barDataDefinitions )
        {
            RifSummaryReaderInterface* reader = curveDef.summaryCase()->summaryReader();

            if ( !reader ) continue;

            // Todo:
            // If curveDef.summaryCase() is a RimGridSummaryCase and we are using summary item as legend and the summary
            // items are wells, then:
            /// use color from eclCase->defaultWellColor( wellName );

            std::vector<time_t>        timeStepStorage;
            const std::vector<time_t>* timeStepsPtr = &timeStepStorage;
            std::vector<double>        values;

            if ( referenceCaseReader )
            {
                std::pair<std::vector<time_t>, std::vector<double>> timeAndValues =
                    RimDerivedSummaryCase::calculateDerivedValues( reader,
                                                                   referenceCaseReader,
                                                                   DerivedSummaryOperator::DERIVED_OPERATOR_SUB,
                                                                   curveDef.summaryAddress() );
                timeStepStorage.swap( timeAndValues.first );
                values.swap( timeAndValues.second );
            }
            else
            {
                timeStepsPtr = &( reader->timeSteps( curveDef.summaryAddress() ) );

                reader->values( curveDef.summaryAddress(), &values );
            }

            const std::vector<time_t>& timesteps = *timeStepsPtr;

            // Find selected timestep indices

            std::vector<int> selectedTimestepIndices;

            for ( time_t tt : selectedTimesteps )
            {
                for ( int timestepIdx = 0; static_cast<unsigned>( timestepIdx ) < timesteps.size(); ++timestepIdx )
                {
                    if ( timesteps[timestepIdx] == tt )
                    {
                        selectedTimestepIndices.push_back( timestepIdx );
                        break;
                    }
                }
            }

            for ( int timestepIdx : selectedTimestepIndices )
            {
                double sortValue = std::numeric_limits<double>::infinity();

                QDateTime dateTime     = RiaQDateTimeTools::fromTime_t( timesteps[timestepIdx] );
                QString   formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {dateTime} );

                QString timestepString = dateTime.toString( formatString );

                QString majorText  = assignGroupingText( m_majorGroupType(), curveDef, timestepString );
                QString medText    = assignGroupingText( m_mediumGroupType(), curveDef, timestepString );
                QString minText    = assignGroupingText( m_minorGroupType(), curveDef, timestepString );
                QString legendText = assignGroupingText( m_sortGroupForLegend(), curveDef, timestepString );

                double value = values[timestepIdx];

                switch ( m_valueSortOperation() )
                {
                    case VALUE:
                        sortValue = value;
                        break;
                    case ABS_VALUE:
                        sortValue = fabs( value );
                        break;
                }

                QString barText;
                QString separator = " ";

                if ( m_useBarText() )
                {
                    if ( m_useQuantityInBarText )
                    {
                        barText += QString::fromStdString( curveDef.summaryAddress().quantityName() ) + separator;
                    }

                    if ( m_useSummaryItemInBarText )
                    {
                        barText += QString::fromStdString( curveDef.summaryAddress().itemUiText() ) + separator;
                    }

                    if ( m_useCaseInBarText && curveDef.summaryCase() )
                    {
                        barText += curveDef.summaryCase()->displayCaseName() + separator;
                    }

                    if ( m_useEnsembleInBarText && curveDef.ensemble() )
                    {
                        barText += curveDef.ensemble()->name() + separator;
                    }

                    if ( m_useTimeStepInBarText )
                    {
                        barText += timestepString + separator;
                    }
                }

                chartBuilder.addBarEntry( majorText, medText, minText, sortValue, legendText, barText, value );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder )
{
    chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R1", "", 0.4 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R2", "", 0.45 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 0.5 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 0.55 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R1", "", 0.7 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R2", "", 0.75 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 1.05 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 1.0 );

    chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );

    chartBuilder.addBarEntry( "T3", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T3", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T3", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T3", "W2", "4", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T3", "W2", "5", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );

    chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );
    chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.6 );
    chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.6 );
    chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.6 );
    chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", -0.3 );

    chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G1", 1.5 );
    chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G2", 1.5 );
    chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G3", 2.0 );
    chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G4", 2.0 );
    chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G5", 1.6 );
    chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G6", 1.6 );
    chartBuilder.addBarEntry( "T5", "", "", 2.6, "R3", "G7", 2.6 );
    chartBuilder.addBarEntry( "T5", "", "", -0.1, "R3", "G8", -0.1 );

    chartBuilder.addBarEntry( "", "", "", 1.2, "", "A", 1.2 );
    chartBuilder.addBarEntry( "", "", "", 1.5, "", "B", 1.5 );
    chartBuilder.addBarEntry( "", "", "", 2.3, "", "C", 2.3 );
    chartBuilder.addBarEntry( "", "", "", 2.0, "", "D", 2.0 );
    chartBuilder.addBarEntry( "", "", "", 1.6, "", "E", 1.6 );
    chartBuilder.addBarEntry( "", "", "", 2.4, "", "F", -2.4 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle && m_curveDefSplitter )
    {
        QString autoTitle;
        QString separator = ", ";

        if ( m_curveDefSplitter->m_ensembles.size() == 1 )
        {
            autoTitle += ( *m_curveDefSplitter->m_ensembles.begin() )->name();
        }

        if ( m_curveDefSplitter->m_singleSummaryCases.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += ( *m_curveDefSplitter->m_singleSummaryCases.begin() )->displayCaseName();
        }

        if ( m_curveDefSplitter->m_summaryItems.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += QString::fromStdString( m_curveDefSplitter->m_summaryItems.begin()->itemUiText() );
        }

        for ( std::string quantName : m_curveDefSplitter->m_quantityNames )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += QString::fromStdString( quantName );
        }

        if ( m_referenceCase() )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += "Compared to " + m_referenceCase->displayCaseName();
        }

        if ( m_useTopBarsFilter )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += " - ";
            autoTitle += "Top " + QString::number( m_maxBarCount() );
        }

        if ( m_selectedTimeSteps().size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += " @ ";

            QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {m_selectedTimeSteps()[0]} );
            autoTitle += m_selectedTimeSteps()[0].toString( formatString );
        }

        m_description = autoTitle;
    }

    if ( m_plotWidget )
    {
        QString plotTitle = description();
        m_plotWidget->setPlotTitle( plotTitle );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && isMdiWindow() );
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::curveDefinitions()
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;
    for ( auto dataEntry : m_data )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_useTopBarsFilter || field == &m_useBarText || field == &m_showPlotTitle || field == &m_useAutoPlotTitle )
    {
        auto attrib = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_useNativeCheckBoxLabel = true;
        }
    }
}
