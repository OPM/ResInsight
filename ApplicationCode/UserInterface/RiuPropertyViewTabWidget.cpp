/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuPropertyViewTabWidget.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewTabWidget::RiuPropertyViewTabWidget( QWidget*           parent,
                                                    caf::PdmObject*    object,
                                                    const QString&     windowTitle,
                                                    const QStringList& uiConfigNameForTabs )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    setWindowTitle( windowTitle );

    QTabWidget* tabWidget = new QTabWidget;

    for ( int i = 0; i < uiConfigNameForTabs.size(); i++ )
    {
        QHBoxLayout* widgetLayout = new QHBoxLayout;
        widgetLayout->setContentsMargins( 0, 0, 0, 0 );

        QWidget* containerWidget = new QWidget;
        containerWidget->setLayout( widgetLayout );

        caf::PdmUiPropertyView* pdmUiPropertyView = new caf::PdmUiPropertyView();
        pdmUiPropertyView->setUiConfigurationName( uiConfigNameForTabs[i] );

        widgetLayout->addWidget( pdmUiPropertyView );

        tabWidget->addTab( containerWidget, uiConfigNameForTabs[i] );
        pdmUiPropertyView->showProperties( object );

        m_pageWidgets.push_back( pdmUiPropertyView );
    }

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( tabWidget );

    // Buttons
    m_dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( m_dialogButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_dialogButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_dialogButtonBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewTabWidget::~RiuPropertyViewTabWidget()
{
    for ( auto w : m_pageWidgets )
    {
        w->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewTabWidget::minimumSizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        QSize pageSize = w->minimumSizeHint();
        pageSize += QSize( 0, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewTabWidget::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        // qDebug() << "tab size hint" << w->sizeHint();

        QSize pageSize = w->sizeHint();
        pageSize += QSize( 0, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* RiuPropertyViewTabWidget::dialogButtonBox()
{
    return m_dialogButtonBox;
}
