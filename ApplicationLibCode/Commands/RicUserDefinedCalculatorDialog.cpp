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

#include "RicUserDefinedCalculatorDialog.h"

#include "RiuTools.h"

#include "RimUserDefinedCalculation.h"
#include "RimUserDefinedCalculationCollection.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicUserDefinedCalculatorDialog::RicUserDefinedCalculatorDialog( QWidget* parent, const QString& title )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
{
    setWindowTitle( title );
    resize( 1200, 800 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicUserDefinedCalculatorDialog::~RicUserDefinedCalculatorDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorDialog::slotTryCloseDialog()
{
    RimUserDefinedCalculationCollection* calcCollection = calculationCollection();

    if ( dirtyCount() > 0 )
    {
        QMessageBox msgBox( this );
        msgBox.setIcon( QMessageBox::Question );

        QString questionText = QString( "Detected calculation expression text modifications." );

        msgBox.setText( questionText );
        msgBox.setInformativeText( "Do you want to trigger calculation?" );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

        int ret = msgBox.exec();
        if ( ret == QMessageBox::No )
        {
            reject();
        }
        else if ( ret == QMessageBox::Yes )
        {
            for ( auto c : calcCollection->calculations() )
            {
                if ( c->isDirty() )
                {
                    c->calculate();
                    c->updateDependentObjects();
                }
            }

            if ( dirtyCount() > 0 )
            {
                return;
            }
        }
        else
        {
            return;
        }
    }

    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorDialog::setUp()
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setContentsMargins( 0, 0, 0, 0 );

    QWidget* calcEditorWidget = getCalculatorWidget();
    mainLayout->addWidget( calcEditorWidget );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Close );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( slotTryCloseDialog() ) );

    mainLayout->addWidget( buttonBox );

    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicUserDefinedCalculatorDialog::dirtyCount() const
{
    size_t count = 0;

    RimUserDefinedCalculationCollection* calcCollection = calculationCollection();
    for ( auto c : calcCollection->calculations() )
    {
        if ( c->isDirty() )
        {
            count++;
        }
    }

    return count;
}
