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

#include "RicCalculatorWidgetCreator.h"

#include "RimUserDefinedCalculation.h"

#include "RicUserDefinedCalculatorUi.h"

#include "cafPdmUiTableView.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCalculatorWidgetCreator::RicCalculatorWidgetCreator( std::unique_ptr<RicUserDefinedCalculatorUi> calculator )
    : m_pdmTableView( nullptr )
{
    m_calculator = std::move( calculator );
    setPdmObject( m_calculator.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCalculatorWidgetCreator::~RicCalculatorWidgetCreator()
{
    if ( m_pdmTableView )
    {
        m_pdmTableView->setChildArrayField( nullptr );

        delete m_pdmTableView;
        m_pdmTableView = nullptr;
    }

    setPdmObject( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculatorWidgetCreator::recursivelyConfigureAndUpdateTopLevelUiOrdering( const caf::PdmUiOrdering& topLevelUiOrdering,
                                                                                  const QString&            uiConfigName )
{
    if ( !m_firstRowLeftLayout || !m_firstRowRightLayout ) return;

    const std::vector<caf::PdmUiItem*>& topLevelUiItems = topLevelUiOrdering.uiItems();

    int layoutItemIndex = 0;
    for ( size_t i = 0; i < topLevelUiItems.size(); ++i )
    {
        if ( topLevelUiItems[i]->isUiHidden( uiConfigName ) ) continue;

        if ( topLevelUiItems[i]->isUiGroup() )
        {
            caf::PdmUiGroup* group    = static_cast<caf::PdmUiGroup*>( topLevelUiItems[i] );
            auto             groupBox = updateGroupBoxWithContent( group, uiConfigName );

            if ( group->keyword() == m_calculator->calculationsGroupName() )
            {
                m_firstRowLeftLayout->addWidget( groupBox );
            }
            else if ( group->keyword() == m_calculator->calulationGroupName() )
            {
                m_firstRowRightLayout->insertWidget( layoutItemIndex++, groupBox );
            }
        }
    }

    if ( m_firstRowRightLayout->itemAt( layoutItemIndex ) != m_parseButtonLayout )
    {
        m_firstRowRightLayout->insertLayout( layoutItemIndex, m_parseButtonLayout );
    }
    layoutItemIndex++;

    if ( m_calculator->currentCalculation() )
    {
        m_pdmTableView->setChildArrayField( m_calculator->currentCalculation()->variables() );
    }
    else
        m_pdmTableView->setChildArrayField( nullptr );

    m_firstRowRightLayout->insertWidget( layoutItemIndex++, m_pdmTableView );

    if ( m_firstRowRightLayout->itemAt( layoutItemIndex ) != m_calculateButtonLayout )
    {
        m_firstRowRightLayout->insertLayout( layoutItemIndex, m_calculateButtonLayout );
    }
    layoutItemIndex++;

    m_pdmTableView->tableView()->resizeColumnsToContents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicCalculatorWidgetCreator::createWidget( QWidget* parent )
{
    m_pdmTableView = new caf::PdmUiTableView( parent );
    m_pdmTableView->tableView()->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_pdmTableView->enableHeaderText( false );

    QHeaderView* verticalHeader = m_pdmTableView->tableView()->verticalHeader();
    verticalHeader->setSectionResizeMode( QHeaderView::Interactive );

    m_pdmTableView->tableView()->resizeColumnsToContents();

    QWidget* widget = new QWidget( parent );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins( 5, 5, 5, 5 );
    widget->setLayout( mainLayout );

    QFrame*      firstRowFrame  = new QFrame( widget );
    QHBoxLayout* firstRowLayout = new QHBoxLayout;
    firstRowLayout->setContentsMargins( 0, 0, 0, 0 );
    firstRowFrame->setLayout( firstRowLayout );

    QFrame* firstRowLeftFrame = new QFrame( widget );
    m_firstRowLeftLayout      = new QHBoxLayout;
    m_firstRowLeftLayout->setContentsMargins( 0, 0, 0, 0 );
    firstRowLeftFrame->setLayout( m_firstRowLeftLayout );

    QFrame* firstRowRightFrame = new QFrame( widget );
    m_firstRowRightLayout      = new QVBoxLayout;
    m_firstRowRightLayout->setContentsMargins( 0, 0, 0, 0 );
    firstRowRightFrame->setLayout( m_firstRowRightLayout );

    QSplitter* rowSplitter = new QSplitter( Qt::Horizontal );
    rowSplitter->setContentsMargins( 0, 0, 0, 0 );
    rowSplitter->setHandleWidth( 6 );
    rowSplitter->setStyleSheet( "QSplitter::handle { image: url(:/SplitterV.png); }" );
    rowSplitter->addWidget( firstRowLeftFrame );
    rowSplitter->addWidget( firstRowRightFrame );
    rowSplitter->setSizes( QList<int>() << 1 << 1 );
    firstRowLayout->addWidget( rowSplitter );

    mainLayout->addWidget( rowSplitter );

    {
        QPushButton* pushButton = new QPushButton( "Parse Expression" );
        connect( pushButton, SIGNAL( clicked() ), this, SLOT( slotParseExpression() ) );
        m_parseButtonLayout = new QHBoxLayout;
        m_parseButtonLayout->addStretch( 10 );
        m_parseButtonLayout->addWidget( pushButton );
    }

    {
        QPushButton* pushButton = new QPushButton( "Calculate" );
        connect( pushButton, SIGNAL( clicked() ), this, SLOT( slotCalculate() ) );
        m_calculateButtonLayout = new QHBoxLayout;
        m_calculateButtonLayout->addStretch( 10 );
        m_calculateButtonLayout->addWidget( pushButton );
    }

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel* RicCalculatorWidgetCreator::updateGroupBoxWithContent( caf::PdmUiGroup* group, const QString& uiConfigName )
{
    QMinimizePanel* groupBox = findOrCreateGroupBox( widget(), group, uiConfigName );

    recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, groupBox->contentFrame(), uiConfigName );
    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicUserDefinedCalculatorUi* RicCalculatorWidgetCreator::calculator() const
{
    return m_calculator.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculatorWidgetCreator::slotCalculate()
{
    m_calculator->calculate();

    m_calculator->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculatorWidgetCreator::slotParseExpression()
{
    m_calculator->parseExpression();

    m_calculator->updateConnectedEditors();
}
