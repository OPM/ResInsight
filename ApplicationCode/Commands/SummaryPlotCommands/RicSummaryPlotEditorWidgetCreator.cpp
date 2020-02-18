/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSummaryPlotEditorWidgetCreator.h"

#include "RicSummaryPlotEditorUi.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuSummaryCurveDefinitionKeywords.h"
#include "RiuSummaryVectorSelectionUi.h"
#include "RiuSummaryVectorSelectionWidgetCreator.h"

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeView.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QTreeView>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorWidgetCreator::RicSummaryPlotEditorWidgetCreator( QWidget* parent )
{
    m_parentWidget = parent;

    m_summaryCurveCreator.reset( new RicSummaryPlotEditorUi() );

    this->setPdmObject( m_summaryCurveCreator.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorWidgetCreator::~RicSummaryPlotEditorWidgetCreator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorWidgetCreator::updateFromSummaryPlot( RimSummaryPlot* summaryPlot )
{
    m_summaryCurveCreator->updateFromSummaryPlot( summaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorWidgetCreator::updateFromDefaultSources( const std::vector<caf::PdmObject*> defaultSources )
{
    m_summaryCurveCreator->updateFromSummaryPlot( nullptr, defaultSources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorWidgetCreator::recursivelyConfigureAndUpdateTopLevelUiOrdering( const caf::PdmUiOrdering& topLevelUiOrdering,
                                                                                         const QString& uiConfigName )
{
    const std::vector<caf::PdmUiItem*>& topLevelUiItems = topLevelUiOrdering.uiItems();
    if ( m_summaryCurveCreator->isCloseButtonPressed() )
    {
        m_summaryCurveCreator->clearCloseButton();

        emit signalCloseButtonPressed();
    }

    if ( !m_layout ) return;

    QWidget* addrWidget = m_summaryCurveCreator->addressSelectionWidget( m_parentWidget );

    m_firstRowLayout->addWidget( addrWidget );

    caf::PdmUiGroup* appearanceGroup =
        findGroupByKeyword( topLevelUiItems, RiuSummaryCurveDefinitionKeywords::appearance(), uiConfigName );
    auto appearanceGroupBox = createGroupBoxWithContent( appearanceGroup, uiConfigName );
    m_lowerLeftLayout->insertWidget( 0, appearanceGroupBox );

    caf::PdmUiGroup* nameConfigGroup =
        findGroupByKeyword( topLevelUiItems, RiuSummaryCurveDefinitionKeywords::nameConfig(), uiConfigName );
    auto nameConfigGroupBox = createGroupBoxWithContent( nameConfigGroup, uiConfigName );
    m_lowerLeftLayout->insertWidget( 1, nameConfigGroupBox );

    QMinimizePanel* curveGroup = getOrCreateCurveTreeGroup();
    m_lowerLeftLayout->insertWidget( 2, curveGroup, 1 );
    m_lowerLeftLayout->addStretch( 0 );
    m_lowerRightLayout->insertWidget( 1, getOrCreatePlotWidget() );

    // Fields at bottom of dialog
    configureAndUpdateFields( 1, m_bottomFieldLayout, topLevelUiItems, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryPlotEditorWidgetCreator::createWidget( QWidget* parent )
{
    QWidget* widget = new QWidget( parent );

    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins( 5, 5, 5, 5 );
    widget->setLayout( m_layout );

    QFrame* firstRowFrame = new QFrame( widget );
    m_firstRowLayout      = new QHBoxLayout;
    m_firstRowLayout->setContentsMargins( 0, 0, 0, 0 );
    firstRowFrame->setLayout( m_firstRowLayout );

    QFrame* secondRowFrame = new QFrame( widget );
    m_secondRowLayout      = new QHBoxLayout;
    m_secondRowLayout->setContentsMargins( 0, 4, 0, 0 );
    secondRowFrame->setLayout( m_secondRowLayout );

    m_lowerLeftLayout = new QVBoxLayout;
    m_lowerLeftLayout->setContentsMargins( 0, 0, 0, 0 );
    m_secondRowLayout->addLayout( m_lowerLeftLayout );

    m_lowerRightLayout = new QVBoxLayout;
    m_lowerRightLayout->setContentsMargins( 0, 0, 0, 0 );
    m_secondRowLayout->addLayout( m_lowerRightLayout );

    {
        auto label = new QLabel( "Plot Preview" );
        label->setAlignment( Qt::AlignCenter );
        auto font = label->font();
        font.setPixelSize( 20 );
        label->setFont( font );

        m_lowerRightLayout->insertWidget( 0, label );
    }

    m_firstColumnSplitter = new QSplitter( Qt::Vertical );
    m_firstColumnSplitter->setContentsMargins( 0, 0, 0, 0 );

    m_firstColumnSplitter->setHandleWidth( 6 );
    m_firstColumnSplitter->setStyleSheet( "QSplitter::handle { image: url(:/SplitterH.png); }" );

    m_firstColumnSplitter->insertWidget( 0, firstRowFrame );
    m_firstColumnSplitter->insertWidget( 1, secondRowFrame );

    const int firstRowPixelHeight  = 500;
    const int secondRowPixelHeight = 300;

    m_firstColumnSplitter->setSizes( QList<int>() << firstRowPixelHeight << secondRowPixelHeight );

    m_layout->addWidget( m_firstColumnSplitter );

    m_bottomFieldLayout = new QHBoxLayout;
    m_bottomFieldLayout->setContentsMargins( 0, 2, 0, 0 );
    m_layout->addLayout( m_bottomFieldLayout );
    m_bottomFieldLayout->insertStretch( 0, 1 );

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RicSummaryPlotEditorWidgetCreator::findGroupByKeyword( const std::vector<caf::PdmUiItem*>& topLevelUiItems,
                                                                        const QString&                      keyword,
                                                                        const QString& uiConfigName )
{
    for ( auto uiItem : topLevelUiItems )
    {
        if ( uiItem->isUiHidden( uiConfigName ) ) continue;

        if ( uiItem->isUiGroup() )
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>( uiItem );
            if ( group->keyword() == keyword )
            {
                return group;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel* RicSummaryPlotEditorWidgetCreator::getOrCreateCurveTreeGroup()
{
    if ( !m_curvesPanel )
    {
        m_curvesPanel = new QMinimizePanel( this->widget() );
        m_curvesPanel->setTitle( "Curves" );
        QVBoxLayout* curvesLayout = new QVBoxLayout( m_curvesPanel->contentFrame() );

        m_curveTreeView = new caf::PdmUiTreeView( m_curvesPanel->contentFrame() );
        curvesLayout->setStretchFactor( m_curveTreeView, 1 );

        curvesLayout->addWidget( m_curveTreeView );

        m_curveTreeView->treeView()->setHeaderHidden( true );
    }

    if ( m_summaryCurveCreator )
    {
        RimSummaryPlot* previewPlot = m_summaryCurveCreator->previewPlot();
        m_curveTreeView->setPdmItem( previewPlot );
        m_curveTreeView->setUiConfigurationName( RicSummaryPlotEditorUi::CONFIGURATION_NAME );
        m_curveTreeView->setExpanded( previewPlot->summaryCurveCollection(), true );
        m_curveTreeView->setExpanded( previewPlot->ensembleCurveSetCollection(), true );
    }

    return m_curvesPanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryPlotEditorWidgetCreator::getOrCreatePlotWidget()
{
    if ( m_summaryCurveCreator )
    {
        auto widget = m_summaryCurveCreator->previewPlot()->createPlotWidget( this->widget() );
        widget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

        return widget;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorWidgetCreator::configureAndUpdateFields( int                                 widgetStartIndex,
                                                                  QBoxLayout*                         layout,
                                                                  const std::vector<caf::PdmUiItem*>& uiItems,
                                                                  const QString&                      uiConfigName )
{
    int currentWidgetIndex = widgetStartIndex;

    for ( size_t i = 0; i < uiItems.size(); ++i )
    {
        if ( uiItems[i]->isUiHidden( uiConfigName ) ) continue;
        if ( uiItems[i]->isUiGroup() ) continue;

        {
            caf::PdmUiFieldHandle* field = dynamic_cast<caf::PdmUiFieldHandle*>( uiItems[i] );

            caf::PdmUiFieldEditorHandle* fieldEditor = findOrCreateFieldEditor( this->widget(), field, uiConfigName );

            if ( fieldEditor )
            {
                // Place the widget(s) into the correct parent and layout
                QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                if ( fieldCombinedWidget )
                {
                    fieldCombinedWidget->setParent( this->widget() );
                    layout->insertWidget( currentWidgetIndex++, fieldCombinedWidget );
                }
                else
                {
                    caf::PdmUiItemInfo::LabelPosType labelPos = field->uiLabelPosition( uiConfigName );

                    QWidget* fieldEditorWidget = fieldEditor->editorWidget();

                    if ( labelPos != caf::PdmUiItemInfo::HIDDEN )
                    {
                        QWidget* fieldLabelWidget = fieldEditor->labelWidget();
                        if ( fieldLabelWidget )
                        {
                            fieldLabelWidget->setParent( this->widget() );

                            layout->insertWidget( currentWidgetIndex++, fieldLabelWidget );

                            fieldLabelWidget->show();
                        }
                    }
                    else
                    {
                        QWidget* fieldLabelWidget = fieldEditor->labelWidget();
                        if ( fieldLabelWidget ) fieldLabelWidget->hide();
                    }

                    if ( fieldEditorWidget )
                    {
                        fieldEditorWidget->setParent( this->widget() ); // To make sure this widget has the current
                                                                        // group box as parent.

                        layout->insertWidget( currentWidgetIndex++, fieldEditorWidget );
                    }
                }

                fieldEditor->updateUi( uiConfigName );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel* RicSummaryPlotEditorWidgetCreator::createGroupBoxWithContent( caf::PdmUiGroup* group,
                                                                              const QString&   uiConfigName )
{
    QMinimizePanel* groupBox = findOrCreateGroupBox( this->widget(), group, uiConfigName );

    recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, groupBox->contentFrame(), uiConfigName );
    return groupBox;
}
