/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicExportSurfaceToGriDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSurfaceToGriDialog::RicExportSurfaceToGriDialog( QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( "Regular Grid Parameters for GRI Export" );

    m_nx         = new QSpinBox( this );
    m_ny         = new QSpinBox( this );
    m_originX    = new QDoubleSpinBox( this );
    m_originY    = new QDoubleSpinBox( this );
    m_incrementX = new QDoubleSpinBox( this );
    m_incrementY = new QDoubleSpinBox( this );

    m_nx->setRange( 2, 100000 );
    m_ny->setRange( 2, 100000 );

    for ( auto* sb : { m_originX, m_originY } )
    {
        sb->setRange( -1.0e9, 1.0e9 );
        sb->setDecimals( 2 );
    }
    for ( auto* sb : { m_incrementX, m_incrementY } )
    {
        sb->setRange( 0.001, 1.0e7 );
        sb->setDecimals( 3 );
    }

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
    connect( m_buttons, &QDialogButtonBox::accepted, this, &RicExportSurfaceToGriDialog::slotOkClicked );
    connect( m_buttons, &QDialogButtonBox::rejected, this, &RicExportSurfaceToGriDialog::slotCancelClicked );

    auto* formLayout = new QFormLayout;
    formLayout->addRow( "Nx (columns):", m_nx );
    formLayout->addRow( "Ny (rows):", m_ny );
    formLayout->addRow( "Origin X:", m_originX );
    formLayout->addRow( "Origin Y:", m_originY );
    formLayout->addRow( "Increment X:", m_incrementX );
    formLayout->addRow( "Increment Y:", m_incrementY );

    auto* mainLayout = new QVBoxLayout( this );
    mainLayout->addLayout( formLayout );
    mainLayout->addWidget( m_buttons );
    setLayout( mainLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGriExportGridParams RicExportSurfaceToGriDialog::openDialog( QWidget* parent, const RicGriExportGridParams& defaults )
{
    RicExportSurfaceToGriDialog dlg( parent );

    dlg.m_nx->setValue( defaults.nx );
    dlg.m_ny->setValue( defaults.ny );
    dlg.m_originX->setValue( defaults.originX );
    dlg.m_originY->setValue( defaults.originY );
    dlg.m_incrementX->setValue( defaults.incrementX );
    dlg.m_incrementY->setValue( defaults.incrementY );

    dlg.exec();
    return dlg.exportParams();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGriExportGridParams RicExportSurfaceToGriDialog::exportParams() const
{
    RicGriExportGridParams params;
    params.accepted   = ( QDialog::result() == QDialog::Accepted );
    params.nx         = m_nx->value();
    params.ny         = m_ny->value();
    params.originX    = m_originX->value();
    params.originY    = m_originY->value();
    params.incrementX = m_incrementX->value();
    params.incrementY = m_incrementY->value();
    return params;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriDialog::slotOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriDialog::slotCancelClicked()
{
    reject();
}
