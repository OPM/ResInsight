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

#include "RiuSummaryVectorSelectionDialog.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCaseCollection.h"

#include "RiuSummaryVectorSelectionUi.h"
#include "RiuSummaryVectorSelectionWidgetCreator.h"
#include "RiuTools.h"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorSelectionDialog::RiuSummaryVectorSelectionDialog( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
{
    m_addrSelWidget =
        std::unique_ptr<RiuSummaryVectorSelectionWidgetCreator>( new RiuSummaryVectorSelectionWidgetCreator() );
    QWidget* addrWidget = m_addrSelWidget->getOrCreateWidget( this );

    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setContentsMargins( 0, 0, 0, 0 );
    mainLayout->addWidget( addrWidget );

    setWindowTitle( "Summary Address Selection" );
    resize( 1200, 800 );

    m_label = new QLabel( "", this );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addStretch( 1 );
    labelLayout->addWidget( m_label );
    labelLayout->addWidget( buttonBox );

    mainLayout->addLayout( labelLayout );

    m_addrSelWidget->summaryAddressSelection()->setFieldChangedHandler( [this]() { this->updateLabel(); } );

    updateLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorSelectionDialog::~RiuSummaryVectorSelectionDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::setCaseAndAddress( RimSummaryCase*                 summaryCase,
                                                         const RifEclipseSummaryAddress& address )
{
    if ( summaryCase )
    {
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        curveDefs.push_back( RiaSummaryCurveDefinition( summaryCase, address, false ) );
        setCurveSelection( curveDefs );
    }
    else
    {
        //  Still need to update the editors
        summaryAddressSelection()->updateConnectedEditors();
        updateLabel();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::setEnsembleAndAddress( RimSummaryCaseCollection*       ensemble,
                                                             const RifEclipseSummaryAddress& address )
{
    if ( ensemble )
    {
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        curveDefs.push_back( RiaSummaryCurveDefinition( ensemble, address ) );
        setCurveSelection( curveDefs );
    }
    else
    {
        //  Still need to update the editors
        summaryAddressSelection()->updateConnectedEditors();
        updateLabel();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::setCurveSelection( const std::vector<RiaSummaryCurveDefinition>& selection )
{
    summaryAddressSelection()->setSelectedCurveDefinitions( selection );
    summaryAddressSelection()->updateConnectedEditors();
    updateLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryVectorSelectionDialog::curveSelection() const
{
    return summaryAddressSelection()->selection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::hideEnsembles()
{
    summaryAddressSelection()->hideEnsembles( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::hideSummaryCases()
{
    summaryAddressSelection()->hideSummaryCases( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::enableMultiSelect( bool enable )
{
    summaryAddressSelection()->setMultiSelectionMode( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::enableIndividualEnsembleCaseSelection( bool enable )
{
    summaryAddressSelection()->enableIndividualEnsembleCaseSelection( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorSelectionUi* RiuSummaryVectorSelectionDialog::summaryAddressSelection() const
{
    return m_addrSelWidget->summaryAddressSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionDialog::updateLabel()
{
    QString                                curveAddressText;
    std::vector<RiaSummaryCurveDefinition> sumCasePairs = this->summaryAddressSelection()->selection();
    if ( sumCasePairs.size() == 1 )
    {
        curveAddressText = sumCasePairs.front().curveDefinitionText();
    }

    if ( curveAddressText.isEmpty() )
    {
        curveAddressText = "<None>";
    }

    QString txt = "Selected Address : ";
    txt += curveAddressText;

    m_label->setText( txt );
}
