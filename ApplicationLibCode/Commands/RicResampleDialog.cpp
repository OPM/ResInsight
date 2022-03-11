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

#include "RicResampleDialog.h"

#include "RiaQDateTimeTools.h"

#include "RiuTools.h"

#include <QAbstractItemView>
#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QToolBar>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicResampleDialog::RicResampleDialog( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
{
    // Create widgets
    m_label           = new QLabel();
    m_timePeriodCombo = new QComboBox();

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    // Connect to signals
    connect( m_buttons, SIGNAL( accepted() ), this, SLOT( slotDialogOkClicked() ) );
    connect( m_buttons, SIGNAL( rejected() ), this, SLOT( slotDialogCancelClicked() ) );

    // Set widget properties
    m_label->setText( "Resampling Period" );

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QHBoxLayout* periodLayout = new QHBoxLayout();
    periodLayout->addWidget( m_label );
    periodLayout->addWidget( m_timePeriodCombo );

    dialogLayout->addLayout( periodLayout );
    dialogLayout->addWidget( m_buttons );

    setLayout( dialogLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicResampleDialog::~RicResampleDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicResampleDialogResult RicResampleDialog::openDialog( QWidget* parent /*= 0*/, const QString& caption /*= QString()*/ )
{
    RicResampleDialog dialog( parent );

    if ( !caption.isEmpty() )
        dialog.setWindowTitle( caption );
    else
        dialog.setWindowTitle( "Export Plot Data" );

    dialog.setPeriodOptions( RiaQDateTimeTools::dateTimePeriods() );

    dialog.resize( 250, 100 );
    dialog.exec();

    return RicResampleDialogResult( dialog.result() == QDialog::Accepted, dialog.selectedDateTimePeriod() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicResampleDialog::setPeriodOptions( const std::vector<RiaDefines::DateTimePeriod>& dateTimePeriods )
{
    QStringList s;
    for ( auto& period : dateTimePeriods )
    {
        QString text = period != RiaDefines::DateTimePeriod::NONE ? RiaQDateTimeTools::dateTimePeriodName( period )
                                                                  : "No Resampling";
        m_timePeriodCombo->addItem( text, QVariant( (int)period ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DateTimePeriod RicResampleDialog::selectedDateTimePeriod() const
{
    int currIndex = m_timePeriodCombo->currentIndex();
    return (RiaDefines::DateTimePeriod)m_timePeriodCombo->itemData( currIndex ).toInt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicResampleDialog::slotDialogOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicResampleDialog::slotDialogCancelClicked()
{
    reject();
}
