/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimDeckPositionDlg.h"

#include "RiuTools.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeckPositionDlg::RimDeckPositionDlg( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
    , m_selectedPosition( -1 )
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    QLabel* label = new QLabel( "Select position of open well keyword:" );
    mainLayout->addWidget( label );

    QHBoxLayout* listLayout = new QHBoxLayout();

    m_list = new QListWidget();
    listLayout->addWidget( m_list );

    QVBoxLayout* upDownLayout = new QVBoxLayout();

    QPushButton* moveUp   = new QPushButton( "Up" );
    QPushButton* moveDown = new QPushButton( "Down" );

    connect( moveUp, SIGNAL( clicked() ), this, SLOT( slotMoveUpClicked() ) );
    connect( moveDown, SIGNAL( clicked() ), this, SLOT( slotMoveDownClicked() ) );

    upDownLayout->addStretch( 1 );
    upDownLayout->addWidget( moveUp );
    upDownLayout->addWidget( moveDown );
    upDownLayout->addStretch( 1 );

    listLayout->addLayout( upDownLayout );

    mainLayout->addLayout( listLayout );

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( m_buttons, SIGNAL( accepted() ), this, SLOT( slotDialogOkClicked() ) );
    connect( m_buttons, SIGNAL( rejected() ), this, SLOT( slotDialogCancelClicked() ) );

    mainLayout->addWidget( m_buttons );

    setLayout( mainLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeckPositionDlg::~RimDeckPositionDlg()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDeckPositionDlg::askForPosition( QWidget* parent, std::vector<std::pair<int, QString>> items, QString newItemName, int defPosition )
{
    RimDeckPositionDlg dlg( parent );

    dlg.setWindowTitle( "Open Well" );
    dlg.addItems( items );
    dlg.addNewItem( newItemName, defPosition );

    dlg.resize( 300, 600 );
    if ( dlg.exec() == QDialog::Accepted )
    {
        return dlg.selectedPosition();
    }

    return defPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDeckPositionDlg::selectedPosition() const
{
    return m_selectedPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::slotDialogOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::slotDialogCancelClicked()
{
    reject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::slotMoveUpClicked()
{
    auto item = m_list->takeItem( m_selectedPosition );
    m_selectedPosition--;
    if ( m_selectedPosition < 0 ) m_selectedPosition = 0;
    m_list->insertItem( m_selectedPosition, item );
    m_list->setCurrentRow( m_selectedPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::slotMoveDownClicked()
{
    auto item = m_list->takeItem( m_selectedPosition );
    m_selectedPosition++;
    if ( m_selectedPosition >= m_list->count() ) m_selectedPosition = m_list->count() - 1;
    m_list->insertItem( m_selectedPosition, item );
    m_list->setCurrentRow( m_selectedPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::addItems( std::vector<std::pair<int, QString>> items )
{
    m_list->clear();

    for ( auto& [index, name] : items )
    {
        QListWidgetItem* item = new QListWidgetItem( name, m_list );
        item->setData( Qt::UserRole, index );
        if ( m_selectedPosition < 0 )
        {
            if ( name == "WCONPROD" )
            {
                m_selectedPosition = index + 1;
            }
            else if ( name == "DATES" )
            {
                m_selectedPosition = index - 1;
            }
        }
    }

    if ( m_selectedPosition < 0 )
    {
        m_selectedPosition = (int)items.size() - 1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeckPositionDlg::addNewItem( QString text, int defPosition )
{
    if ( defPosition >= 0 ) m_selectedPosition = defPosition;
    m_list->insertItem( m_selectedPosition, text );
    m_list->setCurrentRow( m_selectedPosition );
}
