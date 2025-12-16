/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RiuPropertyViewWizard.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewWizard::RiuPropertyViewWizard( QWidget*           parent,
                                              caf::PdmObject*    object,
                                              const QString&     windowTitle,
                                              const QStringList& uiConfigNameForPages,
                                              const QStringList& pageSubTitles )
    : QWizard( parent )
{
    setWindowTitle( windowTitle );
    setWizardStyle( QWizard::ModernStyle );

    for ( int i = 0; i < (int)uiConfigNameForPages.size(); i++ )
    {
        QWizardPage* page       = new QWizardPage();
        QHBoxLayout* pageLayout = new QHBoxLayout;
        pageLayout->setContentsMargins( 0, 0, 0, 0 );

        caf::PdmUiPropertyView* pdmUiPropertyView = new caf::PdmUiPropertyView();
        pdmUiPropertyView->setUiConfigurationName( uiConfigNameForPages[i] );

        pageLayout->addWidget( pdmUiPropertyView );

        page->setObjectName( uiConfigNameForPages[i] );
        page->setTitle( uiConfigNameForPages[i] );
        if ( i < (int)pageSubTitles.size() ) page->setSubTitle( pageSubTitles[i] );
        page->setLayout( pageLayout );

        pdmUiPropertyView->showProperties( object );

        addPage( page );

        m_pageWidgets.push_back( pdmUiPropertyView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPropertyViewWizard::~RiuPropertyViewWizard()
{
    for ( auto w : m_pageWidgets )
    {
        w->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewWizard::minimumSizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        QSize pageSize = w->minimumSizeHint();
        pageSize += QSize( 0, 200 );
        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuPropertyViewWizard::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_pageWidgets )
    {
        QSize pageSize = w->sizeHint();
        pageSize += QSize( 100, 200 );
        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPropertyViewWizard::validateCurrentPage()
{
    int currentPageId = currentId();
    if ( currentPageId < 0 || currentPageId >= (int)m_pageWidgets.size() )
    {
        return QWizard::validateCurrentPage();
    }

    caf::PdmUiPropertyView* pageWidget = m_pageWidgets[currentPageId];
    auto                    object     = pageWidget->currentObject();

    auto resultMap = object->validate( pageWidget->uiConfigurationName() );

    if ( !resultMap.empty() )
    {
        QString errorMessages;
        for ( const auto& [fieldName, message] : resultMap )
        {
            errorMessages += QString( "Field '%1': %2\n" ).arg( fieldName, message );
        }
        QMessageBox::critical( this, "Input Error", errorMessages );
        return false;
    }

    return QWizard::validateCurrentPage();
}
